#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

typedef struct { uint64_t state;  uint64_t inc; } pcg32_random_t;

uint32_t pcg32_random_r(pcg32_random_t* rng)
{
    uint64_t oldstate = rng->state;
    // Advance internal state
    rng->state = oldstate * 6364136223846793005ULL + (rng->inc|1);
    // Calculate output function (XSH RR), uses old state for max ILP
    uint32_t xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    uint32_t rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

void pcg32_srandom_r(pcg32_random_t* rng, uint64_t initstate, uint64_t initseq)
{
    rng->state = 0U;
    rng->inc = (initseq << 1u) | 1u;
    pcg32_random_r(rng);
    rng->state += initstate;
    pcg32_random_r(rng);
}

uint32_t pcg32_boundedrand_r(pcg32_random_t* rng, uint32_t bound)
{
    uint32_t threshold = -bound % bound;

    for (;;) {
        uint32_t r = pcg32_random_r(rng);
        if (r >= threshold)
            return r % bound;
    }
}

void getLmtRands2D(double * rands, int size, int max, int min)
{
    pcg32_random_t rng;
    int rounds = size*size;
	pcg32_srandom_r(&rng, time(NULL) ^ (intptr_t)&printf,(intptr_t)&rounds);
    for (int j = 0; j < size; ++j)
    {
    	for (int i = 0; i < size; ++i)
    	{
	        rands[(j*size)+i]=  (double)((pcg32_boundedrand_r(&rng,max))+min);
    	}
    }
}

void getRandsDoub(double* rands, int rounds, int bound)
{
    pcg32_random_t rng;
    pcg32_srandom_r(&rng, time(NULL) ^ (intptr_t)&printf,(intptr_t)&rounds);
    for (int i = 0; i < rounds; ++i)
    {
        rands[i]=(pcg32_boundedrand_r(&rng,bound));
    }
}


// Taken from itsme86
// https://www.linuxquestions.org/questions/programming-9/replace-a-substring-with-another-string-in-c-170076/
char *replace_str(char *str, const char *orig, char *rep)
{
  static char buffer[4096];
  char *p;

  if(!(p = strstr(str, orig))) 
    // Original function had:
    // 		return str;
    // This is changed to account for multiple occurences.
  	return NULL;

  strncpy(buffer, str, p-str); 
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  return buffer;
}