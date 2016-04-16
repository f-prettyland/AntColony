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
        sum+=edgeProb[i];
        if(probab<((edgeProb[i]/sumEdgeProbs)*bound)){
            return i;
        }
    }
    return (edgeProbSize-1);
}

int getRandom(double seedSeed, int idx, int bound){
    long seed = idx+seedSeed;
    seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
    int result = (seed >> 16)%bound;
    return result;
}

__kernel void stroll(
            __constant struct Params *param,
            __global double *C,
            __global double *P,
            __global double *R,
            __global int *S,
            __global double *SC)
// void stroll(
//             struct Params *param,
//             double *C,
//             double *P,
//             double *R,
//             int *S,
//             double *SC)
// {

    int idx = get_global_id(0);
    // int idx = 0;
    const int bound = 1000;

    int nodes = param->Nodes;
    double alpha = param->Alpha;
    double beta = param->Beta;
    int startNode = param->StartNode;
    
    //Plus one as should end where began
    int solution[nodes+1];
    bool visited[nodes];
    int solnLength = 0;
    double solnCost = 0;

    //start at first node
    solution[solnLength] = startNode;
    visited[startNode] = true;


    // //length of random is |C[0]| (nodes)
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
            double edgeAttraction[nodes-1];
            //as not all nodes are in edge attraction need to map from place to place
            int edgeAttrMap[nodes-1];
            int numPossPaths = 0;

            double sumPossEdgeAttract = 0; 

            for (int i = 0; i < nodes; ++i)
            {
                double possibleEdgeCost = C[((solution[solnLength])*nodes)+i];
                //If there is an edge there and haven't visited before
                if(possibleEdgeCost != 0 && !visited[i]){
                    //Non-normalised attractiveness of edge
                    edgeAttraction[numPossPaths] = (alpha*possibleEdgeCost)+(beta*(P[((solution[solnLength])*nodes)+i]));
                    //add to sum for nomarlisation later
                    sumPossEdgeAttract += possibleEdgeCost;
                    //mark which node this value is for
                    edgeAttrMap[numPossPaths] = i;
                    //increase number of found paths
                    numPossPaths++;
                }
            }
            double freeWill = getRandom(R[solnLength], idx, bound);
            int chosenPath = getProbableEdgeRandomly(freeWill, edgeAttraction, numPossPaths, sumPossEdgeAttract, bound);
            solnCost +=  C[((solution[solnLength])*nodes)+edgeAttrMap[chosenPath]];

            visited[edgeAttrMap[chosenPath]] = true;
            
            solnLength++;
            solution[solnLength] = edgeAttrMap[chosenPath];
        }
    //whilst not at the beginning
    }while((solution[solnLength]) != startNode);


    SC[idx]                             = solnCost;
    //needs fix
    S[idx*sizeof(int)*(nodes+1)]        = solution;

    SC[idx] = 66.0; 
}

