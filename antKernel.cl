typedef struct Params
{
    int Nodes;
    int StartNode;
    double Alpha;
    double Beta;
} Params;

int getProbableEdgeRandomly(double probab, __private double * edgeAttraction, int edgeProbSize, double sumEdgeProbs){
    double sum = 0;
    for (int i = 0; i < edgeProbSize; i++){
        sum+=(edgeAttraction[i]/sumEdgeProbs);
	    if(probab<(sum)){
	        return i;
	    }
    }
    return (edgeProbSize-1);
    // return 0;
}

double getRandom(double seedSeed, int idx){
    double bound = 1000;
    long seed = idx+seedSeed;
    seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
    int result = (seed >> 16)%((int)bound);
    return (result/bound);
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
            __global double *SC
            )
// void stroll(
            // int idx,
//             struct Params *param,
//             double *C,
//             double *P,
//             double *R,
//             int *S,
//             double *SC)
{

    int idx = get_global_id(0);
    // int idx = 0;

    __private int solution[6];
    __private bool visited[5];
    __private double edgeAttraction[5];
    solution[4]=23;
    solution[5]=3;

    int nodes = param->Nodes;
    double alpha = param->Alpha;
    double beta = param->Beta;
    int startNode = param->StartNode;
    
    int solnLength = 0;
    double solnCost = 0;

    for (int i = 0; i < 5; ++i)
    {
        visited[i]=false;
    }

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
                    edgeAttraction[i] = (alpha*attractFromCost(possibleEdgeCost))+(beta*(P[((solution[solnLength])*nodes)+i]));
    				//add to sum for nomarlisation later
    				sumPossEdgeAttract += edgeAttraction[i];
    			}else{
    				// already seen or zero cost
    				edgeAttraction[i] = 0;
    			}
    		}
    		
    		double freeWill = getRandom(R[solnLength], idx);
    		int chosenPath = getProbableEdgeRandomly(freeWill, edgeAttraction, nodes, sumPossEdgeAttract);
            solnCost +=  C[((solution[solnLength])*nodes)+chosenPath];

    		visited[chosenPath] = true;
    		
    		solnLength++;
    		solution[solnLength] = chosenPath;
    	}
    //whilst not at the beginning
    }while((solution[solnLength]) != startNode);


    // SC[idx]                             = 10.0;
    // // solnCost += 5;
    SC[idx]                             = solnCost;
    for (int p = 0; p < nodes+1; p++)
    {
        S[(idx*(nodes+1))+p] = solution[p];
    }

}

