// CS 380 - GPGPU Programming
// Programming Assignment #4
// system includes
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <math.h>
#include <sstream>
#include <algorithm> 
#include <array>
#include <string>
#include <cuda_runtime.h>
#include "device_launch_parameters.h"
#include "CImg.h"
#include "imageprocessing.cuh"

// query GPU functionality we need for CUDA, return false when not available
bool queryGPUCapabilitiesCUDA()
{
	// Device Count
	int devCount;
	// Get the Device Count
	cudaGetDeviceCount(&devCount);
	// Print Device Count
	printf("Device(s): %i\n", devCount);
	// TODO: query anything else you will need
	return devCount > 0;
}


void getconv(float **conv_kernel, int l){
	// l=3 means conv kernel size is 3*3 
	float *tmp = (float*)malloc(l*l*sizeof(float));
	for(int i=0; i<l*l; i++){
		tmp[i] = 1.0;
	}
	*(conv_kernel) = tmp;
}

void getgaussian(float **gaussian_kernel, int l){
 	// l=3 means conv kernel size is 3*3 
	float *tmp = (float*)malloc(l*l*sizeof(float));
	int halflen = l / 2;
	float sigma = 1.0f;
	for(int i=-halflen; i<halflen+1; i++)
		for(int j=-halflen; j<halflen+1; j++){
			tmp[(i+halflen)*l+j+halflen] = 1/(2*3.1415926*sigma*sigma)*exp(-1*((i*i)+(j*j))/(2*sigma*sigma));
		}
	*(gaussian_kernel) = tmp;
}


int savetojpg(cimg_library::CImg<unsigned char> img, std::string filename){
	std::string prefix = "./";
	std::string suffix = ".bmp";
	std::string pfile = prefix + filename + suffix;
	img.save(pfile.c_str());
 return 0;	
}

// entry point
int main(int argc, char** argv)
{
	// query CUDA capabilities
	if (!queryGPUCapabilitiesCUDA())
	{
		// quit in case capabilities are insufficient
		exit(EXIT_FAILURE);
	}
	testCudaCall();

	// simple example taken and modified from http://cimg.eu/
	// load image
	cimg_library::CImg<unsigned char> image("../../data/images/lichtenstein_full.bmp");
	int imgheight = image.height();
	int imgwidth = image.width();
	int imgproduct = imgheight*imgwidth;
	cimg_library::CImg<unsigned char> originimage = image;
	// create image for simple visualization
	cimg_library::CImg<unsigned char> visualization(512, 300, 1, 3, 0);
	const unsigned char red[] = { 255, 0, 0 };
	const unsigned char green[] = { 0, 255, 0 };
	const unsigned char blue[] = { 0, 0, 255 };
    unsigned char  *d_output, *d_input;
	cudaMalloc((void **)&d_input, imgproduct*3*sizeof(unsigned char));
	cudaMalloc((void **)&d_output, imgproduct*3*sizeof(unsigned char));
	cudaMemcpy(d_input, originimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
	dim3 grid(1,512);
	dim3 block(512,1);
	// create displays 
	cimg_library::CImgDisplay inputImageDisplay(image, "click to select row of pixels");
	inputImageDisplay.move(40, 40);
	cimg_library::CImgDisplay visualizationDisplay(visualization, "intensity profile of selected row");
	visualizationDisplay.move(600, 40);
    // image processing part paramter
	float brightnessfactor = 1.0f;
	float contrastfactor = 1.0f;
	float saturationfactor = 1.0f;
	bool smooth, edgedetection, sharpen, gauss;
	smooth = edgedetection = sharpen = gauss = false;
   // smooth conv kernel  
	int smoothsize = 3;
	float * smconv3x3, *dsmconv3x3, *smconv7x7, *dsmconv7x7, *smconv9x9, *dsmconv9x9;
	getconv(&smconv3x3, 3);
	getconv(&smconv7x7, 7);
	getconv(&smconv9x9, 9);
	cudaMalloc((void **)&dsmconv3x3, 3*3*sizeof(float));
	cudaMalloc((void **)&dsmconv7x7, 7*7*sizeof(float));
	cudaMalloc((void **)&dsmconv9x9, 9*9*sizeof(float));
	cudaMemcpy(dsmconv3x3, smconv3x3, 3*3*sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy(dsmconv7x7, smconv7x7, 7*7*sizeof(float), cudaMemcpyHostToDevice);
	cudaMemcpy(dsmconv9x9, smconv9x9, 9*9*sizeof(float), cudaMemcpyHostToDevice);
    // edge conv kernel  
	float * edgeconv3x3, *dedgeconv3x3;
	getconv(&edgeconv3x3, 3);
	edgeconv3x3[0] = edgeconv3x3[2] = edgeconv3x3[6] = edgeconv3x3[8]	= 0.0;
	edgeconv3x3[1] = edgeconv3x3[3] = edgeconv3x3[5] = edgeconv3x3[7] = -1.0;
    edgeconv3x3[4] = 4.0;
	cudaMalloc((void **)&dedgeconv3x3, 3*3*sizeof(float));
	cudaMemcpy(dedgeconv3x3, edgeconv3x3, 3*3*sizeof(float), cudaMemcpyHostToDevice);
	// sharpen factor 
	float sharpenfactor = 0.5;
	// gaussian kernel 
	float* gaconv7x7, *dgaconv7x7;
	float sigma=1;
	getconv(&gaconv7x7,7);
	for(int i=-3; i<4; i++)
		for(int j=-3; j<4; j++){
			gaconv7x7[(i+3)*7+j+3] = 1/(2*3.1415926*sigma*sigma)*exp(-1*((i*i)+(j*j))/(2*sigma*sigma));
		}
    cudaMalloc((void **)&dgaconv7x7, 7*7*sizeof(float));
	cudaMemcpy(dgaconv7x7, gaconv7x7, 7*7*sizeof(float), cudaMemcpyHostToDevice);
      //	printf("image origin %d %d %d %d %d %d %d %d %d", image[0], image[1], image[2], image[512], image[513], image[514], image[1024], image[1025], image[1026]);
	while (!inputImageDisplay.is_closed() && !visualizationDisplay.is_closed()) {
		inputImageDisplay.wait();
		// first copy current image to device
		cudaMemcpy(d_input, image, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
		if (inputImageDisplay.button() && inputImageDisplay.mouse_y() >= 0) {
			// on click redraw visualization
			const int y = inputImageDisplay.mouse_y();
			visualization.fill(0).draw_graph(image.get_crop(0, y, 0, 0, image.width() - 1, y, 0, 0), red, 1, 1, 0, 255, 0);
			visualization.draw_graph(image.get_crop(0, y, 0, 1, image.width() - 1, y, 0, 1), green, 1, 1, 0, 255, 0);
			visualization.draw_graph(image.get_crop(0, y, 0, 2, image.width() - 1, y, 0, 2), blue, 1, 1, 0, 255, 0).display(visualizationDisplay);
		}else if(inputImageDisplay.is_keyB()){ // brightness on
			brightnessfactor -= 0.1;			
			brightnessfactor = brightnessfactor < 0.0 ? 0.0 : brightnessfactor;
			printf("brightness %f \n", brightnessfactor);
			cudaMemcpy(d_input, originimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
			callbrightness(grid, block, d_output, brightnessfactor, d_input, imgheight, imgwidth);
			cudaMemcpy(image, d_output, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToHost);
		}else if(inputImageDisplay.is_keyN()){ // brightness down
			brightnessfactor += 0.1;
			brightnessfactor = brightnessfactor > 1.0 ? 1.0 : brightnessfactor;
			printf("brightness %f \n", brightnessfactor);
			cudaMemcpy(d_input, originimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
			callbrightness(grid, block, d_output, brightnessfactor, d_input, imgheight, imgwidth);
			cudaMemcpy(image, d_output, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToHost);
		}else if(inputImageDisplay.is_keyT()){ // contrast up
			contrastfactor += 0.1;			
			//contrastfactor = contrastfactor > 1.0 ? 1.0 : contrastfactor;
			printf("contrastfactor %f \n", contrastfactor);
			cudaMemcpy(d_input, originimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
			callcontrast(grid, block, d_output, contrastfactor, d_input, imgheight, imgwidth);
			cudaMemcpy(image, d_output, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToHost);
		}else if(inputImageDisplay.is_keyY()){ // contrast down
			contrastfactor -= 0.1;			
			//contrastfactor = contrastfactor < 0.0 ? 0.0 : contrastfactor;
			printf("contrastfactor %f \n", contrastfactor);
			cudaMemcpy(d_input, originimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
			callcontrast(grid, block, d_output, contrastfactor, d_input, imgheight, imgwidth);
			cudaMemcpy(image, d_output, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToHost);
		}else if(inputImageDisplay.is_keyS()){ // saturation up
			saturationfactor += 0.1;			
			saturationfactor = saturationfactor > 1.0 ? 1.0 : saturationfactor;
			printf("saturationfactor %f \n", saturationfactor);
			cudaMemcpy(d_input, originimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
			callsaturation(grid, block, d_output, saturationfactor, d_input, imgheight, imgwidth);
			cudaMemcpy(image, d_output, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToHost);
		}else if(inputImageDisplay.is_keyD()){ // saturation down
			saturationfactor -= 0.1;			
			saturationfactor = saturationfactor < 0.0 ? 0.0 : saturationfactor;
			printf("saturationfactor %f \n", saturationfactor);
			cudaMemcpy(d_input, originimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
			callsaturation(grid, block, d_output, saturationfactor, d_input, imgheight, imgwidth);
			cudaMemcpy(image, d_output, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToHost);
		}else if(inputImageDisplay.is_keyO()){ // smooth on/off
			cudaMemcpy(d_input, originimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
			if(smooth){
				printf("shutdown smooth\n");
				smooth = false;
			  cudaMemcpy(d_output, d_input, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToDevice);
			}else{
				printf("open smooth\n");
				smooth = true;
				if(smoothsize == 3) callsmooth(grid, block, d_output, d_input, dsmconv3x3, 3, imgheight, imgwidth);
				else if(smoothsize == 7) callsmooth(grid, block, d_output, d_input, dsmconv7x7, 7, imgheight, imgwidth);
				else if(smoothsize == 9) callsmooth(grid, block, d_output, d_input, dsmconv9x9, 9, imgheight, imgwidth);
			}
			cudaMemcpy(image, d_output, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToHost);
		}else if(inputImageDisplay.is_keyE()){ // edge detection on/off
			cudaMemcpy(d_input, originimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
			if(edgedetection){
				printf("shutdown edgedetection\n");
				edgedetection = false;
			  cudaMemcpy(d_output, d_input, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToDevice);
			}else{
				printf("open edgedetection\n");
				edgedetection = true;
	  		calledgedetection(grid, block, d_output, d_input, dedgeconv3x3, 3, imgheight, imgwidth);
			}
			cudaMemcpy(image, d_output, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToHost);
		}else if(inputImageDisplay.is_keyP()){ // sharpening on/off
			cudaMemcpy(d_input, originimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
			if(sharpen){
				printf("shutdown sharpen\n");
				sharpen = false;
  		  cudaMemcpy(d_output, d_input, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToDevice);
			}else{
				printf("open sharpen\n");
				sharpen = true;
	  		callsharpen(grid, block, d_output, sharpenfactor, d_input, dedgeconv3x3, 3, imgheight, imgwidth);
			}
			cudaMemcpy(image, d_output, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToHost);
		}else if(inputImageDisplay.is_key1()){
                smoothsize = 3;
			printf("set smmoth size to 3!\n");
		}else if(inputImageDisplay.is_key2()){
			smoothsize = 7;
			printf("set smmoth size to 7!\n");
		}else if(inputImageDisplay.is_key3()){
			smoothsize = 9;
            printf("set smmoth size to 9!\n");
		}else if(inputImageDisplay.is_keyK()){
			sharpenfactor -= 0.1;
			cudaMemcpy(d_input, originimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
   	    	callsharpen(grid, block, d_output, sharpenfactor, d_input, dedgeconv3x3, 3, imgheight, imgwidth);
			cudaMemcpy(image, d_output, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToHost);
		}else if(inputImageDisplay.is_keyI()){
			sharpenfactor += 0.1;
			cudaMemcpy(d_input, originimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
   		    callsharpen(grid, block, d_output, sharpenfactor, d_input, dedgeconv3x3, 3, imgheight, imgwidth);
			cudaMemcpy(image, d_output, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToHost);
		}else if(inputImageDisplay.is_keyG()){
			cudaMemcpy(d_input, originimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
			if(gauss){
				printf("shutdown gauss\n");
				gauss = false;
  		    cudaMemcpy(d_output, d_input, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToDevice);
			}else{
				printf("open gauss\n");
				gauss = true;
	  		callgaussian(grid, block, d_output, d_input, dgaconv7x7, 7, imgheight, imgwidth);
			}
			cudaMemcpy(image, d_output, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToHost);
		}else if(inputImageDisplay.is_keyESC()){ // quit
			break;
		}
		inputImageDisplay.display(image);
	}

	// save test output image
	visualization.save("./test_output.bmp");
	image.save("./test_image.png");

  // profiling part 1
	for(int i=6; i<=10; i++){
		int imagesize = 2 << i, imgproduct = imagesize*imagesize;
		printf("profiling image size %d \n", imagesize);
		cimg_library::CImg<unsigned char> square(imagesize, imagesize, 1, 3, 0);
		unsigned char *d_input, *d_output;
		cudaMalloc((void **)&d_input, imagesize*imagesize*3*sizeof(unsigned char));
		cudaMalloc((void **)&d_output, imagesize*imagesize*3*sizeof(unsigned char));
		cudaMemcpy(d_input, square, imagesize*imagesize*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
		dim3 tmpgrid = dim3((int)(imagesize/16), (int)(imagesize/16));
		dim3 tmpblock = dim3(16, 16);
	    printf("grid 1 512, block 512 1\n");
		callprofilingconvlayer(dim3(1,512), dim3(512,1), d_output, d_input, dsmconv3x3, 3, 1, square.height(), square.width());
		printf("grid %d %d blolck 16 16\n", imagesize/16, imagesize/16);
		cudaMemcpy(d_input, square, imagesize*imagesize*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
		callprofilingconvlayer(tmpgrid, tmpblock, d_output, d_input, dsmconv3x3, 3, 1, square.height(), square.width());
	    cudaMemcpy(d_input, square, imagesize*imagesize*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
		printf("profiling2\n");
		callprofilingconvlayer2(tmpgrid, tmpblock, d_output, d_input, dsmconv3x3, 3, 1, 1, square.height(), square.width());
		cudaFree(d_input);
		cudaFree(d_output);
	}

  // profiling part 2
	for(int i=3; i<10; i+=2){
		printf("using gaussian kernel size %d \n", i);
		float* gkernel;
		getgaussian(&gkernel, i);
		int imagesize = 1024;
		cimg_library::CImg<unsigned char> square(1024, 1024, 1, 3, 0);
		unsigned char *d_input, *d_output;
		float *dkernel;
		cudaMalloc((void **)&d_input, imagesize*imagesize*3*sizeof(unsigned char));
		cudaMalloc((void **)&d_output, imagesize*imagesize*3*sizeof(unsigned char));
		cudaMalloc((void **)&dkernel, i*i*sizeof(float));
		cudaMemcpy(dkernel, gkernel, i*i*sizeof(float), cudaMemcpyHostToDevice);
		cudaMemcpy(d_input, square, imagesize*imagesize*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
		dim3 tmpgrid = dim3((int)(imagesize/16), (int)(imagesize/16));
		dim3 tmpblock = dim3(16, 16);
//		printf("grid 1 512, block 512 1\n");
        cudaDeviceSynchronize();
		printf("grid %d %d blolck 16 16\n", imagesize/16, imagesize/16);
		callprofilingconvlayer(tmpgrid, tmpblock, d_output, d_input, dkernel, i, 1, square.height(), square.width());
		printf("profiling2 \n");
		callprofilingconvlayer2(tmpgrid, tmpblock, d_output, d_input, dkernel, i, 1, 1, square.height(), square.width());
		cudaFree(d_input);
		cudaFree(d_output);
		free(gkernel);
	}

  // convolutional part
	int length = 64*3*3*3;
	std::string filename = "../../data/vgg16_layer_1.raw";
	FILE* fhandle = fopen(filename.c_str(),"rb");
	float *vec = (float*)malloc(length*sizeof(float));
	fread((void*)vec, sizeof(float), length, fhandle);
    float *dvec;
	cudaMalloc((void **)&dvec, length*sizeof(float));
	cudaMemcpy(dvec, vec, length*sizeof(float), cudaMemcpyHostToDevice);
	// conv layer 
	bool convolutionallayer = false;
 	cimg_library::CImg<unsigned char> rdpartimage("../../data/images/lichtenstein_full.bmp");
	cimg_library::CImg<unsigned char> originrdpartimage("../../data/images/lichtenstein_full.bmp");
	grid = dim3(1,512);
	block= dim3(512,1);
	std::string prefix = "./../data/";
	std::string suffix = ".jpg";
    std::string rdfilename;
	while (!inputImageDisplay.is_closed() && !visualizationDisplay.is_closed()) {
		inputImageDisplay.wait();
		// first copy current 3rdpartimage to device
		cudaMemcpy(d_input, rdpartimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
		if (inputImageDisplay.button() && inputImageDisplay.mouse_y() >= 0) {
			// on click redraw visualization
			const int y = inputImageDisplay.mouse_y();
			visualization.fill(0).draw_graph(rdpartimage.get_crop(0, y, 0, 0, rdpartimage.width() - 1, y, 0, 0), red, 1, 1, 0, 255, 0);
			visualization.draw_graph(rdpartimage.get_crop(0, y, 0, 1, rdpartimage.width() - 1, y, 0, 1), green, 1, 1, 0, 255, 0);
			visualization.draw_graph(rdpartimage.get_crop(0, y, 0, 2, rdpartimage.width() - 1, y, 0, 2), blue, 1, 1, 0, 255, 0).display(visualizationDisplay);
		}
		// apply convolutionallayer
		else if(inputImageDisplay.is_keyC()){ 
			for(int i=0; i<64; i++)
			{
				cudaMemcpy(d_input, originrdpartimage, imgproduct*3*sizeof(unsigned char), cudaMemcpyHostToDevice);
				callconvlayer(grid, block, d_output, d_input, dvec+3*3*3*i, 3, 1, rdpartimage.height(), rdpartimage.width());
				cudaMemcpy(rdpartimage, d_output, imgproduct*3*sizeof(unsigned char), cudaMemcpyDeviceToHost);
				// save test output image
				suffix = ".bmp";
				rdfilename = prefix + std::to_string(i)+suffix;
				rdpartimage.save(rdfilename.c_str());
			}
		}	
		else if(inputImageDisplay.is_keyESC()){ // quit
			break;
		}
		inputImageDisplay.display(rdpartimage);
	}
	return EXIT_SUCCESS;
}


