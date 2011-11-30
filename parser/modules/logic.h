#ifndef HELPER
#define HELPER
#include "helper.h"
#endif

dynamic_bitset<> XOR (vector<dynamic_bitset<> > inputs)
{
    size_t numInputs = inputs.size();   
    assert(numInputs > 0);

    dynamic_bitset<> result = inputs[0];

    for (size_t i = 1; i < numInputs; i++)
    {
	result ^= inputs[i];
    }

    return result;
}
