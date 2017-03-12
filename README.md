# Ant Colony Optimisation  
An OpenCL implementation for an ant system optimisation for the travelling salesperson problem.  

## How  
0. Set `platformToUse` under `Structure.h` to set GPU/CPU  
1. Run `make`  
2. Run:  
    + `ants` to see full list of arguments required (Eg. Number of ants, alpha/beta vals, graph set-up values, pheromonal update techniques)  
    + One of the `*.sh` scripts to run large scale tests  

## Design  
The host code is separated into three files `ants.c`, `BorrowedFunc.h` and `Structure.h`. `ant.c` is where the `main` is located and deals with the ant colony data structure and the iterations of ant execution. `Structure.h` deals with OpenCL data structures and parsing of kernel files. Finally, `BorrowedFunc.h` contains functions created by others and borrowed with :heart:.  

### Pheromones  
The default initial value for all pheromones is calculated as the inverse of the length random walk multiplied by the number of nodes present, influenced by the research performed by Sonigoswami et al, 2014 (ISSN : 2248 - 9622). 
The update value applied to pheromones can be performed in one sequential or parallel update (see flags), with the phermonal update function, g (the amount added to each edge), set as:
![](/graphics-graphs/pher-update.png)
Where S<sub>k</sub> is the solution from the k<sup>th</sup> ant. Note the parallel update is a more inefficient calculation, as since OpenCL does not allow for lock free editing of data values, instead each work item must have to work on a separate section of the pheromone array. Okay for evaporation, not for iterating over solutions.  

### Randomness  
In order to make the ants have truly random probable path following a combination of functions were used. First random seeds were generated on the host using the PCG algorithm [[O'Neill, 2015](http://www.pcg-random.org)] seeded following the given protocol (using the current time and the number of rounds required). The number of seeds generated is equal to the number of nodes, as this is how many decisions any one ant will need to make. Finally, during kernel execution - each kernel will add its global ID to the seed and perform an xorshift on this new unique seed, and generates a decimal value from 0 to 1 (inclusive). This gives enough of widely varying probabilities as seen in the kernel density of the results below.
![](/graphics-graphs/kernelDensity.png)

## Results  
![](/graphics-graphs/smallNodes.png)
![](/graphics-graphs/largeNodes.png)

The above figures are created from the `compiledParaVNorm` result dataset and show the difference from parallel and sequential pheromone update. Although sequential is much faster for small values of nodes this starts to change at around 40 nodes indicating at around this level of simulation that the complexity of the parallel update is negligible compared to the speedup gained by breaking the task into data parallel sections.  

Also note for both result datasets the loss of strong results. With this current implemented pheromone update and route selection taken from the final execution cycle - only 19 of 70 tests from `resultsNodes.csv` and 37 of the 188 from `compiledParaVNorm.csv` settled on the best solution found by at least one ant at one time of execution. Here's a time graph as well:  

![](/graphics-graphs/NodesvTime.png)

## Todo  
See `todo-and-notes`  