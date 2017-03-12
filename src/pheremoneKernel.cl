typedef struct Params
{
    int Nodes;
    int StartNode;
    double Alpha;
    double Beta;
    double Evap;
    int K;
} Params;

__kernel void update(
            __constant struct Params *param,
            __global double *P,
            __global int *S,
            __global double *SC
            )
{
   int idx = get_global_id(0);
   int nodes = param->Nodes;
   int k = param->K;
   int evap = param->Evap;
   
   for (int j = 0; j < nodes; ++j)
   {
        P[(idx*nodes)+j] = ((1-evap)*P[(idx*nodes)+j]);
   }

   for (int i = 0; i < k; ++i)
   {
       for (int j = 0; j < nodes; ++j)
       {
            if(idx == S[(i*(nodes+1))+j]){
                int cityDst = S[(i*(nodes+1))+j+1];
                P[(idx*nodes)+cityDst] += (1.f/SC[i]);
            }
       }
   }
}
