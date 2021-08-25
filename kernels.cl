#pragma OPENCL EXTENSION cl_khr_fp64 : enable

__kernel void sum(__global float *a,
                   __global float *b,
                   __global float *c) {
   
   *c = *a + *b;
}

//__kernel void convolution(__global uint8 *im, __global uint8 *imres)
__kernel void convolution(__read_only image2d_t im, __write_only image2d_t imres)
{
   // Create a sampler inside the kernel
   const sampler_t samp = CLK_NORMALIZED_COORDS_FALSE|
   	 	   	  CLK_ADDRESS_CLAMP|
			  CLK_FILTER_NEAREST;

//   int i = get_global_id(0);
//   imres[i] = im[i];
     int2 coord = (int2)(get_global_id(0), get_global_id(1) );

     //     uint4 pix = {0, 0, 0, 255};
     uint nu = 0;
     //     uint4 pixel = read_imageui(im, samp, coord);
     //     uint4 pix = {0, 0, 0, 255};
     float nf = 1.0;
     //     for(int i=0; i<709; i++)
     //       {
     //       for(int j=0; j<500; j++)
     //	 {
     uint4 pixel = read_imageui(im, samp, coord);
     pixel[3] = (uint)pixel[3]-30;
     
     //	   nf = (i*j)/(256*2);
     //	   nu = (uint)nf;
     //     uint4 pix = {0, 0, 0, 100};
     write_imageui(imres, coord, pixel);
     //	 }
     //       } 


}

