#ifndef LOGIC
#define LOGIC
#endif

#ifndef HELPER
#include "helper.h"
#endif

class XOR
{
public:
    dynamic_bitset<> Invocate (vector<dynamic_bitset<> > inputs);
    dynamic_bitset<> Invocate (dynamic_bitset<> in1, dynamic_bitset<> in2);
};

class EQUAL
{
public:
    dynamic_bitset<> Invocate (vector<dynamic_bitset<> > inputs);
    dynamic_bitset<> Invocate (dynamic_bitset<> in1, dynamic_bitset<> in2);
};

class MSB
{
public:
    dynamic_bitset<> Invocate (dynamic_bitset<> input);
};


dynamic_bitset<> XOR::Invocate (vector<dynamic_bitset<> > inputs)
{
    size_t minSize;

    size_t numInputs = inputs.size();   
    assert(numInputs > 0);

    dynamic_bitset<> result = inputs[0];
    minSize = result.size();

    for (size_t i = 1; i < numInputs; i++)
    {
	if (inputs[i].size() < minSize)
	    minSize = inputs[i].size();
	result.resize(minSize);
	inputs[i].resize(minSize);
	result ^= inputs[i];
    }

    return result;
}

dynamic_bitset<> XOR::Invocate (dynamic_bitset<> in1, dynamic_bitset<> in2)
{
    vector<dynamic_bitset<> > vector;
    vector.push_back(in1);
    vector.push_back(in2);
    return XOR::Invocate(vector);
}

dynamic_bitset<> EQUAL::Invocate (vector<dynamic_bitset<> > inputs)
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

dynamic_bitset<> EQUAL::Invocate (dynamic_bitset<> in1, dynamic_bitset<> in2)
{
    vector<dynamic_bitset<> > vector;
    vector.push_back(in1);
    vector.push_back(in2);
    return EQUAL::Invocate(vector);
}

dynamic_bitset<> MSB::Invocate (dynamic_bitset<> input)
{
    bool msbSet = input[input.size()-1];
    return (msbSet?(dynamic_bitset<>(1,1ul)):(dynamic_bitset<>(1, 0ul)));
}


