#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "BorrowedFunc.h"
#include "Structure.h"



int k;
double evap;
int maxIter;
int nodes;
double maxCost;
double minCost;
double pherStart;

void handleArguments(int argc, char *argv[]){
    if(argc<8){
        puts("---This program takes 7 arguments and 2 optional arguments:");
        puts("number of ants per iteration");
        puts("evaporation constant");
        puts("alpha");
        puts("beta");
        puts("max number of iterations");
        puts("number of nodes");
        puts("maximum cost of an edge");
        puts("minimum cost of an edge");
        puts("\n---Optional");
        puts("-iP initial pheremone [if left blank will perform random walk to guess value]");
        puts("-sN start node [if left blank will assign an ant starting location of (id mod number of nodes)]");
        exit(0);
    }

    k = atoi(argv[1]);
    evap = atof(argv[2]);
    params.Alpha = atof(argv[3]);
    params.Beta = atof(argv[4]);
    maxIter = atoi(argv[5]);
    nodes = atoi(argv[6]);
    maxCost = atof(argv[7]);
    minCost = atof(argv[8]);
    if(minCost<=0){
        puts("Minimum cost must be more than 0, assuming to be 1 instead");
        minCost=1.0;
    }
    params.Nodes = nodes;

    // never negative as otherwise would break alpha and beta
    pherStart = -1;
    params.StartNode = -1;

    if(argc >=9){
        char *p;
        for (int i = 9; i < (argc-1); ++i)
        {
            if((p = strstr(argv[i], "-iP"))){
                pherStart = atof(argv[i+1]);
            }
            if((p = strstr(argv[i], "sN"))){
                params.StartNode = atoi(argv[i+1]);
            }
        }
    }
}



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

void initialiseDatastructures(){
    if(pherStart==-1){
        //perform random walk
        pherStart=0.5;
    }

    // Compute the size of the data 
    datasizeParams   = sizeof(Params);
    datasizeC    = sizeof(double)*nodes*nodes;
    datasizeP    = sizeof(double)*nodes*nodes;
    //Need to make a decision at each node so need a random seed at each one of the
    datasizeR    = sizeof(double)*nodes;
    datasizeS    = sizeof(int)*(nodes+1)*k;
    datasizeSC   = sizeof(double)*k;

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

    
    size_t source_size =readInKernel(nodes);
}

int main(int argc, char *argv[]) {

    handleArguments(argc, argv);
    initialiseDatastructures();
    
    // Use this to check the output of each API call
    cl_int status;  
    status |= platformsAndDevices(status);  
    
    status |=  createAntBuffers(status);
    



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

    for (int i = 0; i < maxIter; ++i)
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
    free(devices);

}
