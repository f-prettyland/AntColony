#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "BorrowedFunc.h"

#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)
#define MAX_NODES_DIGITS (15)

const char fileName[] = "./antKernel.cl";
const char nodeReplace[] = "NODESIZE";
const char nodePlusReplace[] = "NODESIZEPLUS1";

typedef struct Params
{
    int Nodes;
    int StartNode;
    double Alpha;
    double Beta;
} Params;

void updatePheremones(int *S, double *P, double evap, double *SC, int k, int nodes) {
    //evaporate
    for (int i = 0; i < k; ++i)
    {
        for (int j = 0; j < nodes; ++j)
        {
            P[(i*nodes)+j] = ((1-evap)*P[(i*nodes)+j]);
        }
    }

    // drop pheremones proportional to success of path
    for (int i = 0; i < k; ++i)
    {
        for (int j = 0; j < nodes; ++j)
        {
            int citysrc = S[(i*k)+j];
            int cityDst = S[(i*k)+j+1];
            P[(citysrc*nodes)+cityDst] += (1.f/SC[i]);
        }
    }
}

int main() {
    // This code executes on the OpenCL host
    const int k = 5;

    // Host data
    double *C   = NULL;  // Cost array
    double *P   = NULL;  // Pheromone array
    double *R   = NULL;     // Ants randomness
    int *S      = NULL;     // Path taken
    double *SC  = NULL;// cost of solution

    //TODO somehow track number of ants outputting this soln?
    //      This can be calc'd afterwards

    // Nodes to represent in each array
    const int nodes = 5;
    // Constant to start pheromone on
    const double pherStart = 0.2;

    const double evap = 0.5;

    Params params;
    params.Nodes = nodes;

    params.StartNode = 0;
    //cost weighting
    params.Alpha = 1;
    //pheromone weighting
    params.Beta = 1;
    
    // Compute the size of the data 
    size_t datasizeParams   = sizeof(Params);

    size_t datasizeC    = sizeof(double)*nodes*nodes;
    size_t datasizeP    = sizeof(double)*nodes*nodes;
    //Need to make a decision at each node so need a random seed at each one of the
    size_t datasizeR    = sizeof(double)*nodes;
    size_t datasizeS    = sizeof(int)*(nodes+1)*k;
    size_t datasizeSC   = sizeof(double)*k;

    // Allocate space for input/output data
    C   = (double*)malloc(datasizeC);
    P   = (double*)malloc(datasizeP);
    R   = (double*)malloc(datasizeR);
    // S   = (int*)malloc(datasizeS);
    S   = (int*)calloc ((nodes+1)*k, sizeof(int));
    SC  = (double*)malloc(datasizeSC);

    // Initialize the input pheromones
    for(int j = 0; j < nodes; j++) {
        for(int i = 0; i < nodes; i++) {
            P[(j*nodes)+i] = pherStart;
        }
    }

    // Initialize the random
    for(int j = 0; j < nodes; j++) {
        R[j] = j+123;
    }

    // todo, calloc this?
    for(int j = 0; j < nodes; j++) {
        for(int i = 0; i < nodes; i++) {
            C[(j*nodes)+i] = 0;
        }
    }

    C[(0*nodes)+1]=2.0;
    C[(0*nodes)+2]=3;
    C[(0*nodes)+3]=3;
    C[(0*nodes)+4]=4;

    C[(1*nodes)+0]=2;
    C[(1*nodes)+2]=2;
    C[(1*nodes)+3]=9;
    C[(1*nodes)+4]=8;

    C[(2*nodes)+0]=3;
    C[(2*nodes)+1]=2;
    C[(2*nodes)+3]=10;
    C[(2*nodes)+4]=9;

    C[(3*nodes)+0]=3;
    C[(3*nodes)+1]=9;
    C[(3*nodes)+2]=10;
    C[(3*nodes)+4]=6;

    C[(4*nodes)+0]=4;
    C[(4*nodes)+1]=8;
    C[(4*nodes)+2]=9;
    C[(4*nodes)+3]=6;

    FILE *fp;
    size_t source_size;
    char *programSource;

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

    // Use this to check the output of each API call
    cl_int status;  
     
    //-----------------------------------------------------
    // STEP 1: Discover and initialize the platforms
    //-----------------------------------------------------
    
    cl_uint numPlatforms = 0;
    cl_platform_id *platforms = NULL;
    
    // Use clGetPlatformIDs() to retrieve the number of 
    // platforms
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
 
    // Allocate enough space for each platform
    platforms =   
        (cl_platform_id*)malloc(
            numPlatforms*sizeof(cl_platform_id));
 
    // Fill in platforms with clGetPlatformIDs()
    status = clGetPlatformIDs(numPlatforms, platforms, 
                NULL);

    //-----------------------------------------------------
    // STEP 2: Discover and initialize the devices
    //----------------------------------------------------- 
    
    cl_uint numDevices = 0;
    cl_device_id *devices = NULL;
    int platformToUse = 0;
    // int platformToUse = 1;
    cl_device_type dType = CL_DEVICE_TYPE_ALL;
    // cl_device_type dType = CL_DEVICE_TYPE_GPU;

    // Use clGetDeviceIDs() to retrieve the number of 
    // devices present
    status = clGetDeviceIDs(
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
    status = clGetDeviceIDs(
        platforms[platformToUse], 
        dType,        
        numDevices, 
        devices, 
        NULL);
    //-----------------------------------------------------
    // STEP 3: Create a context
    //----------------------------------------------------- 
    
    cl_context context = NULL;

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
    
    cl_command_queue cmdQueue;

    // Create a command queue using clCreateCommandQueue(),
    // and associate it with the device you want to execute 
    // on
    cmdQueue = clCreateCommandQueue(
        context, 
        devices[0], 
        0, 
        &status);

    //-----------------------------------------------------
    // STEP 5: Create device buffers
    //----------------------------------------------------- 
    
    cl_mem bufferParam;  // cost array on the device
    cl_mem bufferC;  // cost array on the device
    cl_mem bufferP;  // Pheromone array on the device
    cl_mem bufferR;  // Input array on the device
    cl_mem bufferS;  // Output path from the device
    cl_mem bufferSC;  // Output cost from the device

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

    status = clEnqueueWriteBuffer(
        cmdQueue, 
        bufferC, 
        CL_FALSE, 
        0, 
        datasizeC,                         
        C, 
        0, 
        NULL, 
        NULL);


    status = clEnqueueWriteBuffer(
        cmdQueue, 
        bufferR, 
        CL_FALSE, 
        0, 
        datasizeR,                                  
        R, 
        0, 
        NULL, 
        NULL);

    status = clEnqueueWriteBuffer(
        cmdQueue, 
        bufferParam, 
        CL_TRUE, 
        0, 
        datasizeParams,                                  
        &params, 
        0, 
        NULL, 
        NULL);

    //-----------------------------------------------------
    // STEP 7: Create and compile the program
    //----------------------------------------------------- 
    cl_program program = clCreateProgramWithSource(
        context, 
        1, 
        (const char**)&programSource,                                 
        NULL, 
        &status);

    status = clBuildProgram(
        program, 
        numDevices, 
        devices, 
        NULL, 
        NULL, 
        NULL);

    cl_kernel kernel = NULL;
    
    for (int i = 0; i < 4; ++i)
    {

        status = clEnqueueWriteBuffer(
            cmdQueue, 
            bufferP, 
            CL_FALSE, 
            0, 
            datasizeP,                                  
            P, 
            0, 
            NULL, 
            NULL);

       
        //-----------------------------------------------------
        // STEP 8: Create the kernel
        //----------------------------------------------------- 


        // Use clCreateKernel() to create a kernel from the 
        // and walking function (named "stroll")
        kernel = clCreateKernel(program, "stroll", &status);

        //-----------------------------------------------------
        // STEP 9: Set the kernel arguments
        //----------------------------------------------------- 
        
        // Associate the input and output buffers with the 
        // kernel 
        // using clSetKernelArg()
        status  = clSetKernelArg(
            kernel, 
            0, 
            sizeof(cl_mem), 
            &bufferParam);
        status  = clSetKernelArg(
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
        status = clEnqueueNDRangeKernel(
            cmdQueue, 
            kernel, 
            1, 
            NULL, 
            globalWorkSize, 
            NULL, 
            0, 
            NULL, 
            NULL);

        clFinish(cmdQueue);
        //-----------------------------------------------------
        // STEP 12: Read the output buffer back to the host
        //----------------------------------------------------- 
        
        // Use clEnqueueReadBuffer() to read the OpenCL output  
        // buffer (bufferC) 
        // to the host output array (C)
        clEnqueueReadBuffer(
            cmdQueue, 
            bufferS, 
            CL_TRUE, 
            0, 
            datasizeS, 
            S, 
            0, 
            NULL, 
            NULL);
        clEnqueueReadBuffer(
            cmdQueue, 
            bufferSC, 
            CL_TRUE, 
            0, 
            datasizeSC, 
            SC, 
            0, 
            NULL, 
            NULL);


        for (int j = 0; j < k; ++j)
        {
            printf("Output %d is dun %d, score: %f\n",i, j, SC[j]);
            for (int p = 0; p < nodes+1; ++p)
            {
                printf("%d\n", S[(j*(nodes+1))+p]);
            }
            printf("\n");
        }

        printf("%s\n", "PHEREBEFORE----------------------");
        for (int j = 0; j < nodes; ++j)
        {
            for (int p = 0; p < nodes; ++p)
            {
                printf("%f ", P[(j*(nodes))+p]);
            }
            printf("\n");
        }

        updatePheremones(S, P, evap, SC, k, nodes);


        printf("%s\n", "PHEREAFTER-----------------------");
        for (int j = 0; j < nodes; ++j)
        {
            for (int p = 0; p < nodes; ++p)
            {
                printf("%f ", P[(j*(nodes))+p]);
            }
            printf("\n");
        }
        printf("\n");
        printf("\n");
    }
    

    //-----------------------------------------------------
    // STEP 13: Release OpenCL resources
    //----------------------------------------------------- 
    
    // Free OpenCL resources
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(cmdQueue);
    clReleaseMemObject(bufferParam);
    clReleaseMemObject(bufferC);
    clReleaseMemObject(bufferP);
    clReleaseMemObject(bufferR);
    clReleaseMemObject(bufferS);
    clReleaseMemObject(bufferSC);
    clReleaseContext(context);

    // Free host resources
    free(C);
    free(P);
    free(S);
    free(SC);
    free(R);
    free(platforms);
    free(devices);
}
