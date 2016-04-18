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
{
   int idx = get_global_id(0);

    __private int solution[NODESIZEPLUS1];
    __private bool visited[NODESIZE];
    __private double edgeAttraction[NODESIZE];

    int nodes = param->Nodes;
    int startNode = param->StartNode;
    double alpha = param->Alpha;
    double beta = param->Beta;
    if(startNode == -1){
        startNode = get_global_id(0) % nodes;
    }

    int solnLength = 0;
    double solnCost = 0;

    for (int i = 0; i < NODESIZE; ++i)
    {
        visited[i]=false;
    }

    //start at first node
    solution[solnLength] = startNode;
    visited[startNode] = true;

    bool dan = false;
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
            int possiblePaths =0;
    		double sumPossEdgeAttract = 0;

    		for (int i = 0; i < nodes; ++i)
    		{
    			double possibleEdgeCost = C[((solution[solnLength])*nodes)+i];
    			//If there is an edge there and haven't visited before
    			if(possibleEdgeCost != 0 && !visited[i]){
    				//Non-normalised attractiveness of edge
                    edgeAttraction[i] = pow(attractFromCost(possibleEdgeCost),alpha)*pow((P[((solution[solnLength])*nodes)+i]),beta);
    				//add to sum for nomarlisation later
    				sumPossEdgeAttract += edgeAttraction[i];
                    possiblePaths++;
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

