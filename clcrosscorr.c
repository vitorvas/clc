#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include"../../stb/stb_image.h"
#include"../../stb/stb_image_write.h"

#define CL_TARGET_OPENCL_VERSION 110

#include<CL/cl.h>
#include"clcrosscorr.h"

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<time.h> // To use clock()

int main(int argc, char* argv[])
{
  int err=0, w=0, h=0;

  // Le imagem stb
  size_t c; //components: 3 RGB, 1 Grayscale

  /*  if(argc < 2) {
    c = atoi(argv[2]);
    //    n = std::stoi(argv[3]);
    printf("Esqueceu o argumento, né minha filha?\n");
    return -1;
  }
  if(argc == 3) {
    c = atoi(argv[2]);
  }
  else */
  c=1;
  char nome[11]="marble8.png";

  // Estruturas de dados para as imagens
  //  uint8_t* img = stbi_load(argv[1], &w, &h, NULL, c);
  uint8_t* img = stbi_load(nome, &w, &h, NULL, c);
  uint8_t* arr = malloc(sizeof(uint8_t)*w*h*c);
  //  uint8_t* arr = calloc(w*h*c, sizeof(uint8_t));
  
  // Get device info
  cl_bool res;

  // Copia a imagem para o vector
  float tmp = 0.0;
  
  /*  for(size_t i=0; i<w*h*c; i++)//=i+8)
  {
    tmp = img[i]*0.95;
    //arr[i] = (uint8_t)tmp;
    arr[i] = (uint8_t)255;
    }*/
  
  cl_platform_id platform;
  cl_device_id device;
  cl_context context;
  cl_command_queue queue;
  cl_kernel kernel;
  cl_int clerr;
  
  clGetPlatformIDs(1, &platform, NULL);
  clerr = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
  if(clerr == CL_DEVICE_NOT_FOUND)
      clerr = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);
  
  context = clCreateContext(NULL, 1, &device, NULL, NULL, &clerr);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-3);
    exit(0);
  }

  cl_bool query;
  clerr = clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, query, NULL, NULL);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-3);
    exit(0);
  }

  char devname[255] = {'\0'};
  clGetDeviceInfo(device, CL_DEVICE_NAME, 255, devname, NULL);

  printf("Device name: %s\n", &devname);
  cl_image_format format;
  format.image_channel_order = CL_A; // o Cochon só aceita CL_A, CL_RGBA, CL_ARGB, CL_BGRA
  format.image_channel_data_type = CL_UNSIGNED_INT8;

  // Ver os formatos de imagem suportados
  /*  cl_image_format formlist[30];
  clerr = clGetSupportedImageFormats(context, CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE2D,
				     30, formlist, NULL);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-3);
    exit(0);
  }
  for(int i=0; i<30; i++)
  {
    printf(" ------ %d) image_channel_order %x, image_channel_data_type %x. \n", i, formlist[i].image_channel_order, formlist[i].image_channel_data_type);
  }
  */
  
  // Já tem a fila, agora tem que ler o programa e depois gerar o programa
  size_t psize[1];
  FILE *fp;
  cl_program program;
  char *buffer[1];
  //  char buf[92] = "__kernel void sum(__global float *a, __global float *b, __global float *c) {*c = *a + *b;}";
  
  fp = fopen("kernels.cl", "r");
  fseek(fp, 0, SEEK_END);
  psize[0] = ftell(fp);
  rewind(fp);
  
  // Allocate variable for source code
  // buffer = (char*)malloc(psize*sizeof(char));
  buffer[0] = malloc(psize[0]*sizeof(buffer[0]));
  //  buffer[psize] = '\0';
  fread(buffer[0], sizeof(char), psize[0], fp);
  fclose(fp);

  // --- Outra forma de calcular o tamanho do arquivo
  // stat is a system call to get file status
  // man stat(2)
  /*  struct stat statbuf;
  stat(fileName, &statbuf);
  
  char *kernel = (char *) malloc(statbuf.st_size + 1);
  fread(kernel, statbuf.st_size, 1, fh);
  kernel[statbuf.st_size] = '\0';*/

  // Imprime o programa
  //  printf("--- PROGRAM ---\n");
  //  for(size_t i=0; i<psize; i++)
  //    printf("%c", buffer[i]);
  //  printf("\n");
  
  program = clCreateProgramWithSource(context, 1, (const char**)&buffer, NULL, &clerr);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-3);
    exit(0);
  }

  // Agora dar um build no programa, pegando log e vendo se deu build corretamente
  printf(" --- Building program... ");

  clock_t t;
  t = clock();
  clerr = clBuildProgram(program, 0, NULL, "-cl-std=CL1.1", NULL, NULL);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-3);
  }
  else
  {
    t = clock()-t;
    printf("Ok! in %f seconds.\n", (double)t/CLOCKS_PER_SEC);
  }
  
  // Detalhes do programa built
  // Na primeira pega o tamanho da string para alocá-la.
  size_t log_size;
  char *str_log;
  
  clerr = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-3);
    exit(0);
  }

  str_log = malloc(log_size*sizeof(str_log));

  clerr = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, str_log, NULL);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-3);
    exit(0);
  }
  for(uint i=0; i<log_size; i++)
    printf("%c", str_log[i]);
  printf("\n");
  free(str_log);
  
  // Aqui pega detalhes do programa. O que eu queria mesmo era o detalhe do build
  size_t program_size;
  clerr = clGetProgramInfo(program, CL_PROGRAM_SOURCE, 0, NULL, &program_size);

  str_log = malloc(program_size*sizeof(str_log));
  
  // Depois pega efetivamente a string
  clerr = clGetProgramInfo(program, CL_PROGRAM_SOURCE, program_size, str_log, NULL);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-3);
    exit(0);
  }
  /*  for(uint i=0; i<program_size; i++)
    printf("%c", str_log[i]);
    printf("\n"); */
  free(str_log);

  // Criar o kernel
  kernel = clCreateKernel(program, "convolution", NULL);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-3);
    exit(0);
  }
  
  
  // Aprendendo a usar as estruturas para imagens
  // host: cl_mem/cl_sampler
  // device: image2d_t/sampler_t
  
  //cl_image_desc imdesc;

  // Não implementado na versão do OpenCL do Cochon
  // FUNCIONA a linha abaixo. O problema era: CL_MEM_USE_HOST_PTR. Passei para COPY em USE e deu
  // CL_MEM_READ_WRITE e CL_MEM_READ_ONLY funcionam, ambos.
  // Não funciona compilando com o clang, só com o gcc. Com os mesmos parâmetros.
  cl_mem image = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
				 &format, w, h, w*sizeof(uint8_t), (void*)img, &clerr);
  
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-4);
    exit(0);
  }

  // Cria a imagem destino
  cl_mem image_dest = clCreateImage2D(context, CL_MEM_WRITE_ONLY,
				      &format, w, h, 0, NULL, &clerr);
  //  				      &format, w, h, w*sizeof(uint8_t), NULL, &clerr);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-4);
    exit(0);
  }
  
  // Criar um cl_sampler (pag 125 do livro) de teste
  // clCreateSampler tá descontinuado desde a versao 2.0 do OpenCl. Usar clCreateSamplerWithProperties
  cl_addressing_mode mode = CL_ADDRESS_CLAMP;
  cl_filter_mode fmode = CL_FILTER_NEAREST;

  // CL_FALSE para não normalizar as coordenadas, que é o default
  cl_sampler sampler = clCreateSampler(context, CL_FALSE, mode, fmode, &clerr);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-4);
    exit(0);
  }

  // Criar a fila
  //  queue = clCreateCommandQueue(context, device, 0, &clerr);
  queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &clerr);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-3);
    exit(0);
  }

  // A ideia é só copiar o conteúdo de uma imagem em outra pra testar antes de fazer
  // muita coisa
  // O kernel já foi criado. Antes de chamar (enqueue) o kernel com as imagens
  // tem que setar os argumentos do kernel.
  clSetKernelArg(kernel, 0, sizeof(cl_mem), &image);
  clSetKernelArg(kernel, 1, sizeof(cl_mem), &image_dest);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-3);
    exit(0);
  }
  
  /*------ IMPORTANTE: o clEnqueueTask está deprecado e é tosco: chama
    com workgroup 1, sem ser pararlelo. Nao faz sentido usá-lo.
    clerr = clEnqueueTask(queue, kernel, 0, NULL, NULL);*/
  uint8_t workdim = w*h*c;
  //size_t work_dim[] = {w*h*c, 0, 0};
  size_t work_dim[] = {w, h, 1};

  // O cl_event é iniciado aqui, mas é parte da estrutura para fazer o profiling
  cl_event event;

  clerr = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, work_dim, NULL, 0, NULL, &event);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-3);
    exit(0);
  }

  // Mandatory to profile since clEnqueueNDRangeKernel is not blocking
  clFinish(queue);
  
  // Profiling the kernel call
  cl_ulong start, end;
  float executionTimeInMilliseconds;
  
  clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_QUEUED,
			  sizeof(cl_ulong), &start, NULL);
  clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_SUBMIT,
			  sizeof(cl_ulong), &end, NULL);
  executionTimeInMilliseconds = (end - start) * 1.0e-6f;
  printf("clEnqueueNDRangeKernel execution time: %f (ms)\n", executionTimeInMilliseconds);


  // OpenCL images behave differently from buffers
  // Read data from device image
  //  clerr = clEnqueueReadBuffer();
  cl_bool b = CL_TRUE;
  size_t origin[] = {0,0,0};
  //  clerr = clEnqueueReadImage(queue, image_dest, f, origin, work_dim, w*sizeof(uint8_t), 0, arr, 0, NULL, NULL);
  clerr = clEnqueueReadImage(queue, image_dest, b, origin, work_dim, 0, 0, arr, 0, NULL, &event);
  if(clerr != CL_SUCCESS)
  {
    printf(" ---- CL Error: %s (line %d)\n", clGetErrorString(clerr), __LINE__-3);
    exit(0);
  }

  
  // Para medir o tempo de GPU (Guia da NVIDIA: OpenCL_Best_Practices_Guide.pdf (meu diretorio)
  // O profiling tem que ser depois do envio pra fila
  clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START,
			  sizeof(cl_ulong), &start, NULL);
  clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END,
			  sizeof(cl_ulong), &end, NULL);
  executionTimeInMilliseconds = (end - start) * 1.0e-6f;
  printf("clEnqueueReadImage execution time: %f (ms)\n", executionTimeInMilliseconds);
    
  // Salva imagem stb
  if(stbi_write_png("result.png", w, h, c, arr, 0)==0)
  {
    printf("(E) Erro ao gravar imagem (stbi_write_png(...))\n");
    return -1;
  }

  free(buffer[0]);
  
  clReleaseSampler(sampler);
  clReleaseMemObject(image);
  clReleaseMemObject(image_dest);
  clReleaseCommandQueue(queue);

  stbi_image_free(img);
  free(arr);
  
  return 0;
}
