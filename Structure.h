#include <CL/cl.h>
#define MAX_SOURCE_SIZE (0x100000)
#define MAX_NODES_DIGITS (15)

const char fileName[] = "./antKernel.cl";
const char nodeReplace[] = "NODESIZE";
const char nodePlusReplace[] = "NODESIZEPLUS1";

char * programSource;

typedef struct Params
{
    int Nodes;
    int StartNode;
    double Alpha;
    double Beta;
} Params;

cl_command_queue cmdQueue;
cl_context context = NULL;
cl_uint numDevices = 0;
cl_device_id *devices = NULL;
cl_program program = NULL;
cl_kernel kernel = NULL;

cl_mem bufferParam;  // parameters on the device
cl_mem bufferC;  // cost array on the device
cl_mem bufferP;  // Pheromone array on the device
cl_mem bufferR;  // Input array on the device
cl_mem bufferS;  // Output path from the device
cl_mem bufferSC;  // Output cost from the device

// Host data
Params params;
double *C   = NULL;  // Cost array
double *P   = NULL;  // Pheromone array
double *R   = NULL;     // Ants randomness
int *S      = NULL;     // Path taken
double *SC  = NULL; // cost of solution

size_t datasizeParams;
size_t datasizeC;
size_t datasizeP;
size_t datasizeR;
size_t datasizeS;
size_t datasizeSC;


size_t readInKernel(int nodes){
    FILE *fp;
    size_t source_size;

    /* Load kernel source file */
    fp = fopen(fileName, "r");
    if (!fp) {
        fprintf(stderr, "Failed to load kernel.\n");    
        exit(1);
    }   
    programSource = (char *)malloc(MAX_SOURCE_SIZE);
    source_size = fread(programSource, 1, MAX_SOURCE_SIZE, fp);

    char strNodes[MAX_NODES_DIGITS];
    sprintf(strNodes, "%d", (nodes+1));
    char *replaceOut;
    replaceOut = replace_str(programSource, nodePlusReplace, strNodes);
    while(replaceOut != NULL){
        programSource = replaceOut;
        replaceOut = replace_str(programSource, nodePlusReplace, strNodes);
    }
    sprintf(strNodes, "%d", nodes);
    replaceOut = replace_str(programSource, nodeReplace, strNodes);
    while(replaceOut != NULL){
        programSource = replaceOut;
        replaceOut = replace_str(programSource, nodeReplace, strNodes);
    }
    fclose(fp);
    return source_size;
}

cl_int platformsAndDevices(cl_int status){
    //-----------------------------------------------------
    // STEP 1: Discover and initialize the platforms
    //-----------------------------------------------------

    cl_uint numPlatforms = 0;
    cl_platform_id *platforms = NULL;

    // Use clGetPlatformIDs() to retrieve the number of 
    // platforms
    status |= clGetPlatformIDs(0, NULL, &numPlatforms);

    // Allocate enough space for each platform
    platforms =   
        (cl_platform_id*)malloc(
            numPlatforms*sizeof(cl_platform_id));

    // Fill in platforms with clGetPlatformIDs()
    status |= clGetPlatformIDs(numPlatforms, platforms, 
                NULL);

    //-----------------------------------------------------
    // STEP 2: Discover and initialize the devices
    //----------------------------------------------------- 

    int platformToUse = 0;
    // int platformToUse = 1;
    cl_device_type dType = CL_DEVICE_TYPE_ALL;
    // cl_device_type dType = CL_DEVICE_TYPE_GPU;

    // Use clGetDeviceIDs() to retrieve the number of 
    // devices present
    status |= clGetDeviceIDs(
        platforms[0], 
        dType, 
        0, 
        NULL, 
        &numDevices);

    // Allocate enough space for each device
    devices = 
        (cl_device_id*)malloc(
            numDevices*sizeof(cl_device_id));

    printf("NUM %d\n", numDevices);

    // Fill in devices with clGetDeviceIDs()
    status |= clGetDeviceIDs(
        platforms[platformToUse], 
        dType,        
        numDevices, 
        devices, 
        NULL);
    //-----------------------------------------------------
    // STEP 3: Create a context
    //----------------------------------------------------- 


    // Create a context using clCreateContext() and 
    // associate it with the devices
    context = clCreateContext(
        NULL, 
        numDevices, 
        devices, 
        NULL, 
        NULL, 
        &status);

    //-----------------------------------------------------
    // STEP 4: Create a command queue
    //----------------------------------------------------- 


    // Create a command queue using clCreateCommandQueue(),
    // and associate it with the device you want to execute 
    // on
    cmdQueue = clCreateCommandQueue(
        context, 
        devices[0], 
        0, 
        &status);

    free(platforms);
}

cl_int createAntBuffers(cl_int status){

    // if doing CL_MEM_READ_WRITE

    bufferParam = clCreateBuffer(
        context,
        CL_MEM_READ_ONLY,
        sizeof(Params),
        NULL,
        &status);

    bufferC = clCreateBuffer(
        context, 
        CL_MEM_READ_ONLY,                         
        datasizeC, 
        NULL, 
        &status);

    bufferP = clCreateBuffer(
        context, 
        CL_MEM_READ_ONLY,                         
        datasizeP, 
        NULL, 
        &status);

    bufferR = clCreateBuffer(
        context, 
        CL_MEM_READ_ONLY,                         
        datasizeR, 
        NULL, 
        &status);

    bufferS = clCreateBuffer(
        context, 
        CL_MEM_WRITE_ONLY,                 
        datasizeS, 
        NULL, 
        &status);

    bufferSC = clCreateBuffer(
        context, 
        CL_MEM_WRITE_ONLY,                 
        datasizeSC, 
        NULL, 
        &status);

    //-----------------------------------------------------
    // STEP 6: Write host data to device buffers
    //----------------------------------------------------- 

    status |= clEnqueueWriteBuffer(
        cmdQueue, 
        bufferC, 
        CL_FALSE, 
        0, 
        datasizeC,                         
        C, 
        0, 
        NULL, 
        NULL);


    status |= clEnqueueWriteBuffer(
        cmdQueue, 
        bufferR, 
        CL_FALSE, 
        0, 
        datasizeR,                                  
        R, 
        0, 
        NULL, 
        NULL);

    status |= clEnqueueWriteBuffer(
        cmdQueue, 
        bufferParam, 
        CL_TRUE, 
        0, 
        datasizeParams,                                  
        &params, 
        0, 
        NULL, 
        NULL);
    return status;
}

cl_int createProgram(cl_int status){
    //-----------------------------------------------------
    // STEP 7: Create and compile the program
    //----------------------------------------------------- 
    program = clCreateProgramWithSource(
        context, 
        1, 
        (const char**)&programSource,                                 
        NULL, 
        &status);

    status |= clBuildProgram(
        program, 
        numDevices, 
        devices, 
        NULL, 
        NULL, 
        NULL);

    return status;
}

cl_int queueAntStroll(cl_int status, int k){
    status |= clEnqueueWriteBuffer(cmdQueue, bufferP, CL_FALSE, 0, datasizeP, P, 0, NULL, NULL);

    kernel = clCreateKernel(program, "stroll", &status);

    status  |= clSetKernelArg(
        kernel, 
        0, 
        sizeof(cl_mem), 
        &bufferParam);
    status  |= clSetKernelArg(
        kernel, 
        1, 
        sizeof(cl_mem), 
        &bufferC);
    status |= clSetKernelArg(
        kernel, 
        2, 
        sizeof(cl_mem), 
        &bufferP);
    status |= clSetKernelArg(
        kernel, 
        3, 
        sizeof(cl_mem), 
        &bufferR);
    status |= clSetKernelArg(
        kernel, 
        4, 
        sizeof(cl_mem), 
        &bufferS);
    status |= clSetKernelArg(
        kernel, 
        5, 
        sizeof(cl_mem), 
        &bufferSC);

    //-----------------------------------------------------
    // STEP 10: Configure the work-item structure
    //-----------------------------------------------------   
    size_t globalWorkSize[1];    
    globalWorkSize[0] = k;

    //-----------------------------------------------------
    // STEP 11: Enqueue the kernel for execution
    //----------------------------------------------------- 
    
    // Execute the kernel by using 
    // clEnqueueNDRangeKernel().
    // 'globalWorkSize' is the 1D dimension of the 
    // work-items
    status |= clEnqueueNDRangeKernel(
        cmdQueue, 
        kernel, 
        1, 
        NULL, 
        globalWorkSize, 
        NULL, 
        0, 
        NULL, 
        NULL);
}


cl_int readOutput(cl_int status){
    //-----------------------------------------------------
    // STEP 12: Read the output buffer back to the host
    //----------------------------------------------------- 
    
    // Use clEnqueueReadBuffer() to read the OpenCL output  
    // buffer (bufferC) 
    // to the host output array (C)
    status |= clEnqueueReadBuffer(
        cmdQueue, 
        bufferS, 
        CL_TRUE, 
        0, 
        datasizeS, 
        S, 
        0, 
        NULL, 
        NULL);
    status |= clEnqueueReadBuffer(
        cmdQueue, 
        bufferSC, 
        CL_TRUE, 
        0, 
        datasizeSC, 
        SC, 
        0, 
        NULL, 
        NULL);
    
    return status;
}