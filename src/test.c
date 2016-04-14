#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/time.h>
#include <string.h>

typedef int Dtype
const int M = 128;
const int N = 128;
const int K = 128;

void SimpleMatrixMultiplication(
    const int M,
    const int N,
    const int K,
    const Dtype *A, const int incRowA,
    const Dtype *B, const int incRowB,
    const Dtype *C, const int incRowC){

    for(int i = 0; i < M; i++){
        for(int j = 0; j < N; j ++){
            Dtype accum = 0;
            for(int k = 0; k < K; k++){
                accum += A[i*incRowA+k] * B[k*incRowB+j];
            }
            C[i*incRowC+j] = accum;
        }
    }
}

int main(int argc, char** argv) {
  // int M, N, K;
  // if (argc < 4) {
  //   fprintf(stderr, "Usage: please give the value of M, N, K\n");
  //   return 2;
  // }
  // else{
  //   M = atoi(argv[1]);
  //   N = atoi(argv[2]);
  //   K = atoi(argv[3]);
  // }
    int incRowA = K;
    int incRowB = N;
    int incRowC = N;

	Dtype* A = (Dtype*)malloc(sizeof(int)*M*incRowA);
	Dtype* B = (Dtype*)malloc(sizeof(int)*K*incRowB);
	Dtype* C = (Dtype*)malloc(sizeof(int)*M*incRowC);

    for(int i = 0; i < M; i++){
        for(int j = 0; j < K; j++){
            A[i*incRowA+j] = 1;
        }
    }
    
    for(int i = 0; i < K; i++){
        for(int j = 0; j < N; j++){
            B[i*incRowB+j] = 1;
        }
    }
    
    for(int i = 0; i < M; i++){
        for(int j = 0; j < N; j++){
            C[i*incRowC+j] = 0;
        }
    }



    SimpleMatrixMultiplication(
            M, N, K,
            A, incRowA,
            B, incRowB,
            c, incRowC);

    int error = 0;
    for(int i = 0; i < M; i++){
    // 	// fprintf(stderr, "%d \n", fix16_to_int(M3[i]));
    	for(int j = 0; j < N; j++){
            if(c[i*incRowC+j] != M){
                error++;
    		    // fprintf(stderr, "%d  at %d, %d \n", M3[i*M3_incRow+j], i, j);
            }
        }
    }
    printf("M,  N,  K,  cycle,  instr,  #error \n");
    printf("%d, %d, %d, %d, %d, %d \n", M, N, K, error);
}