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
 * \brief This kernel performs a linear operation on image im and returns
 * the modified image imres.
 *
 * @param image2d_t, image2d_t 
*/
//__kernel void convolution(__global uint8 *im, __global uint8 *imres)
__kernel void filter(__read_only image2d_t im, __write_only image2d_t imres)
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

     /* int sx, sy = 3; */
     /* for(int i=-5; i<6; i++) */
     /*   { */
     /* 	 for(int j=-5; j<6; j++) */
     /* 	   { */
     /* 	     coordtest=(int2)(get_global_id(0)+i, get_global_id(1)+j); */
     /* 	     pixel+=read_imageui(im, samp, coordtest); */
     /* 	   } */
     /*   } */

     // ERRO! Estou mudando o sampling das coordenadas! Nada a ver!
     // Tenho que fazer o filtro é no READ_IMAGEUI ali embaixo

     // CORES de 0 a 2
     //       float tmppixel = 0.0;

     //     tmppixel = pixel[0]/121; // 121 é a soma de 11*11 do for -5 a 6 (aberto)
     //     pixel[0] = (uint)tmppixel;
     //     tmppixel = pixel[1]/121; // 121 é a soma de 11*11 do for -5 a 6 (aberto)
     //     pixel[1] = (uint)tmppixel;
     //     tmppixel = pixel[2]/121; // 121 é a soma de 11*11 do for -5 a 6 (aberto)
     //     pixel[2] = (uint)tmppixel;
     
     //  tmppixel = pixel[3]/121; // 121 é a soma de 11*11 do for -5 a 6 (aberto)
     //  pixel[3] = (uint)tmppixel;
     //     pixel[3] = 255;
     
     //	   nf = (i*j)/(256*2);
     //	   nu = (uint)nf;
     //     uint4 pix = {0, 0, 0, 100};
     //write_imageui(imres, coord, pixel);
     
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
__kernel void convolution(__read_only image2d_t im, __read_only image2d_t knl,
			  __global char* vec)
//  __global uint* vec) // Isso aqui muda a imagem final! E ainda fica 100bytes maior...
  
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
   int pitch = get_image_width(im); 
  
   int knl_width = 0; 
   int knl_height = 0; 

   knl_width = get_image_width(knl); 
   knl_height = get_image_height(knl); 

  // Test for kernel lenghts odd or even and
  // adapt the "pitch" for the loop
  // If the kernel is even, the operation will be biased
  // i.e., more neighbours on the right side will be taken in account
  
   uint dx = 0; 
   uint dy = 0; 

   if(knl_width%2) 
     dx = (knl_width/2)-1; 
   else 
     dx = knl_width/2; 

   if(knl_height%2) 
     dy = (knl_height/2)-1; 
   else 
     dy = knl_height/2; 

      
  //  uint nu = 0;
  uint pixel = read_imageui(im, samp, coord).w;
  //uint4 pix = {0, 0, 0, 255};
  uint pix = 0;
  //  float nf = 1.0;
  
  __private uint tmp;
  //   uint tmp;
  tmp = 0;
  /* total[0] = 0; 
     total[1] = 0; 
     total[2] = 0; 
     total[3] = 0; */
  
  // Remembering that read_imageui and write_imageui are OpenCL functions
  // uint4 pixel = (get_global_id(0), get_global_id(1), 0, 0); 

  int sx, sy = 3;
  for(int i=0; i<knl_width; i++)
    //for(int i=0; i<5; i++)
  {
    for(int j=0; j<knl_height; j++)
    //    for(int j=0; j<5; j++)
    {
      // Tem que usar read_image_ui pra ler as posições na knl
      coordtest=(int2)(get_global_id(0)+i-dx, get_global_id(1)+j-dy);
      //      coordtest=(int2)(get_global_id(0)+i-2, get_global_id(1)+j-2);
      // Accumulate
      //
      // IMPORTANT
      // --------------------------
      //
      // O read_imageui chamado no loop é que deixa ultra-lento quando chamo o kernel assim!
      pix = read_imageui(knl, samp, coordtest).w; // Return only one component
      // Isso aqui deve estar facil estourando o valor máximo pra int. Tenho que mudar pra usar
      // Normalizado na imagem.
      tmp+=pix*pixel;
    }
  }
  //vec[coordtest[0]+(pitch)*coordtest[1]]=total[0];
  vec[get_global_id(1)*pitch+get_global_id(0)]= tmp/(knl_width*knl_height);
}

/**
 * test wrote
 *
 * \brief Write global_id data to a vector
 */
__kernel void test(__global float* vec) {

  size_t x, y;
  x = get_global_id(0);
  y = get_global_id(1);

  uint width;

  // get_work_dim() de acordo com o numero de work items
  // especificado na execucao do kernel
  width = get_work_dim();
  
  // y*width + x = position
  vec[y*width+x]=(float)y*width+x;
}
