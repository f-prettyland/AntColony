#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Params
{
    int Nodes;
    int StartNode;
    double Alpha;
    double Beta;
} Params;

int getProbableEdgeRandomly(int probab, double* edgeProb, int edgeProbSize, double sumEdgeProbs, int bound){
    double sum = 0;
    for (int i = 0; i < edgeProbSize; ++i){
        sum+=(edgeProb[i]/sumEdgeProbs)*bound;
        if(probab<(sum)){
            return i;
        }
    }
    return (edgeProbSize-1);
    // return 0;
}

int getRandom(double seedSeed, int idx, int bound){
    long seed = idx+seedSeed;
    seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
    int result = (seed >> 16)%bound;
    return result;
}

double attractFromCost(double cost){
    return (1.f/cost);
}

// __kernel void stroll(
//             __constant struct Params *param,
//             __global double *C,
//             __global double *P,
//             __global double *R,
//             __global int *S,
//             __global double *SC,
//             __local int * solution,
//             __local bool * visited,
//             __local double * edgeAttraction
//             )
void stroll(
            struct Params *param,
            double *C,
            double *P,
            double *R,
            int *S,
            double *SC,
            int * solution,
            bool * visited,
            double * edgeAttraction
            )
{

    // int idx = get_global_id(0);
    int idx = 0;
    const int bound = 1000;

        int nodes = param->Nodes;
        double alpha = param->Alpha;
        double beta = param->Beta;
        int startNode = param->StartNode;
        
        int solnLength = 0;
        double solnCost = 0;

        //start at first node
        solution[solnLength] = startNode;
        visited[startNode] = true;

        do{
            //if it has gone to all the nodes, but is not at the beginning
            if(solnLength==(nodes-1)){
                //ASSUMPTION THAT GRAPH IS CONNECTED TAKES PLACE HERE CAN CHECK FOR 0 THEN NEVER TAKE THAT PATH AGAIN
                //must go back to beginning no need to check others
                solnCost += C[((solution[solnLength])*nodes)+0];
                solnLength++;
                solution[solnLength] = startNode;
            }else{
                //can be -1 as path to self should be 0 (not allowed)

                //as not all nodes are in edge attraction need to map from place to place
                // int edgeAttrMap[nodes-1];
                // int numPossPaths = 0;

                double sumPossEdgeAttract = 0; 


                for (int i = 0; i < nodes; ++i)
                {
                    double possibleEdgeCost = C[((solution[solnLength])*nodes)+i];
                    //If there is an edge there and haven't visited before
                    if(possibleEdgeCost != 0 && !visited[i]){
                        //Non-normalised attractiveness of edge
                        edgeAttraction[i] = (alpha*possibleEdgeCost)+(beta*attractFromCost(P[((solution[solnLength])*nodes)+i]));
                        //add to sum for nomarlisation later
                        sumPossEdgeAttract += possibleEdgeCost;
                    }else{
                        // already seen or zero cost
                        edgeAttraction[i] = 0;
                    }
                }
                
                int freeWill = getRandom(R[solnLength], idx, bound);
                int chosenPath = getProbableEdgeRandomly(freeWill, edgeAttraction, nodes, sumPossEdgeAttract, bound);
                // int chosenPath = 0;
                solnCost +=  C[((solution[solnLength])*nodes)+chosenPath];

                visited[chosenPath] = true;
                
                solnLength++;
                solution[solnLength] = chosenPath;
            }
        //whilst not at the beginning
        }while((solution[solnLength]) != startNode);


        SC[idx]                             = solnCost;
        //needs fix
        S[idx*sizeof(int)*(nodes+1)]        = solution;

        SC[idx] = 62.0; 
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
    const double pherStart = 1;

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
    S   = (int*)malloc(datasizeS);
    SC  = (double*)malloc(datasizeSC);

    // Initialize the input pheromones
    for(int j = 0; j < nodes; j++) {
        for(int i = 0; i < nodes; i++) {
            P[(j*nodes)+i] = pherStart;
        }
    }

    // Initialize the random
    for(int j = 0; j < nodes; j++) {
        R[j] = j+345;
        // R[j] = 0;
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


    int solution[nodes+1];
    bool visited[nodes];

    double edgeAttraction[nodes];

    stroll(&params,C,P,R,S,SC,solution,visited,edgeAttraction);

    for (int i = 0; i < nodes+1; ++i)
    {
        printf("%d\n", solution[i]);
    }
}