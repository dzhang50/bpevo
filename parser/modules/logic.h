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

dynamic_bitset<> EQUAL (vector<dynamic_bitset<> > inputs)
{
    size_t numInputs = inputs.size();   
    assert(numInputs > 0);

    bool isEqual=true;
    dynamic_bitset<> value = inputs[0];

    for (size_t i = 1; i < numInputs; i++)
    {
	isEqual = (value == inputs[i]);
	if (!isEqual)
	    break;
    }

    dynamic_bitset<> result;
    result.push_back(isEqual);
	
    return result;
}

dynamic_bitset<> MSB (dynamic_bitset<> input)
{
    bool msbSet = input[input.size()-1];
    return (msbSet?(dynamic_bitset<>(1,1ul)):(dynamic_bitset<>(1, 0ul)));
}


