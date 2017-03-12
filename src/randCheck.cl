typedef struct Params
{
    int Nodes;
    int StartNode;
    double Alpha;
    double Beta;
    double Evap;
    int K;
} Params;

double getRandom(double seedSeed, int idx){
    double bound = 1000;
    long seed = idx+seedSeed;
    seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
    int result = (seed >> 16)%((int)bound);
    return (result/bound);
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
   int nodes = param->Nodes;

    //start at first node
    int solnLength = 0;

    double freeWill = getRandom(R[solnLength], idx);
    SC[idx] = freeWill;


    // SC[idx]                             = 10.0;
    // // solnCost += 5;

}

