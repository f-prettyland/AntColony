typedef struct Params
{
    int Nodes;
    int StartNode;
    double Alpha;
    double Beta;
} Params;

uint getRandom(double seedSeed, int idx, int bound){
	ulong seed = idx+seedSeed;
	seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
	uint result = (seed >> 16)%bound;
	return result;
}


__kernel void stroll(
			__constant struct Params *param,
			__global double *C,
            __global double *P,
            __global double *R,
            __global int *S,
            __global double *SC)
{
	int idx = get_global_id(0);
	const int bound = 1000;



	uint result = getRandom(222, idx, bound);

	SC[idx] = result;
}

