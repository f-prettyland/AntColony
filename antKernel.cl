typedef struct Params
{
    int Nodes;
    int StartNode;
    double Alpha;
    double Beta;
} Params;

int getProbableEdgeRandomly(int probab, __local double * edgeAttraction, int edgeProbSize, double sumEdgeProbs, int bound){
    double sum = 0;
    for (int i = 0; i < edgeProbSize; i++){
        sum+=(edgeAttraction[i]/sumEdgeProbs)*bound;
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

__kernel void stroll(
            __constant struct Params *param,
            __global double *C,
            __global double *P,
            __global double *R,
            __global int *S,
            __global double *SC,
            __local int * solution,
            __local bool * visited,
            __local double * edgeAttraction
            )
// void stroll(
//             struct Params *param,
//             double *C,
//             double *P,
//             double *R,
//             int *S,
//             double *SC)
{

    int idx = get_global_id(0);
    // int idx = 0;
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


    SC[idx]								= solnCost;
    //needs fix
    S[idx*sizeof(int)*(nodes+1)]		= solution;

}

