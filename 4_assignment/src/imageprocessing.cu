#include "imageprocessing.cuh"
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <string>
#include <math.h>
#include <assert.h>
#include <sstream>
#include <cuda_runtime.h>
// TODO: read about the CUDA programming model: https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#programming-model
// If everything is setup correctly, this file is compiled by the CUDA/C++ compiler (that is different from the C++ compiler).
// The CUDA/C++ compiler understands certain things that your C++ compiler doesn't understand - like '__global__', 'threadIdx', and function calls with triple-angle brackets, e.g., testArray<<<...>>>();
#define RIDX(X,Y,H,W) (X*W+Y)
#define GIDX(X,Y,H,W) ((H+X)*W+Y)
#define BIDX(X,Y,H,W) ((H*2+X)*W+Y)

// do not use this method for anything else than verifying cuda compiled, linked and executed

__global__  void brightness(unsigned char* out_image, float brightfactor, unsigned char* in_image, int height, int width){
    int pos_x = threadIdx.x + blockIdx.x * blockDim.x;
	int pos_y = threadIdx.y + blockIdx.y * blockDim.y;
	if(pos_x >= width || pos_y >= height) return;
	
	unsigned char r = in_image[pos_x * width + pos_y];
    unsigned char g = in_image[(height + pos_x) * width + pos_y];
    unsigned char b = in_image[(height * 2 + pos_x) * width + pos_y];
  
	out_image[pos_x * width + pos_y] = r*brightfactor;
	if(out_image[pos_x * width + pos_y] > 255)
		out_image[pos_x * width + pos_y] = 255;
	    out_image[(height + pos_x) * width + pos_y] = g*brightfactor;
	if(out_image[(height + pos_x) * width + pos_y] > 255)
	    out_image[(height + pos_x) * width + pos_y] = 255;
	    out_image[(height * 2 + pos_x) * width + pos_y] = b*brightfactor;
	if(out_image[(height * 2 + pos_x) * width + pos_y]>255)
		out_image[(height * 2 + pos_x) * width + pos_y] = 255;
}

__global__  void contrast(unsigned char* out_image, float contrastfactor, unsigned char* in_image, int height, int width){
    int pos_x = threadIdx.x + blockIdx.x * blockDim.x;
	int pos_y = threadIdx.y + blockIdx.y * blockDim.y;
	if(pos_x >= width || pos_y >= height) return;
	unsigned char r = in_image[pos_x * width + pos_y];
    unsigned char g = in_image[(height + pos_x) * width + pos_y];
    unsigned char b = in_image[(height * 2 + pos_x) * width + pos_y];
	out_image[pos_x * width + pos_y] = r*contrastfactor + (1-contrastfactor)*255;
	if(out_image[pos_x * width + pos_y] > 255)
		out_image[pos_x * width + pos_y] = 255;
	    out_image[(height + pos_x) * width + pos_y] = g*contrastfactor + (1-contrastfactor)*255;
	if(out_image[(height + pos_x) * width + pos_y] > 255)
	  	out_image[(height + pos_x) * width + pos_y] = 255;
		out_image[(height * 2 + pos_x) * width + pos_y] = b*contrastfactor + (1-contrastfactor)*255;
	if(out_image[(height * 2 + pos_x) * width + pos_y]>255)
		out_image[(height * 2 + pos_x) * width + pos_y] = 255;
}

__global__ void saturation(unsigned char* out_image, float saturationfactor, unsigned char* in_image, int height, int width){
  	int pos_x = threadIdx.x + blockIdx.x * blockDim.x;
	int pos_y = threadIdx.y + blockIdx.y * blockDim.y;
	if(pos_x >= width || pos_y >= height) return;
	unsigned char r = in_image[pos_x * width + pos_y];
 	unsigned char g = in_image[(height + pos_x) * width + pos_y];
  	unsigned char b = in_image[(height * 2 + pos_x) * width + pos_y];
	float intensity = (54.1875*r+182.427*g+18.3855*b) / 255.0;
	out_image[pos_x * width + pos_y] = r*saturationfactor + (1-saturationfactor)*intensity;
	if(out_image[pos_x * width + pos_y] > 255)
		out_image[pos_x * width + pos_y] = 255;
		out_image[(height + pos_x) * width + pos_y] = g*saturationfactor + (1-saturationfactor)*intensity;
	if(out_image[(height + pos_x) * width + pos_y] > 255)
	 	out_image[(height + pos_x) * width + pos_y] = 255;
		out_image[(height * 2 + pos_x) * width + pos_y] = b*saturationfactor + (1-saturationfactor)*intensity;
	if(out_image[(height * 2 + pos_x) * width + pos_y]>255)
		out_image[(height * 2 + pos_x) * width + pos_y] = 255;
}
__global__ void smooth(unsigned char* out_image, unsigned char* in_image, float *conv_kernel, int length, int height, int width){
  	int pos_x = threadIdx.x + blockIdx.x * blockDim.x;
	int pos_y = threadIdx.y + blockIdx.y * blockDim.y;
	if(pos_x >= width || pos_y >= height) return;
	int l = 2*length + 1;
	float size = l*l;
	float tmpr, tmpg, tmpb, originr, origing, originb;
	originr = ((float)in_image[RIDX(pos_x, pos_y, height, width)]) / 255.0f;
	origing = ((float)in_image[GIDX(pos_x, pos_y, height, width)]) / 255.0f;
	originb = ((float)in_image[BIDX(pos_x, pos_y, height, width)]) / 255.0f;
	tmpr = tmpg = tmpb = 0.0f;
	for(int i=(-length); i<=length; i++){
		for(int j=(-length); j<=length; j++){
			int convidx = (i+length)*l+j+length;
			if(pos_x + i < 0 || pos_x + i >= height || pos_y + j < 0 || pos_y + j >= width){
				tmpr += conv_kernel[convidx]*originr; 
				tmpg += conv_kernel[convidx]*origing; 
				tmpb += conv_kernel[convidx]*originb;
			}else{
				tmpr += conv_kernel[convidx]*((float)in_image[RIDX((pos_x+i), (pos_y+j), height, width)])/255.0f; 
				tmpg += conv_kernel[convidx]*((float)in_image[GIDX((pos_x+i),(pos_y+j),height, width)])/255.0f;
				tmpb += conv_kernel[convidx]*((float)in_image[BIDX((pos_x+i), (pos_y+j), height, width)])/255.0f;
			}
		}
	}
	tmpr /= size; tmpg /= size; tmpb /= size;
	out_image[RIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpr*255.0f);
	out_image[GIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpg*255.0f);
	out_image[BIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpb*255.0f);
	if(out_image[RIDX(pos_x, pos_y, height, width)]  > 255)
		out_image[RIDX(pos_x, pos_y, height, width)]  = 255;
	if(out_image[GIDX(pos_x, pos_y, height, width)] > 255)
	  out_image[GIDX(pos_x, pos_y, height, width)] = 255;
	if(out_image[BIDX(pos_x, pos_y, height, width)] > 255)
		out_image[BIDX(pos_x, pos_y, height, width)] = 255;
}

__global__ void edgedetection(unsigned char* out_image, unsigned char* in_image, float *conv_kernel, int length, int height, int width){
	int pos_x = threadIdx.x + blockIdx.x * blockDim.x;
	int pos_y = threadIdx.y + blockIdx.y * blockDim.y;
	if(pos_x >= width || pos_y >= height) return;
	int l = 2*length + 1;
	float size = l*l;
	float tmpr, tmpg, tmpb, originr, origing, originb;
	originr = ((float)in_image[RIDX(pos_x, pos_y, height, width)]) / 255.0f;
	origing = ((float)in_image[GIDX(pos_x, pos_y, height, width)]) / 255.0f;
	originb = ((float)in_image[BIDX(pos_x, pos_y, height, width)]) / 255.0f;
	tmpr = tmpg = tmpb = 0.0f;
	for(int i=(-length); i<=length; i++){
		for(int j=(-length); j<=length; j++){
			int convidx = (i+length)*l+j+length;
			if(pos_x + i < 0 || pos_x + i >= height || pos_y + j < 0 || pos_y + j >= width){
				tmpr += conv_kernel[convidx]*originr; 
				tmpg += conv_kernel[convidx]*origing; 
				tmpb += conv_kernel[convidx]*originb;
			}else{
				tmpr += conv_kernel[convidx]*((float)in_image[RIDX((pos_x+i), (pos_y+j), height, width)])/255.0f; 
				tmpg += conv_kernel[convidx]*((float)in_image[GIDX((pos_x+i), (pos_y+j), height, width)])/255.0f;
				tmpb += conv_kernel[convidx]*((float)in_image[BIDX((pos_x+i), (pos_y+j), height, width)])/255.0f;
			}
		}
	}
	//tmpr /= size; tmpg /= size; tmpb /= size;
	out_image[RIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpr*255.0f);
	out_image[GIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpg*255.0f);
	out_image[BIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpb*255.0f);
	if(out_image[RIDX(pos_x, pos_y, height, width)]  > 255)
		out_image[RIDX(pos_x, pos_y, height, width)]  = 255;
	if(out_image[GIDX(pos_x, pos_y, height, width)] > 255)
	  out_image[GIDX(pos_x, pos_y, height, width)] = 255;
	if(out_image[BIDX(pos_x, pos_y, height, width)] > 255)
		out_image[BIDX(pos_x, pos_y, height, width)] = 255;
}
__global__ void sharpen(unsigned char* out_image, float sharpenfactor, unsigned char* in_image, float *conv_kernel, int length, int height, int width){
    int pos_x = threadIdx.x + blockIdx.x * blockDim.x;
	int pos_y = threadIdx.y + blockIdx.y * blockDim.y;
	if(pos_x >= width || pos_y >= height) return;
	int l = 2*length + 1;
	float size = l*l;
	float tmpr, tmpg, tmpb, originr, origing, originb;
	originr = ((float)in_image[RIDX(pos_x, pos_y, height, width)]) / 255.0f;
	origing = ((float)in_image[GIDX(pos_x, pos_y, height, width)]) / 255.0f;
	originb = ((float)in_image[BIDX(pos_x, pos_y, height, width)]) / 255.0f;
	tmpr = tmpg = tmpb = 0.0f;
	for(int i=(-length); i<=length; i++){
		for(int j=(-length); j<=length; j++){
			int convidx = (i+length)*l+j+length;
			if(pos_x + i < 0 || pos_x + i >= height || pos_y + j < 0 || pos_y + j >= width){
				tmpr += conv_kernel[convidx]*originr; 
				tmpg += conv_kernel[convidx]*origing; 
				tmpb += conv_kernel[convidx]*originb;
			}else{
				tmpr += conv_kernel[convidx]*((float)in_image[RIDX((pos_x+i), (pos_y+j), height, width)])/255.0f; 
				tmpg += conv_kernel[convidx]*((float)in_image[GIDX((pos_x+i), (pos_y+j), height, width)])/255.0f;
				tmpb += conv_kernel[convidx]*((float)in_image[BIDX((pos_x+i), (pos_y+j), height, width)])/255.0f;
			}
		}
	}
	tmpr = originr+sharpenfactor*tmpr; tmpg = origing + sharpenfactor*tmpg; tmpb = originb + sharpenfactor*tmpb;
	out_image[RIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpr*255.0f);
	out_image[GIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpg*255.0f);
	out_image[BIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpb*255.0f);
	if(out_image[RIDX(pos_x, pos_y, height, width)]  > 255)
		out_image[RIDX(pos_x, pos_y, height, width)]  = 255;
	if(out_image[GIDX(pos_x, pos_y, height, width)] > 255)
	  out_image[GIDX(pos_x, pos_y, height, width)] = 255;
	if(out_image[BIDX(pos_x, pos_y, height, width)] > 255)
		out_image[BIDX(pos_x, pos_y, height, width)] = 255;
}

__global__ void gaussian(unsigned char* out_image, unsigned char* in_image, float *conv_kernel, int length, int height, int width){
    int pos_x = threadIdx.x + blockIdx.x * blockDim.x;
	int pos_y = threadIdx.y + blockIdx.y * blockDim.y;
	if(pos_x >= width || pos_y >= height) return;
	int l = 2*length + 1;
	float size = l*l;
	float tmpr, tmpg, tmpb, originr, origing, originb;
	originr = ((float)in_image[RIDX(pos_x, pos_y, height, width)]) / 255.0f;
	origing = ((float)in_image[GIDX(pos_x, pos_y, height, width)]) / 255.0f;
	originb = ((float)in_image[BIDX(pos_x, pos_y, height, width)]) / 255.0f;
	tmpr = tmpg = tmpb = 0.0f;
	for(int i=(-length); i<=length; i++){
		for(int j=(-length); j<=length; j++){
			int convidx = (i+length)*l+j+length;
			if(pos_x + i < 0 || pos_x + i >= height || pos_y + j < 0 || pos_y + j >= width){
				tmpr += conv_kernel[convidx]*originr; 
				tmpg += conv_kernel[convidx]*origing; 
				tmpb += conv_kernel[convidx]*originb;
			}else{
				tmpr += conv_kernel[convidx]*((float)in_image[RIDX((pos_x+i), (pos_y+j), height, width)])/255.0f; 
				tmpg += conv_kernel[convidx]*((float)in_image[GIDX((pos_x+i), (pos_y+j), height, width)])/255.0f;
				tmpb += conv_kernel[convidx]*((float)in_image[BIDX((pos_x+i), (pos_y+j), height, width)])/255.0f;
			}
		}
	}
	out_image[RIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpr*255.0f);
	out_image[GIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpg*255.0f);
	out_image[BIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpb*255.0f);
	if(out_image[RIDX(pos_x, pos_y, height, width)]  > 255)
		out_image[RIDX(pos_x, pos_y, height, width)]  = 255;
	if(out_image[GIDX(pos_x, pos_y, height, width)] > 255)
	  out_image[GIDX(pos_x, pos_y, height, width)] = 255;
	if(out_image[BIDX(pos_x, pos_y, height, width)] > 255)
		out_image[BIDX(pos_x, pos_y, height, width)] = 255;
}

__global__ void profilingconvlayer2(unsigned char* out_image, unsigned char* in_image, float *conv_kernel, int length, int channel, int height, int width){
    int pos_x = threadIdx.x + blockIdx.x * blockDim.x;
	int pos_y = threadIdx.y + blockIdx.y * blockDim.y;
	if(pos_x >= width || pos_y >= height) return;
	__shared__ float sdata[16*16*3];
	__shared__ float sconv_kernel[16*16*3];
	if(threadIdx.x*16+threadIdx.y < length*length*3) sconv_kernel[threadIdx.x*16+threadIdx.y] = conv_kernel[threadIdx.x*16+threadIdx.y];
	sdata[threadIdx.x*16+threadIdx.y] = ((float)in_image[RIDX(pos_x, pos_y, height, width)]) / 255.0f;
	sdata[threadIdx.x*16+threadIdx.y+16*16] = ((float)in_image[GIDX(pos_x, pos_y, height, width)]) / 255.0f;
	sdata[threadIdx.x*16+threadIdx.y+16*16*2] = ((float)in_image[BIDX(pos_x, pos_y, height, width)]) / 255.0f;
	int l = 2*length + 1;
	float size = l*l;
	float tmpr, tmpg, tmpb, originr, origing, originb;
	originr = sdata[threadIdx.x*16+threadIdx.y];
	origing = sdata[threadIdx.x*16+threadIdx.y+16*16];
	originb = sdata[threadIdx.x*16+threadIdx.y+16*16*2];
	tmpr = tmpg = tmpb = 0.0f;
	__syncthreads();
	for(int i=(-length); i<=length; i++){
		for(int j=(-length); j<=length; j++){
			int convidx = (i+length)*l+j+length;
			if(pos_x + i < 0 || pos_x + i >= height || pos_y + j < 0 || pos_y + j >= width){
				tmpr += sconv_kernel[convidx]*originr; 
				tmpg += sconv_kernel[convidx]*origing; 
				tmpb += sconv_kernel[convidx]*originb;
			}else{
				int idx = (int)threadIdx.x;
				int idy = (int)threadIdx.y;
				if(idx+i<0 || idx+i>=16 || idy + j < 0 || idy + j >= 16){
					tmpr += sconv_kernel[convidx]*((float)in_image[RIDX((pos_x+i), (pos_y+j), height, width)])/255.0f; 
					tmpg += sconv_kernel[convidx]*((float)in_image[GIDX((pos_x+i), (pos_y+j), height, width)])/255.0f;
					tmpb += sconv_kernel[convidx]*((float)in_image[BIDX((pos_x+i), (pos_y+j), height, width)])/255.0f;
				}else {
					tmpr += sconv_kernel[convidx]*sdata[(threadIdx.x+i)*16+threadIdx.y]; 
					tmpg += sconv_kernel[convidx]*sdata[(threadIdx.x+i)*16+threadIdx.y+16*16];
					tmpb += sconv_kernel[convidx]*sdata[(threadIdx.x+i)*16+threadIdx.y+16*16*2];
				}
			}
		}
	}
	if(tmpr >= 1.0f) tmpr = 1.0f;
	if(tmpg >= 1.0f) tmpg = 1.0f;
	if(tmpb >= 1.0f) tmpb = 1.0f;
	out_image[RIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpr*255.0f);
	out_image[GIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpg*255.0f);
	out_image[BIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpb*255.0f);
}
__global__ void profilingconvlayer(unsigned char* out_image, unsigned char* in_image, float *conv_kernel, int length, int height, int width){
    int pos_x = threadIdx.x + blockIdx.x * blockDim.x;
	int pos_y = threadIdx.y + blockIdx.y * blockDim.y;
	if(pos_x >= width || pos_y >= height) return;
	int l = 2*length + 1;
	float size = l*l;
	float tmpr, tmpg, tmpb, originr, origing, originb;
	originr = ((float)in_image[RIDX(pos_x, pos_y, height, width)]) / 255.0f;
	origing = ((float)in_image[GIDX(pos_x, pos_y, height, width)]) / 255.0f;
	originb = ((float)in_image[BIDX(pos_x, pos_y, height, width)]) / 255.0f;
	tmpr = tmpg = tmpb = 0.0f;
	for(int i=(-length); i<=length; i++){
		for(int j=(-length); j<=length; j++){
			int convidx = (i+length)*l+j+length;
			if(pos_x + i < 0 || pos_x + i >= height || pos_y + j < 0 || pos_y + j >= width){
				tmpr += conv_kernel[convidx]*originr; 
				tmpg += conv_kernel[convidx]*origing; 
				tmpb += conv_kernel[convidx]*originb;
			}else{
				tmpr += conv_kernel[convidx]*((float)in_image[RIDX((pos_x+i), (pos_y+j), height, width)])/255.0f; 
				tmpg += conv_kernel[convidx]*((float)in_image[GIDX((pos_x+i), (pos_y+j), height, width)])/255.0f;
				tmpb += conv_kernel[convidx]*((float)in_image[BIDX((pos_x+i), (pos_y+j), height, width)])/255.0f;
			}
		}
	}
	out_image[RIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpr*255.0f);
	out_image[GIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpg*255.0f);
	out_image[BIDX(pos_x, pos_y, height, width)] = (unsigned char) (tmpb*255.0f);
	if(out_image[RIDX(pos_x, pos_y, height, width)]  > 255)
		out_image[RIDX(pos_x, pos_y, height, width)]  = 255;
	if(out_image[GIDX(pos_x, pos_y, height, width)] > 255)
	  out_image[GIDX(pos_x, pos_y, height, width)] = 255;
	if(out_image[BIDX(pos_x, pos_y, height, width)] > 255)
		out_image[BIDX(pos_x, pos_y, height, width)] = 255;
}

__global__ void convlayer(float* out_image, unsigned char* in_image, float *conv_kernel, int length, int height, int width){
    int pos_x = threadIdx.x + blockIdx.x * blockDim.x;
	int pos_y = threadIdx.y + blockIdx.y * blockDim.y;
	if(pos_x >= width || pos_y >= height) return;
	int l = 2*length + 1;
	int size = l*l;
	float tmpr, tmpg, tmpb, originr, origing, originb;
	originr = ((float)in_image[RIDX(pos_x, pos_y, height, width)]) / 255.0f;
	origing = ((float)in_image[GIDX(pos_x, pos_y, height, width)]) / 255.0f;
	originb = ((float)in_image[BIDX(pos_x, pos_y, height, width)]) / 255.0f;
	tmpr = tmpg = tmpb = 0.0f;
	for(int i=(-length); i<=length; i++){
		for(int j=(-length); j<=length; j++){
			int convidx = (i+length)*l+j+length;
			if(pos_x + i < 0 || pos_x + i >= height || pos_y + j < 0 || pos_y + j >= width){
				tmpr += conv_kernel[convidx]*originr; 
				tmpg += conv_kernel[convidx+size]*origing; 
				tmpb += conv_kernel[convidx+2*size]*originb;
			}else{
				tmpr += conv_kernel[convidx]*((float)in_image[RIDX((pos_x+i), (pos_y+j), height, width)])/255.0f; 
				tmpg += conv_kernel[convidx+size]*((float)in_image[GIDX((pos_x+i), (pos_y+j), height, width)])/255.0f;
				tmpb += conv_kernel[convidx+2*size]*((float)in_image[BIDX((pos_x+i), (pos_y+j), height, width)])/255.0f;
			}
		}
	}
	out_image[RIDX(pos_x, pos_y, height, width)] = tmpr;
	out_image[GIDX(pos_x, pos_y, height, width)] = tmpg;
	out_image[BIDX(pos_x, pos_y, height, width)] = tmpb;
}

__global__ void getminmaxonRGBalongy(float* inputformax, float*inputformin, float* minmaxoutput, int height, int width){
	int pos_x = threadIdx.x + blockIdx.x * blockDim.x;
	int pos_y = threadIdx.y + blockIdx.y * blockDim.y;
	if(pos_x >= width || pos_y >= height) return;
	for ( unsigned int s = 1; s < gridDim.y; s*=2){
		int idx = pos_y+s;
		if(pos_y % (2*s) == 0){
			float maxbottomhalf = inputformax[RIDX((pos_x), (idx), height, width)];
		 	float maxself = inputformax[RIDX((pos_x), (pos_y), height, width)];
			float minbottomhalf = inputformin[RIDX((pos_x), (idx), height, width)];
		 	float minself = inputformin[RIDX((pos_x), (pos_y), height, width)];
			if (maxbottomhalf > maxself){
				inputformax[RIDX((pos_x), (pos_y), height, width)] = maxbottomhalf;
			}
			if (minbottomhalf < minself){
				inputformin[RIDX((pos_x), (pos_y), height, width)] = minbottomhalf;
			}
			maxbottomhalf = inputformax[GIDX((pos_x), (idx), height, width)];
  			maxself = inputformax[GIDX((pos_x), (pos_y), height, width)];
			minbottomhalf = inputformin[GIDX((pos_x), (idx), height, width)];
		    minself = inputformin[GIDX((pos_x), (pos_y), height, width)];
			if (maxbottomhalf > maxself){
				inputformax[GIDX((pos_x), (pos_y), height, width)] = maxbottomhalf;
			}
			if (minbottomhalf < minself){
				inputformin[GIDX((pos_x), (pos_y), height, width)] = minbottomhalf;
			}
			maxbottomhalf = inputformax[BIDX((pos_x), (idx), height, width)];
   		    maxself = inputformax[BIDX((pos_x), (pos_y), height, width)];
			minbottomhalf = inputformin[BIDX((pos_x), (idx), height, width)];
		    minself = inputformin[BIDX((pos_x), (pos_y), height, width)];
			if (maxbottomhalf > maxself){
				inputformax[BIDX((pos_x), (pos_y), height, width)] = maxbottomhalf;
			}
			if (minbottomhalf < minself){
				inputformin[BIDX((pos_x), (pos_y), height, width)] = minbottomhalf;
			}
		}
		__syncthreads();
	}
	if(pos_x == 0 and pos_y == 0){
	 minmaxoutput[0] = inputformax[0];
	 minmaxoutput[1] = inputformin[0];
	 minmaxoutput[2] = inputformax[512*512];
	 minmaxoutput[3] = inputformin[512*512];
	 minmaxoutput[4] = inputformax[512*512*2];
	 minmaxoutput[5] = inputformin[512*512*2];
	}
		__syncthreads();
}

__global__ void getminmaxonRGBalongx(float* inputformax, float*inputformin, int height, int width){
	int pos_x = threadIdx.x + blockIdx.x * blockDim.x;
	int pos_y = threadIdx.y + blockIdx.y * blockDim.y;
	if(pos_x >= width || pos_y >= height) return;
	// reduce along x
	for ( unsigned int s = 1; s < blockDim.x; s*=2){
		int idx = pos_x+s;
		if(pos_x % (2*s) == 0){
			float maxbottomhalf = inputformax[RIDX((idx), (pos_y), height, width)];
		    float maxself = inputformax[RIDX((pos_x), (pos_y), height, width)];
			float minbottomhalf = inputformin[RIDX((idx), (pos_y), height, width)];
	   	    float minself = inputformin[RIDX((pos_x), (pos_y), height, width)];
			if (maxbottomhalf > maxself){
				inputformax[RIDX((pos_x), (pos_y), height, width)] = maxbottomhalf;
			}

			if (minbottomhalf < minself){
				inputformin[RIDX((pos_x), (pos_y), height, width)] = minbottomhalf;
			}
    		maxbottomhalf = inputformax[GIDX((idx), (pos_y), height, width)];
	    	maxself = inputformax[GIDX((pos_x), (pos_y), height, width)];
			minbottomhalf = inputformin[GIDX((idx), (pos_y), height, width)];
		    minself = inputformin[GIDX((pos_x), (pos_y), height, width)];
			if (maxbottomhalf > maxself){
				inputformax[GIDX((pos_x), (pos_y), height, width)] = maxbottomhalf;
			}
			if (minbottomhalf < minself){
				inputformin[GIDX((pos_x), (pos_y), height, width)] = minbottomhalf;
			}
			maxbottomhalf = inputformax[BIDX((idx), (pos_y), height, width)];
		    maxself = inputformax[BIDX((pos_x), (pos_y), height, width)];
			minbottomhalf = inputformin[BIDX((idx), (pos_y), height, width)];
		    minself = inputformin[BIDX((pos_x), (pos_y), height, width)];
			if (maxbottomhalf > maxself){
				inputformax[BIDX((pos_x), (pos_y), height, width)] = maxbottomhalf;
			}
			if (minbottomhalf < minself){
				inputformin[BIDX((pos_x), (pos_y), height, width)] = minbottomhalf;
			}
		}
		__syncthreads();
	}

}

__global__ void NormalizationOutput(unsigned char* out_image, float*input, float* minmaxoutput, int height, int width){
	int pos_x = threadIdx.x + blockIdx.x * blockDim.x;
	int pos_y = threadIdx.y + blockIdx.y * blockDim.y;
	if(pos_x >= width || pos_y >= height) return;
	float current = input[RIDX((pos_x), (pos_y), height, width)];
	current = (current - minmaxoutput[1])/(minmaxoutput[0] - minmaxoutput[1]);
    out_image[RIDX((pos_x), (pos_y), height, width)] = (unsigned char) (current*255.0f);
	current = input[GIDX((pos_x), (pos_y), height, width)];
	current = (current - minmaxoutput[3])/(minmaxoutput[2] - minmaxoutput[3]);
    out_image[GIDX((pos_x), (pos_y), height, width)] = (unsigned char) (current*255.0f);
 	current = input[BIDX((pos_x), (pos_y), height, width)];
	current = (current - minmaxoutput[5])/(minmaxoutput[4] - minmaxoutput[5]);
    out_image[BIDX((pos_x), (pos_y), height, width)] = (unsigned char) (current*255.0f);
}

__global__ void floattoimage(unsigned char* out_image, float*input, int height, int width){
	int pos_x = threadIdx.x + blockIdx.x * blockDim.x;
	int pos_y = threadIdx.y + blockIdx.y * blockDim.y;
	if(pos_x >= width || pos_y >= height) return;
	float current = input[RIDX((pos_x), (pos_y), height, width)];
    out_image[RIDX((pos_x), (pos_y), height, width)] = (unsigned char) (current*255.0f);
	current = input[GIDX((pos_x), (pos_y), height, width)];
    out_image[GIDX((pos_x), (pos_y), height, width)] = (unsigned char) (current*255.0f);
 	current = input[BIDX((pos_x), (pos_y), height, width)];
    out_image[BIDX((pos_x), (pos_y), height, width)] = (unsigned char) (current*255.0f);
}

// wrapper
void callbrightness(dim3 grid, dim3 block, unsigned char* out_image, float brightfactor, unsigned char* d_input, int height, int width){
	brightness <<<grid, block>>> (out_image, brightfactor, d_input, height, width);
}
void callcontrast(dim3 grid, dim3 block, unsigned char* out_image, float contrastfactor, unsigned char* d_input, int height, int width){
	 contrast <<<grid, block>>> (out_image, contrastfactor, d_input, height, width);
}
void callsaturation(dim3 grid, dim3 block, unsigned char* out_image, float saturationfactor, unsigned char* d_input, int height, int width){
	 saturation <<<grid, block>>> (out_image, saturationfactor, d_input, height, width);
}
void callsmooth(dim3 grid, dim3 block, unsigned char* out_image,  unsigned char* d_input, float *conv_kernel, int length,int height, int width){
	 assert(length % 2 == 1);
	 int halflen = length/2;
	 smooth <<<grid, block>>> (out_image, d_input, conv_kernel, halflen, height, width);
     cudaError_t err = cudaGetLastError();
     if (err != cudaSuccess) 
	  printf("Error: %s\n", cudaGetErrorString(err));
}
void calledgedetection(dim3 grid, dim3 block, unsigned char* out_image,unsigned char* d_input, float *conv_kernel, int length, int height, int width){
	assert(length % 2 == 1);
	int halflen = length/2;
	edgedetection <<<grid, block>>> (out_image, d_input, conv_kernel, halflen, height, width);
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) 
	  printf("Error: %s\n", cudaGetErrorString(err));

}
void callsharpen(dim3 grid, dim3 block, unsigned char* out_image, float sharpenfactor, unsigned char* d_input, float *conv_kernel, int length, int height, int width){
	assert(length % 2 == 1);
	int halflen = length/2;
	sharpen <<<grid, block>>> (out_image, sharpenfactor, d_input, conv_kernel, halflen, height, width);
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) 
	  printf("Error: %s\n", cudaGetErrorString(err));
}
void callgaussian(dim3 grid, dim3 block, unsigned char* out_image, unsigned char* d_input, float *conv_kernel, int length, int height, int width){
	assert(length % 2 == 1);
	int halflen = length/2;
	gaussian <<<grid, block>>> (out_image, d_input, conv_kernel, halflen, height, width);
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) 
	  printf("Error: %s\n", cudaGetErrorString(err));
}

void callconvlayer(dim3 grid, dim3 block, unsigned char* out_image, unsigned char* d_input, float *conv_kernel, int length, int Nconvlayer, int height, int width){
	assert(length % 2 == 1);
	int halflen = length/2;
//	minmax_binary_op<float> binary_op;
//	minmax_pair<int> init = unary_op(data[0]);
	float *floatout;
	cudaMalloc((void **)&floatout, 512*512*3*sizeof(float));
	float *minworkspace;
	cudaMalloc((void **)&minworkspace, 512*512*3*sizeof(float));
	float *maxworkspace;
	cudaMalloc((void **)&maxworkspace, 512*512*3*sizeof(float));
	float *minmaxoutput;
	cudaMalloc((void **)&minmaxoutput, 3*2*sizeof(float));
	float *hostminmax = (float*)malloc(3*2*sizeof(float));
	convlayer <<<grid, block>>> (floatout, d_input, conv_kernel, halflen, height, width);
	cudaMemcpy(minworkspace, floatout, 512*512*3*sizeof(float), cudaMemcpyDeviceToDevice);
	cudaMemcpy(maxworkspace, floatout, 512*512*3*sizeof(float), cudaMemcpyDeviceToDevice);
  getminmaxonRGBalongx <<<grid, block>>> (minworkspace, maxworkspace, height, width);
	cudaError_t err = cudaGetLastError();
	 if (err != cudaSuccess) 
		printf("Error: %s\n", cudaGetErrorString(err));
	getminmaxonRGBalongy <<<grid, block>>> (minworkspace, maxworkspace, minmaxoutput, height, width);
	cudaMemcpy(hostminmax, minmaxoutput, 3*2*sizeof(float), cudaMemcpyDeviceToHost);
	NormalizationOutput <<<grid, block>>> (out_image, floatout, minmaxoutput, height, width);
//	floattoimage <<<grid, block>>> (out_image, floatout, height, width);
	err = cudaGetLastError();
	 if (err != cudaSuccess) 
		printf("Error: %s\n", cudaGetErrorString(err));
}

void callprofilingconvlayer(dim3 grid, dim3 block, unsigned char* out_image, unsigned char* d_input, float *conv_kernel, int length, int Nconvlayer, int height, int width){
	assert(length % 2 == 1);
	assert(Nconvlayer == 1);
	int halflen = length/2;
	unsigned char *charout;
	cudaMalloc((void **)&charout, height*width*3*sizeof(unsigned char));
    cudaEvent_t start, stop;
	cudaEventCreate(&start);
    cudaEventCreate(&stop);
	cudaEventRecord(start);
	profilingconvlayer <<<grid, block>>> (charout, d_input, conv_kernel, halflen, height, width);
    cudaEventRecord(stop);
	cudaEventSynchronize(stop);
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) 
	  printf("Error: %s\n", cudaGetErrorString(err));
    float milliseconds = 0;
	cudaEventElapsedTime(&milliseconds, start, stop);
	printf("take time: %f ms \n", milliseconds);
	cudaFree(charout);
}

void callprofilingconvlayer2(dim3 grid, dim3 block, unsigned char* out_image, unsigned char* d_input, float *conv_kernel, int length, int channel, int Nconvlayer, int height, int width){
	assert(length % 2 == 1);
	assert(Nconvlayer == 1);
	int halflen = length/2;
	unsigned char *charout;
	cudaMalloc((void **)&charout, height*width*3*sizeof(unsigned char));
    cudaEvent_t start, stop;
	cudaEventCreate(&start);
    cudaEventCreate(&stop);
	cudaEventRecord(start);
	profilingconvlayer2 <<<grid, block>>> (charout, d_input, conv_kernel, halflen, channel, height, width);
    cudaEventRecord(stop);
	cudaEventSynchronize(stop);
    cudaError_t err = cudaGetLastError();
    if (err != cudaSuccess) 
	  printf("Error: %s\n", cudaGetErrorString(err));
    float milliseconds = 0;
	cudaEventElapsedTime(&milliseconds, start, stop);
	printf("take time: %f ms \n", milliseconds);
	cudaFree(charout);
}



__global__  void testArray(float* dst, float value) {
	unsigned int index = threadIdx.x;
	dst[index] = value;
}
	void testCudaCall() {
	// quick and dirty test of CUDA setup
	const unsigned int N = 1024;
	float *device_array;
	cudaMalloc(&device_array, N * sizeof(float));
	testArray <<<1, N>>> (device_array, -2.5f);
	float *x = (float *)malloc(N*sizeof(float));
	cudaMemcpy(x, device_array, N * sizeof(float), cudaMemcpyDeviceToHost);
	std::cout << "quick and dirty test of CUDA setup: " << x[0] << " " << x[1] << " " << x[1023] << std::endl;
	cudaFree(device_array);
}

// TODO: implement the image processing operations using CUDA kernels
