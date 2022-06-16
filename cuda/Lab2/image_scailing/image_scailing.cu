#include<stdio.h>
#include"scrImagePgmPpmPackage.h"
#include "gputimer.h"

//Kernel which calculate the resized image
__global__ void createResizedImage(unsigned char *imageScaledData, int scaled_width, float scale_factor, cudaTextureObject_t texObj)
{
	const unsigned int tidX = blockIdx.x*blockDim.x + threadIdx.x;
	const unsigned int tidY = blockIdx.y*blockDim.y + threadIdx.y;
	const unsigned index = tidY*scaled_width+tidX;
       	
	// Step 4: Read the texture memory from your texture reference in CUDA Kernel
	imageScaledData[index] = tex2D<unsigned char>(texObj,(float)(tidX*scale_factor),(float)(tidY*scale_factor));
}

int main(int argc, char*argv[])
{
	// Scaling ratio, Block size, Grid size
	int height=0, width =0, scaled_height=0,scaled_width=0;
	//Define the scaling ratio	
	float scaling_ratio=0.5;

	if (argc <= 1) 
		printf("Please enter img path!\n");

	char *img_name = argv[1];

	if(argc > 2)
		sscanf(argv[2],"%f",&scaling_ratio);

	int block_size = 32;

	if (argc > 3) 
        block_size = atoi(argv[3]);

	GpuTimer timer;
    timer.Start();

	unsigned char*data;
	unsigned char*scaled_data,*d_scaled_data;

	char *inputStr = img_name;
	char outputStr[1024] = {"scaled.pgm"};
	cudaError_t returnValue;

	//Create a channel Description to be used while linking to the tecture
	cudaArray* cu_array;
	cudaChannelFormatKind kind = cudaChannelFormatKindUnsigned;
	cudaChannelFormatDesc channelDesc = cudaCreateChannelDesc(8, 0, 0, 0, kind);

	get_PgmPpmParams(inputStr, &height, &width);	//getting height and width of the current image
	data = (unsigned char*)malloc(height*width*sizeof(unsigned char));
	// printf("\n Reading image width height and width [%d][%d]", height, width);
	scr_read_pgm( inputStr , data, height, width );//loading an image to "inputimage"

	scaled_height = (int)(height*scaling_ratio);
	scaled_width = (int)(width*scaling_ratio);
	scaled_data = (unsigned char*)malloc(scaled_height*scaled_width*sizeof(unsigned char));
	// printf("\n scaled image width height and width [%d][%d]", scaled_height, scaled_width);

	//Allocate CUDA Array
 	returnValue = cudaMallocArray( &cu_array, &channelDesc, width, height);
	// returnValue = (cudaError_t)(returnValue | cudaMemcpy( cu_array, data, height * width * sizeof(unsigned char), cudaMemcpyHostToDevice));
	returnValue = (cudaError_t)(returnValue | cudaMemcpyToArray( cu_array, 0, 0, data, height * width * sizeof(unsigned char), cudaMemcpyHostToDevice));

	if(returnValue != cudaSuccess)
		printf("\n Got error while running CUDA API Array Copy");

	// Step 1. Specify texture
	struct cudaResourceDesc resDesc;
	memset(&resDesc, 0, sizeof(resDesc));
	resDesc.resType = cudaResourceTypeArray;
	resDesc.res.array.array = cu_array;
	// Step 2. Specify texture object parameters
	struct cudaTextureDesc texDesc;
	memset(&texDesc, 0, sizeof(texDesc));
	texDesc.addressMode[0] = cudaAddressModeClamp;
	texDesc.addressMode[1] = cudaAddressModeClamp;
	texDesc.filterMode = cudaFilterModePoint;
	texDesc.readMode = cudaReadModeElementType;
	texDesc.normalizedCoords = 0;

	// Step 3: Create texture object
	cudaTextureObject_t texObj = 0;
	cudaCreateTextureObject(&texObj, &resDesc, &texDesc, NULL);

	if(returnValue != cudaSuccess) 
		printf("\n Got error while running CUDA API Bind Texture");
	
	cudaMalloc(&d_scaled_data, scaled_height*scaled_width*sizeof(unsigned char) );


	dim3 dimBlock(block_size, block_size,1);
	dim3 dimGrid(scaled_width/dimBlock.x + 1,scaled_height/dimBlock.y + 1,1);

	// printf("\n Launching grid with blocks [%d][%d] ", dimGrid.x,dimGrid.y);

	createResizedImage<<<dimGrid, dimBlock>>>(d_scaled_data,scaled_width,1/scaling_ratio, texObj);

	returnValue = (cudaError_t)(returnValue | cudaDeviceSynchronize());

	returnValue = (cudaError_t)(returnValue |cudaMemcpy (scaled_data , d_scaled_data, scaled_height*scaled_width*sizeof(unsigned char), cudaMemcpyDeviceToHost ));
	if(returnValue != cudaSuccess) 
		printf("\n Got error while running CUDA API kernel");

	// Step 5: Destroy texture object
	cudaDestroyTextureObject(texObj);
	
	scr_write_pgm( outputStr, scaled_data, scaled_height, scaled_width, "####" ); //storing the image with the detections
		
	if(data != NULL)
		free(data);
	if(cu_array !=NULL)
		cudaFreeArray(cu_array);
	if(scaled_data != NULL)
		free(scaled_data);
	if(d_scaled_data!=NULL)
		cudaFree(d_scaled_data);

    timer.Stop();
	float timer_result = timer.Elapsed();

    // printf("image;scale;block_size;time\n");
    printf("%s;%f;%d;%f\n", img_name, scaling_ratio, block_size, timer_result);

	return 0;
}
