#include "simpleFlow.hpp"
#include <omp.h>

sflow::LocalGrid grid;
double total=0;

void process()
{
//#pragma omp parallel for reduction(+:total)
#pragma acc enter data copyin(grid.coords)
#pragma acc parallel loop    
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
    
}


int main(int argc, char *argv[])
{

    std::cout << "running on " << omp_get_max_threads() << " threads" << std::endl;
    
    double start = omp_get_wtime();
    grid.loadStarCD("trial");
    double end = omp_get_wtime();
    std::cout << "load time: " << (end-start) << std::endl;

    std::vector<int> ia;
    std::vector<int> ja;

    sflow::buildCSR(grid, ia, ja);
    
    std::cout << "csr: " << ia.size() << " ; " << ja.size() << std::endl;

    start = omp_get_wtime();
    process();
    end = omp_get_wtime();

    std::cout << "meaningless total: " << total << std::endl;
    std::cout << "time: " << (end-start) << std::endl;
    return 0;
}
