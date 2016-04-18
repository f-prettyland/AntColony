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
int maxCost;
int minCost;
double pherStart;
int randBound = 10;
int verbosity = 1;

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
        printf("-rB random bound limit [if left blank will take harcoded limit: %d]\n", randBound);
        printf("-vB verbosity, 0 is only final output, 1 is intermediate solutions, 2 is intermediate all values [if left blank will take harcoded limit: %d]\n", verbosity);
        exit(0);
    }

    k = atoi(argv[1]);
    evap = atof(argv[2]);
    params.Alpha = atof(argv[3]);
    params.Beta = atof(argv[4]);
    maxIter = atoi(argv[5]);
    nodes = atoi(argv[6]);
    minCost = atoi(argv[7]);
    maxCost = atoi(argv[8]);
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
            if((p = strstr(argv[i], "-rB"))){
                randBound = atoi(argv[i+1]);
            }

            if((p = strstr(argv[i], "-vB"))){
                verbosity = atoi(argv[i+1]);
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

void createGraph(){
    //make random
    getLmtRands2D(C, nodes, maxCost, minCost);

    //make sure cannot go to self
    for(int j = 0; j < nodes; j++) {
            C[(j*nodes)+j] = 0;
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

    getRandsDoub(R, nodes, randBound);
    createGraph();
    size_t source_size =readInKernel(nodes);
}

void freeMemory(){
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


int bestSoln(){
    int best =0;
    for (int i = 0; i < k; ++i)
    {
        if(SC[best]>SC[i]){
            best =i;
        }
    }
    return best;
}

int main(int argc, char *argv[]) {
    handleArguments(argc, argv);
    initialiseDatastructures();
    
    // Use this to check the output of each API call
    cl_int status;  
    status |= platformsAndDevices(status);  
    if(status < 0){
        fprintf(stderr, "%s %d\n", "Error in platform identification error code: ", status);
        exit(0);
    }
    status |=  createAntBuffers(status);
    if(status < 0){
        fprintf(stderr, "%s %d\n", "Error in buffer generation with error code: ", status);
        exit(0);
    }
    status |=  createProgram(status);
    if(status < 0){
        fprintf(stderr, "%s %d\n", "Error in compilation with error code: ", status);
        exit(0);
    }
    
    if(verbosity>=2){
        printf("%s\n", "INIT PHEREMONE");
        printf("%s\n", "---------");
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

    for (int i = 0; i < maxIter; ++i)
    {

        status |= queueAntStroll(status, k);

        clFinish(cmdQueue);
        
        status |= readOutput(status);

        if(verbosity>=1){
            printf("\n");
            printf("\n");
            printf("------------------\n");
            printf("Iteration %d\n", i);
            printf("------------------\n");
            for (int j = 0; j < k; ++j)
            {
                printf("Output %d is dun %d, score: %f\n",i, j, SC[j]);
                for (int p = 0; p < nodes+1; ++p)
                {
                    printf("%d ", S[(j*(nodes+1))+p]);
                }
                printf("\n");
            }
        }

        updatePheremones(S, P, evap, SC, k, nodes);

        if(verbosity>=2){
            printf("%s\n", "PHEREMONE");
            printf("%s\n", "---------");
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
    }
    
    printf("\n");
    printf("\n");
    printf("------------------------------------\n");
    printf("After %d iterations best solution is: \n", maxIter);
    int bs = bestSoln();
    for (int p = 0; p < nodes+1; ++p)
    {
        printf("%d ", S[(bs*(nodes+1))+p]);
    }
    printf("\nWith a total travel cost of %f\n", SC[bs]);

    freeMemory();
}
