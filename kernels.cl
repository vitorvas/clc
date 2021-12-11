/** \file kernels.cl
* 
* The OpenCL kernels are all defined in this file. The compilation of any kernels
* is defined in the source file that invoke the kernels.
*
* Note: OpenCL kernels have no return values.
*
*/
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

/**
 * sum
 *
 * \brief Test kernel with a simple sum of vectors.
 */
__kernel void sum(__global float *a,
                   __global float *b,
                   __global float *c) {
   
   *c = *a + *b;
}
/**
 * convolution
 *
 * \brief This kernel performs a convolution between the source image and the
 * the "kernel" image.
 *
 * @param image2d_t, image2d_t 
*/
//__kernel void convolution(__global uint8 *im, __global uint8 *imres)
__kernel void convolution(__read_only image2d_t im, __write_only image2d_t imres)
{
   // Create a sampler inside the kernel
   const sampler_t samp = CLK_NORMALIZED_COORDS_FALSE|
                          CLK_ADDRESS_CLAMP_TO_EDGE| 
			  CLK_FILTER_NEAREST;

//   int i = get_global_id(0);
//   imres[i] = im[i];
     int2 coord = (int2)(get_global_id(0), get_global_id(1));
     int2 coordtest = (int2)(get_global_id(0)+10, get_global_id(1)+10);
     
     //     uint4 pix = {0, 0, 0, 255};
     uint nu = 0;
     //     uint4 pixel = read_imageui(im, samp, coord);
     //     uint4 pix = {0, 0, 0, 255};
     float nf = 1.0;

     // Remembering that read_imageui and write_imageui are OpenCL functions
     uint4 pixel = read_imageui(im, samp, coordtest);
     pixel[3] = (uint)pixel[3]+30;
     
     //	   nf = (i*j)/(256*2);
     //	   nu = (uint)nf;
     //     uint4 pix = {0, 0, 0, 100};
     write_imageui(imres, coord, pixel);

}

/**
 * convolution
 *
 * \brief This kernel performs a convolution between the source image and the
 * the "kernel" image.
 *
 * @param image2d_t, image2d_t 
*/
//__kernel void convolution(__global uint8 *im, __global uint8 *imres)
__kernel void convolution2(__read_only image2d_t im, __read_only image2d_t knl,
			   __write_only image2d_t imres)
{
  // Create a sampler inside the kernel
  const sampler_t samp = CLK_NORMALIZED_COORDS_FALSE|
    CLK_ADDRESS_CLAMP_TO_EDGE|
    CLK_FILTER_NEAREST;
  
  //   int i = get_global_id(0);
  //   imres[i] = im[i];
  //  int dx = 60;
  //  int dy = 0;
  
  int2 coord = (int2)(get_global_id(0), get_global_id(1));
  int2 coordtest = (int2)(0, 0);

  
  //     uint4 pix = {0, 0, 0, 255};
  uint nu = 0;
  //     uint4 pixel = read_imageui(im, samp, coord);
  //     uint4 pix = {0, 0, 0, 255};
  float nf = 1.0;
  
  // Remembering that read_imageui and write_imageui are OpenCL functions
  uint4 pixel = (0, 0, 0, 0); 

  int sx, sy = 3;
  for(int i=-5; i<6; i++)
  {
    for(int j=-5; j<6; j++)
      {
	coordtest=(int2)(get_global_id(0)+i, get_global_id(1)+j);
	pixel+=read_imageui(im, samp, coordtest);
      }
  }
  float tmppixel = 0.0;

  // ERRO! Estou mudando o sampling das coordenadas! Nada a ver!
  // Tenho que fazer o filtro é no READ_IMAGEUI ali embaixo
  tmppixel = pixel[3]/121; // 121 é a soma de 11*11 do for -5 a 6 (aberto)
  pixel[3] = (uint)tmppixel;
  
  //  pixel[3] = (uint)pixel[3];
  
  //	   nf = (i*j)/(256*2);
  //	   nu = (uint)nf;
  //     uint4 pix = {0, 0, 0, 100};
  write_imageui(imres, coord, pixel);
}
