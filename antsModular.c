#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <CL/cl.h>

#define MAX_SOURCE_SIZE (0x100000)

const char fileName[] = "./antKernel.cl";

typedef struct Params
{
    int Nodes;
    int StartNode;
    double Alpha;
    double Beta;
} Params;

typedef struct ColonyAndContext
{
	Params params;
	int k;
	int nodes;
	double *C;
	double *P;
	double *R;
	int *S;
	double *SC;
	size_t datasizeParams;
	size_t datasizeC;
	size_t datasizeP;
	size_t datasizeR;
	size_t datasizeS;
	size_t datasizeSC;
	size_t source_size;
	char *programSource;
} ColonyAndContext;

void initialize(ColonyAndContext cc, double pherStart) {
	// This code executes on the OpenCL host
	cc.k = 5;

	// Host data
	cc.C   = NULL;  // Cost array
	cc.P   = NULL;  // Pheromone array
	cc.R   = NULL;     // Ants randomness
	cc.S   = NULL;     // Path taken
	cc.SC  = NULL;// cost of solution

	// Nodes to represent in each array
	cc.nodes = 5;

	cc.params.Nodes = (cc.nodes);

	cc.params.StartNode = 0;
	//cost weighting
	cc.params.Alpha = 1;
	//pheromone weighting
	cc.params.Beta = 1;
	
	// Compute the size of the data 
	cc.datasizeParams   = sizeof(Params);

	cc.datasizeC    = sizeof(double)*(cc.nodes)*(cc.nodes);
	cc.datasizeP    = sizeof(double)*(cc.nodes)*(cc.nodes);
	//Need to make a decision at each node so need a random seed at each one of the
	cc.datasizeR    = sizeof(double)*(cc.nodes);
	cc.datasizeS    = sizeof(int)*((cc.nodes)+1)*(cc.k);
	cc.datasizeSC   = sizeof(double)*(cc.k);

	// Allocate space for input/output data
	cc.C   = (double*)malloc(cc.datasizeC);
	cc.P   = (double*)malloc(cc.datasizeP);
	cc.R   = (double*)malloc(cc.datasizeR);
	cc.S   = (int*)malloc(cc.datasizeS);
	cc.SC  = (double*)malloc(cc.datasizeSC);

	// Initialize the input pheromones
	for(int j = 0; j < (cc.nodes); j++) {
	    for(int i = 0; i < (cc.nodes); i++) {
	        cc.P[(j*(cc.nodes))+i] = pherStart;
	    }
	}

	// Initialize the random
	for(int j = 0; j < (cc.nodes); j++) {
	    cc.R[j] = j+1;
	}

	// todo, calloc this?
	for(int j = 0; j < (cc.nodes); j++) {
	    for(int i = 0; i < (cc.nodes); i++) {
	        cc.C[(j*(cc.nodes))+i] = 0;
	    }
	}

	cc.C[(0*(cc.nodes))+1]=2.0;
	cc.C[(0*(cc.nodes))+2]=3;
	cc.C[(0*(cc.nodes))+3]=3;
	cc.C[(0*(cc.nodes))+4]=4;

	cc.C[(1*(cc.nodes))+0]=2;
	cc.C[(1*(cc.nodes))+2]=2;
	cc.C[(1*(cc.nodes))+3]=9;
	cc.C[(1*(cc.nodes))+4]=8;

	cc.C[(2*(cc.nodes))+0]=3;
	cc.C[(2*(cc.nodes))+1]=2;
	cc.C[(2*(cc.nodes))+3]=10;
	cc.C[(2*(cc.nodes))+4]=9;

	cc.C[(3*(cc.nodes))+0]=3;
	cc.C[(3*(cc.nodes))+1]=9;
	cc.C[(3*(cc.nodes))+2]=10;
	cc.C[(3*(cc.nodes))+4]=6;

	cc.C[(4*(cc.nodes))+0]=4;
	cc.C[(4*(cc.nodes))+1]=8;
	cc.C[(4*(cc.nodes))+2]=9;
	cc.C[(4*(cc.nodes))+3]=6;

	FILE *fp;
	/* Load kernel source file */
	fp = fopen(fileName, "r");
	if (!fp) {
	    fprintf(stderr, "Failed to load kernel.\n");    
	    exit(1);
	}   
	cc.programSource = (char *)malloc(MAX_SOURCE_SIZE);
	cc.source_size = fread(cc.programSource, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);
} 