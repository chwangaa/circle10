/*! \file convolutional_layer.h
    \brief GEMM version of convolutional layer
*/
// Convolutional Layer --------------------------------------------------------
#ifndef CONVOLUTIONAL_LAYER_H
#define CONVOLUTIONAL_LAYER_H

#include "layer.h"
#include "fft.h"
#include "math_functions.h"
#include <math.h>
typedef Layer conv_layer_t;



inline void padOneChannel(Dtype* start, float* new, int pad, int X, int Y){
  assert(start);
  assert(pad >=0 );
  int newX = X + 2*pad;
  int newY = Y + 2*pad;
  // the first few rows will all be 0s
  for(int i = 0; i < pad; i++){
    for(int j = 0; j < newY; j++){
      new[i*newY+j] = 0;
    }
  }
  // then last few rows will all be 0s
  for(int i = X+pad; i < newX; i++){
    for(int j = 0; j < newY; j++){
      new[i*newY+j] = 0;
    }
  }
  // finnally deal with the middle rows
  for(int i = pad; i < X+pad; i++){
    for(int j = 0; j < pad; j++){
      new[i*newY+j] = 0;
    }
    for(int j = pad; j < Y+pad; j++){
      new[i*newY+j] = start[(i-pad)*Y+(j-pad)];
    }
    for(int j = Y+pad; j < newY; j++){
      new[i*newY+j] = 0;
    }
  }
}

inline float* padImage(Dtype* image, int pad, int X, int Y, int Z){
  assert(pad >= 0);
  if(pad == 0){
    return image;
  }
  else{
    int newX = X+2*pad;
    int newY = Y+2*pad;
    float* padded = (float*)malloc(sizeof(float)*(newX)*(newY)*Z);
    assert(padded);
    for(int c = 0; c < Z; c++){
      padOneChannel(&image[X*Y*c], &padded[newX*newY*c], pad, X, Y);
    }
    return padded;
  }
}
/*! 
    \brief conv layer forward routine
    \param l the layer, internally defines convolution parameters like kernel size, padding size, etc.
    \param in the input images
    \param out the ouput images
    \param start the index in the input batch from which to perform forwarding
    \param end the index in the input batch from which to stop perform forwarding

    start and end helps to define the batch size, i.e. batch_size = end-start. They are useful as one can flexibly decides
    whether to load a lot of batches into main memory and use indices to define batch size. And even variable batch_size is 
    possible with changing values of start and end
*/
void conv_forward_fft(const conv_layer_t* l, const vol_t** in, vol_t** out, const int start, const int end) {
  for (int i = start; i <= end; i++) {
    assert(l->stride == 1);
    unsigned int pad = l->pad;
    unsigned int imageX = l->in_sx;
    unsigned int imageY = l->in_sy;
    unsigned int imageC = l->in_depth;
    unsigned int imageSize = (imageX + 2*pad) * (imageY + 2*pad) * imageC;
    unsigned int num_output_layers = l->out_depth;
    unsigned int kernelX = l->sx;
    unsigned int kernelY = l->sy;
    unsigned int kernelC = l->in_depth;
    unsigned int outputX = l->out_sx;
    unsigned int outputY = l->out_sy;
    unsigned int outputSize = l->out_sx * l->out_sy;
    assert(outputX == imageX + 2*pad - kernelX + 1);
    Dtype* image = in[i]->w;
    FourierDomain2D* imageFFT = (FourierDomain2D*)malloc(sizeof(FourierDomain2D*)*imageSize);
    
    assign_bias(num_output_layers, outputSize, l->biases->w, out[i]->w);

    float* image_padded = padImage(image, pad, imageX, imageY, imageC);
    



    int NimageX = imageX + 2*pad;
    int NimageY = imageY + 2*pad;
    constructFTSetFromFloat(image_padded, NimageX, NimageY, imageC, imageFFT, NimageX, NimageY);
    for(int j = 0; j < num_output_layers; j++){
      FourierDomain2D* kernelFFT = &(l->fourier_weights[imageSize*j]);
      assert(kernelFFT);
      LinearCorrelation3InFourierDomain(
                imageFFT, NimageX, NimageY, imageC,
                kernelFFT,NimageX, NimageY, kernelC,
                FFT_ACCUMULATE,
                &out[i]->w[j*outputSize], outputX, outputY);
    }

    free(imageFFT);
    if(pad){
      free(image_padded);
    }
  }
}


void initializeFourierWeightsForConvolutionalLayer(conv_layer_t* l){

    fprintf(stderr, "initialize weights in fourier domain \n");
    unsigned int pad = l->pad;
    assert(pad >= 0);
    unsigned int imageX = l->in_sx + 2*pad;
    unsigned int imageY = l->in_sy + 2*pad;
    unsigned int imageC = l->in_depth;
    unsigned int imageSize = imageX * imageY * imageC;
    unsigned int num_output_layers = l->out_depth;
    unsigned int kernelX = l->sx;
    unsigned int kernelY = l->sy;
    unsigned int kernelC = l->in_depth;
    unsigned int kernelSize = kernelX * kernelY * kernelC;

    // FourierDomain2D* kernelFFT = (FourierDomain2D*)malloc(sizeof(FourierDomain2D)*imageSize*num_output_layers);
    FourierDomain2D* kernelFFT = l->fourier_weights;

    for(int j = 0; j < num_output_layers; j++){
      weight_t* kernel = &(l->filters_flat->w[j*kernelSize]); 
      constructFTSetFromFloatReverse(kernel, kernelX, kernelY, kernelC, &(kernelFFT[j*imageSize]), imageX, imageY);
    }
}
/*! 
    \brief constructor of convolutional layer
    \param in_sx the width of input image
    \param in_sy the height of input image
    \param in_depth the channel number of input image
    \param sx the width of convolution kernel, note the kernel must be square
    \param filters the number of output images, this means the total weight size is filters*sx*sx
    \param stride the number of pixels between consecutive convolutional kernel movement, usually 1
    \param pad the number of 0 paddings to add to the input image before applying convolutions
*/
conv_layer_t* make_conv_layer(int in_sx, int in_sy, int in_depth,
                              int sx, int filters, int stride, int pad) {
  assert(pad >=0);
  assert(in_sx == in_sy);

  conv_layer_t* l = (conv_layer_t*)malloc(sizeof(conv_layer_t));

  l->type = CONVOLUTIONAL;
  // required
  l->sx = sx;             // kernel size
  l->in_depth = in_depth; // input depth
  l->in_sx = in_sx;       // input width
  l->in_sy = in_sy;       // input height
  l->out_depth = filters; // output depth
  
  // optional
  l->sy = l->sx;          /// for LokiCNN, it is required that kernel width == kernel height
  l->stride = stride;     // stride
  l->pad = pad;           // pad

  // computed
  l->out_sx = floor((in_sx + pad * 2 - sx) / stride + 1);
  // fprintf(stderr, "the output dimension is %d x %d \n", l->out_sx, l->out_sx);
  l->out_sy = floor((in_sy + pad * 2 - sx) / stride + 1);
  l->col_inputs = (storage_t*)malloc(sizeof(storage_t)*sx*sx*l->out_sx*l->out_sy*in_depth);
  l->filters_flat = make_vol(sx*sx*in_depth, filters, 1, 0);

  l->biases = make_vol(1, 1, l->out_depth, 0);

  l->forward = &conv_forward_fft;

  unsigned int imageSize = (in_sx + 2*pad) * (in_sy + 2*pad) * in_depth;
  unsigned int num_output_layers = l->out_depth;
  FourierDomain2D* kernelFFT = (FourierDomain2D*)malloc(sizeof(FourierDomain2D)*imageSize*num_output_layers);
  l->fourier_weights = kernelFFT;
  return l;
}


/*! 
    \brief default convolutional layer weight loader
    \param l pointer to the convolutional layer
    \param params specifiers of convolutional layer parameters, used to track whether the weight file matches the convolutional layer's requirement
    \param weights pointer to the starting point of weight array

    this is just one way of loading values, one can easily define its own loader according to specific needs
    for loading from weights trained by caffe, this is an easy approach
*/
void conv_load(conv_layer_t* l, const int* params, const weight_t* weights) {  
  int sx, sy, depth, filters;
  sx = params[0]; sy = params[1]; depth = params[2]; filters = params[3];
  assert(sx == l->sx);
  assert(sy == l->sy);
  assert(depth == l->in_depth);
  assert(filters == l->out_depth);

  int i=0;
  int index = 0;
  for(int d = 0; d < l->out_depth; d++){
    for (int z = 0; z < depth; z++)
      for (int x = 0; x < sx; x++)
        for (int y = 0; y < sy; y++){
          weight_t val = weights[i++];
          l->filters_flat->w[index++] = val;
        }
  }
  initializeFourierWeightsForConvolutionalLayer(l);
  free_vol(l->filters_flat);
  for(int d = 0; d < l->out_depth; d++) {
    weight_t val = weights[i++];
    set_vol(l->biases, 0, 0, d, readDouble(val));
  }
}

#endif