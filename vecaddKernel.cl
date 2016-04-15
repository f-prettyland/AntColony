__kernel void vecadd(__global int *A,
            __global int *B,
            __global int *C)
{

   // Get the work-item’s unique ID
   int idx = get_global_id(0);
   int bound = 1000;
   ulong seed = idx+222;                    
   seed = (seed * 0x5DEECE66DL + 0xBL) & ((1L << 48) - 1);
   uint result = (seed >> 16)%bound;                             
   C[idx] = result;    

   // Add the corresponding locations of
   // 'A' and 'B', and store the result in 'C'.
}
