#include "lokicnn.h"



int main(int argc, char** argv){


	int input_size = atoi(argv[1]);
	int input_depth = atoi(argv[2]);

	int kernel_size = 3;
	int output_depth = atoi(argv[3]);

	int pad = 1;



	// construct a convolutional layer according to the specified
    // parameters
	conv_layer_t* conv = make_conv_layer(input_size, input_size, input_depth,
					kernel_size,
					output_depth,
					1,
					pad); 

	fprintf(stderr, "input size %d %d %d \n", input_size, input_size, input_depth);
	fprintf(stderr, "output size %d %d %d \n", conv->out_sx, conv->out_sy, conv->out_depth);


	// make dummy input and output
  	vol_t** input = (vol_t**)malloc(sizeof(vol_t*)*1);
	input[0] = make_vol(input_size, input_size, input_depth, 0);
	vol_t** output = (vol_t**)malloc(sizeof(vol_t*)*1);
	output[0] = make_vol(conv->out_sx, conv->out_sx, output_depth, 0);


	uint64_t start_time = timestamp_us();
	conv->forward(conv, input, output, 0, 0);
	uint64_t end_time = timestamp_us();
	double dt = (double)(end_time-start_time) / 1000.0;
	fprintf(stderr, " takes %lf ms to complete \n", dt);


}