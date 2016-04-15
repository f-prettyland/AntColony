__kernel void stroll(__global double *C,
            __global double *P,
            __global double *R,
            __global int *S,
            __global double *SC)
{
	int idx = getGlobal();
	
	bool visited[10];

	//length of random is |C[0]| (nodes)


	int bound = 1000;
	ulong seed = idx+222;
	seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
	uint result = (seed >> 16)%bound;

	SC[idx] = len; 
}

int getGlobal(){
	return get_global_id(0);
}