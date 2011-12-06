#ifndef LOGIC
#define LOGIC
#endif

#ifndef HELPER
#include "helper.h"
#endif

enum OPERATION{AND_OP, XOR_OP, ADD_OP, OR_OP};

class CONCAT
{
public:
    dynamic_bitset<> Invocate (vector<dynamic_bitset<> > inputs);
    dynamic_bitset<> Invocate (dynamic_bitset<> in1, dynamic_bitset<> in2);
};

class ADD
{
public:
    dynamic_bitset<> Invocate (vector<dynamic_bitset<> > inputs);
    dynamic_bitset<> Invocate (dynamic_bitset<> in1, dynamic_bitset<> in2);
};

class OR
{
public:
    dynamic_bitset<> Invocate (vector<dynamic_bitset<> > inputs);
    dynamic_bitset<> Invocate (dynamic_bitset<> in1, dynamic_bitset<> in2);
};

class AND
{
public:
    dynamic_bitset<> Invocate (vector<dynamic_bitset<> > inputs);
    dynamic_bitset<> Invocate (dynamic_bitset<> in1, dynamic_bitset<> in2);
};

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

dynamic_bitset<> PerformLogic (vector<dynamic_bitset<> > inputs, OPERATION op)
{
    size_t minSize;

    size_t numInputs = inputs.size();   
    assert(numInputs > 0);

    dynamic_bitset<> result = inputs[0];
    ulong resultSum = result.to_ulong();
    minSize = result.size();

    for (size_t i = 1; i < numInputs; i++)
    {
	if (inputs[i].size() < minSize)
	    minSize = inputs[i].size();
	result.resize(minSize);
	inputs[i].resize(minSize);

	switch(op)
	{
	case AND_OP:
	    result &= inputs[i];
	    break;
	case XOR_OP:
	    result ^= inputs[i];
	    break;
	case ADD_OP:
	    resultSum += inputs[i].to_ulong();
	    break;
	case OR_OP:
	    result |= inputs[i];
	    break;
	default:
	    assert(false);
	}
    }

    return (op == ADD_OP)?dynamic_bitset<>(minSize, resultSum):result;
}

dynamic_bitset<> CONCAT::Invocate (vector<dynamic_bitset<> > inputs)
{
    ostringstream bitsetConcat;
    size_t numInputs = inputs.size();

    for (size_t i = 0; i < numInputs; i++)
    {
	bitsetConcat << inputs[i];
    }

    dynamic_bitset<> returnVal(bitsetConcat.str());
    return returnVal;
}

dynamic_bitset<> CONCAT::Invocate (dynamic_bitset<> in1, dynamic_bitset<> in2)
{
    vector<dynamic_bitset<> > vector;
    vector.push_back(in1);
    vector.push_back(in2);
    return CONCAT::Invocate(vector);
}

dynamic_bitset<> OR::Invocate (vector<dynamic_bitset<> > inputs)
{
    return (PerformLogic(inputs, OR_OP));
}

dynamic_bitset<> OR::Invocate (dynamic_bitset<> in1, dynamic_bitset<> in2)
{
    vector<dynamic_bitset<> > vector;
    vector.push_back(in1);
    vector.push_back(in2);
    return OR::Invocate(vector);
}

dynamic_bitset<> ADD::Invocate (vector<dynamic_bitset<> > inputs)
{
    return (PerformLogic(inputs, ADD_OP));
}

dynamic_bitset<> ADD::Invocate (dynamic_bitset<> in1, dynamic_bitset<> in2)
{
    vector<dynamic_bitset<> > vector;
    vector.push_back(in1);
    vector.push_back(in2);
    return ADD::Invocate(vector);
}

dynamic_bitset<> AND::Invocate (vector<dynamic_bitset<> > inputs)
{
    return (PerformLogic(inputs, AND_OP));
}

dynamic_bitset<> AND::Invocate (dynamic_bitset<> in1, dynamic_bitset<> in2)
{
    vector<dynamic_bitset<> > vector;
    vector.push_back(in1);
    vector.push_back(in2);
    return AND::Invocate(vector);
}


dynamic_bitset<> XOR::Invocate (vector<dynamic_bitset<> > inputs)
{
    return PerformLogic ( inputs, XOR_OP);
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


