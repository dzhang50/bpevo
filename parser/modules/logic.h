#ifndef LOGIC
#define LOGIC
#endif

#ifndef HELPER
#include "helper.h"
#endif

enum OPERATION{AND_OP, XOR_OP, OR_OP};

class MUX
{
public:
    dynamic_bitset<> Invocate (dynamic_bitset<> sel, vector<dynamic_bitset<> > inputs);
    dynamic_bitset<> Invocate (dynamic_bitset<> sel, dynamic_bitset<> in1, dynamic_bitset<> in2);
};

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
    size_t maxSize;

    size_t numInputs = inputs.size();   
    assert(numInputs > 0);

    dynamic_bitset<> result = inputs[0];

    maxSize = result.size();

    for (size_t i = 1; i < numInputs; i++)
    {
	if (inputs[i].size() > maxSize)
	    maxSize = inputs[i].size();
	result.resize(maxSize);
	inputs[i].resize(maxSize);

	switch(op)
	{
	case AND_OP:
	    result &= inputs[i];
	    break;
	case XOR_OP:
	    result ^= inputs[i];
	    break;
	case OR_OP:
	    result |= inputs[i];
	    break;
	default:
	    assert(false);
	}
    }

    return result;
}

dynamic_bitset<> MUX::Invocate (dynamic_bitset<> sel, vector<dynamic_bitset<> > inputs)
{
    size_t numInputs = inputs.size();
    //size_t lengthlimit = sizeof(ulong) * 8;
    if (sel.size() > lengthlimit)
	sel.resize(lengthlimit);
    //cout << "MUX index\n" << endl;
    assert (sel.size() <= 64);
    size_t index = (sel.to_ulong() % numInputs);
    return inputs[index];
}

dynamic_bitset<> MUX::Invocate (dynamic_bitset<> sel, dynamic_bitset<> in1, dynamic_bitset<> in2)
{
    vector<dynamic_bitset<> > vector;
    vector.push_back(in1);
    vector.push_back(in2);
    return MUX::Invocate(sel, vector);
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
    assert(false);
}

dynamic_bitset<> ADD::Invocate (dynamic_bitset<> in1, dynamic_bitset<> in2)
{
    size_t resultSize = max(in1.size(), in2.size()) + 1;
    dynamic_bitset<> result(resultSize);
    size_t minSize = min(in1.size(), in2.size());
    bool carry = false;
    uint temp = 0;
    size_t i;
    for (i = 0; i < minSize; i++)
    {
	temp = carry + in1[i] + in2[i];
	//cout << "temp:" << temp << endl;
	switch(temp)
	{
	case 0:
	    result[i] = false;
	    carry = false;
	    //assert (!carry && !in1[i] && !in2[i]);
	    break;
	case 1:
	    result[i] = true;
	    carry = false;
	    //assert (!carry && !in1[i] && !in2[i]);
	    break;
	case 2:
	    result[i] = false;
	    carry = true;
	    break;
	case 3:
	    result[i] = true;
	    carry = true;
	    break;
	default:
	    assert(false);
	}
    }

    if (carry)
	result[i] = true;

    return result;
    
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


