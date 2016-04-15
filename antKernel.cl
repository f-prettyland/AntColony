typedef struct Params
{
    int Nodes;
    int StartNode;
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
	
	//Plus one as should end where began
	int[nodes+1] solution;
	bool[nodes] visited;
	int solnLength = 0;
	double solnCost = 0;

	//start at first node
	solution[solnLength] = StartNode;
	visited[StartNode] = true;


	// //length of random is |C[0]| (nodes)
	do{
		//if it has gone to all the nodes, but is not at the beginning
		if(solnLength==(nodes-1)){
			//ASSUMPTION THAT GRAPH IS CONNECTED TAKES PLACE HERE CAN CHECK FOR 0 THEN NEVER TAKE THAT PATH AGAIN
			//must go back to beginning no need to check others
			solnCost += C[((solution[solnLength])*nodes)+0];
			solnLength++;
			solution[solnLength] = StartNode;
		}else{
			//can be -1 as path to self should be 0 (not allowed)
			double[nodes-1] edgeAttraction;
			//as not all nodes are in edge attraction need to map from place to place
			int[nodes-1] edgeAttrMap;
			int numPossPaths = 0;

			double sumPossEdgeAttract = 0; 

			for (int i = 0; i < nodes; ++i)
			{
				double possibleEdgeCost = C[((solution[solnLength])*nodes)+i];
				//If there is an edge there and haven't visited before
				if(possibleEdgeCost != 0 && !visited[i]){
					//Non-normalised attractiveness of edge
					edgeAttraction[numPossPaths] = (alpha*possibleEdgeCost)+(beta*(P[((solution[solnLength])*nodes)+i]))
					//add to sum for nomarlisation later
					sumPossEdgeAttract += possibleEdgeCost;
					//mark which node this value is for
					edgeAttrMap[numPossPaths] = i;
					//increase number of found paths
					numPossPaths++;
				}
			}
			double freeWill = getRandom(R[solnLength], idx);
			int chosenPath = getProbableEdgeRandomly(freeWill, edgeAttraction, numPossPaths, sumPossEdgeAttract);
			solnCost +=  C[((solution[solnLength])*nodes)+edgeAttrMap[chosenPath]];

			visited[edgeAttrMap[chosenPath]] = true;
			
			solnLength++;
			solution[solnLength] = edgeAttrMap[chosenPath];
		}
	//whilst not at the beginning
	}while((solution[solnLength]) != StartNode)


	SC[idx]								= solnCost;
	S[idx*sizeof(int)*(nodes+1)]		= solution;

	SC[idx] = 66.0; 
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