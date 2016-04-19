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
   for (int j = 0; j < nodes; ++j)
   {
        P[(idx*nodes)+j] = ((1-params.Evap)*P[(idx*nodes)+j]);
   }

   for (int i = 0; i < k; ++i)
   {
       for (int j = 0; j < nodes; ++j)
       {
            if(idx == S[(i*k)+j]){
                int cityDst = S[(i*k)+j+1];
                P[(citysrc*nodes)+cityDst] += (1.f/SC[i]);
            }
       }
   }
}

