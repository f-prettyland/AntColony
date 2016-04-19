#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include "BorrowedFunc.h"
#include "Structure.h"

int k;
int maxIter;
int nodes;
int maxCost;
int minCost;
double pherStart;
int randBound = 10;
int verbosity = 1;
bool parallelP = true;

void handleArguments(int argc, char *argv[]){
    if(argc<8){
        puts("---This program takes 7 arguments and 2 optional arguments:");
        puts("1 number of ants per iteration");
        puts("2 evaporation constant");
        puts("3 alpha");
        puts("4 beta");
        puts("5 max number of iterations");
        puts("6 number of nodes");
        puts("7 maximum cost of an edge");
        puts("8 minimum cost of an edge");
        puts("\nEg: ./ants 5 0.1 1 2 200 20 10 100");
        puts("\n\n---Optional");
        puts("-iP initial pheremone [if left blank will perform random walk to guess value]");
        puts("-sN start node [if left blank will assign an ant starting location of (id mod number of nodes)]");
        printf("-rB random bound limit [if left blank will take harcoded limit: %d]\n", randBound);
        printf("-vB verbosity, 0 is only final output, 1 is intermediate solutions, 2 is intermediate all values, -1 is for csv output [if left blank will take harcoded limit: %d]\n", verbosity);
        printf("-sPh sets to do serial pheremonal updates\n");
        exit(0);
    }

    k = atoi(argv[1]);
    params.K = k;
    params.Evap = atof(argv[2]);
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
        //flags
        for (int i = 9; i < (argc); ++i)
        {
            if((p = strstr(argv[i], "-sPh"))){
                parallelP =false;
            }
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

int randomWalkLength(){
    bool visited[nodes];
    for (int i = 0; i < nodes; ++i)
    {
        visited[i]=false;
    }
    int startNode = 0;
    int solnLength = 0;
    double solnCost = 0;
    int currNode = startNode ;
    visited[startNode] = true;
    pcg32_random_t rng;
    pcg32_srandom_r(&rng, time(NULL) ^ (intptr_t)&printf,(intptr_t)&nodes);
    do{
        if(solnLength==(nodes-1)){
            solnCost += C[((currNode)*nodes)+0];
            solnLength++;
            currNode = startNode;
        }else{
            int possiblePlaces[nodes-1];
            int numberOfPlaces=0;

            for (int i = 0; i < nodes; ++i)
            {
                double possibleEdgeCost = C[((currNode)*nodes)+i];
                if(possibleEdgeCost != 0 && !visited[i]){
                    possiblePlaces[numberOfPlaces] =i;
                    numberOfPlaces++;
                }
            }
            int randomEdge = ((pcg32_boundedrand_r(&rng,numberOfPlaces)));
            int chosenPath = possiblePlaces[randomEdge];

            solnCost +=  C[((currNode)*nodes)+chosenPath];
            visited[chosenPath] = true;
            solnLength++;
            currNode = chosenPath;
        }
    //whilst not at the beginning
    }while((currNode) != startNode);
    return solnCost;
}


void initialiseDatastructures(){
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

    getRandsDoub(R, nodes, randBound);
    createGraph();
    size_t source_size =readInKernel(nodes);

    if(pherStart==-1){
        //perform random walk
        pherStart=(1.f/randomWalkLength());
    }

    // Initialize the input pheromones
    for(int j = 0; j < nodes; j++) {
        for(int i = 0; i < nodes; i++) {
            P[(j*nodes)+i] = pherStart;
        }
    }
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

void updatePheremonesSeq(int *S, double *P, double *SC, int k, int nodes) {
    //evaporate
    for (int i = 0; i < nodes; ++i)
    {
        for (int j = 0; j < nodes; ++j)
        {
            P[(i*nodes)+j] = ((1-params.Evap)*P[(i*nodes)+j]);
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

void outputSolutionArray(int i){
    printf("\n");
    printf("\n");
    printf("------------------\n");
    printf("Iteration %d\n", i);
    printf("------------------\n");
    for (int j = 0; j < k; ++j)
    {
        printf("Output %d is dun %d, score: %.2f\n",i, j, SC[j]);
        for (int p = 0; p < nodes+1; ++p)
        {
            printf("%d ", S[(j*(nodes+1))+p]);
        }
        printf("\n");
    }
}

void outputPheremoneArray(){
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
    status |=  createAntProgram(status);
    if(status < 0){
        fprintf(stderr, "%s %d\n", "Error in compilation with error code: ", status);
        exit(0);
    }


    if(verbosity>=2){
        outputPheremoneArray();
    }
    if(parallelP){
        readInPherKernel();

        status |=  createPheremoneProgram(status);
        if(status < 0){
            fprintf(stderr, "%s %d\n", "Error in pheremone program compilation with error code: ", status);
            exit(0);
        }
    }
    double bestSolnThroughout = (INT_MAX-1);

    clock_t start = clock(), diff;

    for (int i = 0; i < maxIter; ++i)
    {

        status |= queueAntStroll(status, k);

        clFinish(cmdQueue);
        
        status |= readOutput(status);

        if(verbosity>=1){
            outputSolutionArray(i);
        }

        int bs = bestSoln();
        if(SC[bs]<bestSolnThroughout){
            bestSolnThroughout = SC[bs];
        }
        if(!parallelP){
            updatePheremonesSeq(S, P, SC, k, nodes);
        }else{
            status |=  createPherBuffers(status);
            if(status < 0){
                fprintf(stderr, "%s %d\n", "Error in pheremone buffer generation with error code: ", status);
                exit(0);
            }
            status |= queuePherUpdate(status, nodes);
            if(status < 0){
                fprintf(stderr, "%s %d\n", "Error in pheremone execution with error code: ", status);
                exit(0);
            }
            clFinish(cmdQueue);
            
            status |= readPherOutput(status);
            if(status < 0){
                fprintf(stderr, "%s %d\n", "Error in pheremone buffer output with error code: ", status);
                exit(0);
            }
        }

        if(verbosity>=2){
            outputPheremoneArray();
        }
    }
    diff = clock() - start;

    int msec = diff * 1000 / CLOCKS_PER_SEC;

    int bs = bestSoln();
    if(verbosity>=0){
        printf("\n");
        printf("\n");
        printf("------------------------------------\n");
        printf("After %d iterations the lowest final value is: \n", maxIter);
        for (int p = 0; p < nodes+1; ++p)
        {
            printf("%d ", S[(bs*(nodes+1))+p]);
        }
        printf("\nWith a total travel cost of %.2f though the lowest cost travel has been %.2f\n", SC[bs], bestSolnThroughout);
        printf("It took %f millisecs\n", msec);
    }else{
        printf("%d, %f, %f, %f, %d, %d, %d, %d, %f, %f, %d, %d\n", k, params.Evap, params.Alpha, params.Beta, maxIter, nodes, minCost, maxCost, SC[bs], bestSolnThroughout, parallelP, msec);
    }
    freeMemory();
}
