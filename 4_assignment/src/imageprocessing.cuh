#pragma once


// kernel
__global__ void initArray(float* dst, float value);
__global__ void brightness(unsigned char* out_image, float brightfactor, unsigned char* in_image, int height, int width);
__global__ void contrast(unsigned char* out_image, float contrastfactor, unsigned char* in_image, int height, int width);
__global__ void saturation(unsigned char* out_image, float sharpenfactor, unsigned char* in_image, int height, int width);
__global__ void smooth(unsigned char* out_image, unsigned char* in_image,  float *conv_kernel, int length, int height, int width);
__global__ void edgedetection(unsigned char* out_image, unsigned char* in_image, float *conv_kernel, int length, int height, int width);
__global__ void sharpen(unsigned char* out_image, float sharpenfactor, unsigned char* in_image, float *conv_kernel, int length, int height, int width);
__global__ void gaussian(unsigned char* out_image, unsigned char* in_image, float *conv_kernel, int length, int height, int width);
__global__ void profilingconvlayer(unsigned char* out_image, unsigned char* in_image, float *conv_kernel, int length, int height, int width);
__global__ void convlayer(float* out_image, unsigned char* in_image, float *conv_kernel, int length, int height, int width);
__global__ void getminmaxonRGBalongy(float* inputformax, float*intputformin, float* minmaxoutput, int height, int width);
__global__ void getminmaxonRGBalongx(float* inputformax, float*intputformin, int height, int width);
__global__ void NormalizationOutput(unsigned char* out_image, float*input, float* minmaxoutput, int height, int width);
__global__ void floattoimage(unsigned char* out_image, float*input, int height, int width);
__global__ void profilingconvlayer2(unsigned char* out_image, unsigned char* in_image, float *conv_kernel, int length, int channel, int height, int width);

// wrapper
void callbrightness(dim3 grid, dim3 block, unsigned char* out_image, float brightfactor, unsigned char* in_image, int height, int width);
void callcontrast(dim3 grid, dim3 block, unsigned char* out_image, float contrastfactor, unsigned char* d_input, int height, int width);
void callsaturation(dim3 grid, dim3 block, unsigned char* out_image, float saturationfactor, unsigned char* d_input, int height, int width);
void callsmooth(dim3 grid, dim3 block, unsigned char* out_image,  unsigned char* d_input, float *conv_kernel, int length,int height, int width);
void calledgedetection(dim3 grid, dim3 block, unsigned char* out_image, unsigned char* d_input, float *conv_kernel, int length, int height, int width);
void callsharpen(dim3 grid, dim3 block, unsigned char* out_image, float sharpenfactor, unsigned char* d_input, float *conv_kernel, int length, int height, int width);
void callgaussian(dim3 grid, dim3 block, unsigned char* out_image, unsigned char* d_input, float *conv_kernel, int length, int height, int width);
void callconvlayer(dim3 grid, dim3 block, unsigned char* out_image, unsigned char* d_input, float *conv_kernel, int length, int Nconvlayer, int height, int width);
void callprofilingconvlayer(dim3 grid, dim3 block, unsigned char* d_output, unsigned char* d_input, float *conv_kernel, int length, int Nconvlayer, int height, int width);
void callprofilingconvlayer2(dim3 grid, dim3 block, unsigned char* out_image, unsigned char* d_input, float *conv_kernel, int length, int channel, int Nconvlayer, int height, int width);


void testCudaCall();
