#include "utils.hpp"

std::string OutputData::PostFix;
std::string OutputData::CsvExtension;


unsigned int getClosestPowerOfTwo(unsigned int n)
{
    unsigned int m = 2;
    while(m < n) m<<=1;
    
    return m;
}

