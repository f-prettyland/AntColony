typedef struct Params
{
    int Nodes;
    double Alpha;
    double Beta;
} Params;

const int bound = 1000;
__kernel void stroll(
			__constant struct Params *param,
			__global double *C,
            __global double *P,
            __global double *R,
            __global int *S,
            __global double *SC)
{
	int idx = get_global_id(0);

	int nodes = param->Nodes;
	double alpha = param->Alpha;
	double beta = param->Beta;
	
	int currNode = 0;
	//Plus one as should end where began
	int[nodes+1] solution;
	bool[nodes] visited;
	int solnLength = 0;

	// //length of random is |C[0]| (nodes)
	do{
		double[nodes-1] edgeAttraction;
		int numPossPaths = 0;

		double sumPossEdgeAttract = 0; 

		for (int i = 0; i < nodes; ++i)
		{
			double possibleEdgeCost = C[(currNode*nodes)+i];
			//If there is an edge there and haven't visited before
			if(possibleEdgeCost != 0 && !visited[i]){
				//Non-normalised attractiveness of edge
				edgeAttraction[numPossPaths] = (alpha*possibleEdgeCost)+(beta*(P[(currNode*nodes)+i]))
				//add to sum for nomarlisation later
				sumPossEdgeAttract += possibleEdgeCost;
				//increase number of found paths
				numPossPaths++;
			}
		}
		double freeWill = getRandom(R[solnLength], idx);
		int chosenPath = getProbableEdgeRandomly(freeWill, edgeAttraction, numPossPaths, sumPossEdgeAttract);

	}while(currNode !=0)



	// SC[idx] = param->Nodes; 
	SC[idx] = 50.0; 
}

int getProbableEdgeRandomly(uint probab, double* edgeProb, int edgeProbSize, double sumEdgeProbs){
	double sum = 0;
	for (int i = 0; i < edgeProbSize; ++i)
	{
		sum+=edgeProb[i];
		if(probab<((edgeProb[i]/sumEdgeProbs)*bound))
			return i
	}
	return (edgeProbSize-1);
}

uint getRandom(double seedSeed, int idx){

	ulong seed = idx+seedSeed;
	seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
	uint result = (seed >> 16)%bound;
	return result;
}