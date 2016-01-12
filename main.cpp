#include "simpleFlow.hpp"
#include <omp.h>


int main(int argc, char *argv[])
{
    sflow::LocalGrid grid;
    std::cout << "running on " << omp_get_max_threads() << " threads" << std::endl;

    
    double start = omp_get_wtime();
    grid.loadStarCD("trial");
    double end = omp_get_wtime();
    std::cout << "load time: " << (end-start) << std::endl;

    double total=0;

    

    start = omp_get_wtime();

#pragma omp parallel for reduction(+:total)
    for (unsigned int c=0; c<grid.tets.size(); c++)
    {
        for (int i=0; i<4; i++)
        {
            
            for (int z=0; z<3; z++)
            {
                total += grid.coords[ grid.tets[c][i] ][z];
            }
        }
    }

    end = omp_get_wtime();

    std::cout << "meaningless total: " << total << std::endl;
    std::cout << "time: " << (end-start) << std::endl;
    return 0;
}
