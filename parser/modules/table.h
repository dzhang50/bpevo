#ifndef TABLES
#define TABLES
#endif

#ifndef HELPER
#include "helper.h"
#endif

#ifndef LOGIC
#include "logic.h"
#endif

class TABLE
{
protected:
    ulong width;
    dynamic_bitset<> *table;
    ulong numEntries; 
public:
    TABLE (ulong numEntriesInput, ulong widthInput);
    vector<dynamic_bitset<> > Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
    dynamic_bitset<> Invocate(dynamic_bitset<> out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
    void Initialize(ulong value);
};

class TABLE_CNTR : public TABLE
{
public:
    TABLE_CNTR(ulong numEntriesInput, ulong widthInput);
    vector<dynamic_bitset<> > Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
    dynamic_bitset<> Invocate(dynamic_bitset<> out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
    void Initialize();
};

class TABLE_2BITCNTR : TABLE_CNTR
{
public:
    TABLE_2BITCNTR(ulong numEntriesInput);
    vector<dynamic_bitset<> > Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
    dynamic_bitset<> Invocate(dynamic_bitset<> out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
};


TABLE::TABLE (ulong numEntriesInput, ulong widthInput)
{
    numEntries = numEntriesInput;
    width = widthInput;
    table = new dynamic_bitset<>[numEntries];
    Initialize(0);
}

void TABLE::Initialize(ulong value)
{
    for (ulong i = 0; i < numEntries; i++)
    {
	table[i].resize(width);
	table[i] = dynamic_bitset<>(width, value);
    }
}

vector<dynamic_bitset<> > TABLE::Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable)
{
    ulong numReads = out_idx.size();
    vector<dynamic_bitset<> > output;
    ulong index;
    
    for (ulong i = 0; i < numReads; i++)
    {
	//cout << "table read index\n" << endl;
	if (out_idx[i].size() > lengthlimit)
	    out_idx[i].resize(lengthlimit);
	assert (out_idx[i].size() <= 64);
	index = out_idx[i].to_ulong();
	index %= numEntries;
	output.push_back(table[index]);
    }
    assert (in_enable.size() > 0);
    if (in_enable.test(0))
    {
	//cout << "table write index\n" << endl;
	if (in_idx.size() > lengthlimit)
	    in_idx.resize(lengthlimit);
	assert (in_idx.size() <= 64);
	index = in_idx.to_ulong();
	index %= numEntries;
	table[index] = in_data;
	table[index].resize(width);
    }

    return output;
}

dynamic_bitset<> TABLE::Invocate(dynamic_bitset<> out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable)
{
    vector<dynamic_bitset<> > vector;
    vector.push_back(out_idx);
    return (TABLE::Invocate(vector, in_data, in_idx, in_enable)[0]);
    
}

TABLE_CNTR::TABLE_CNTR(ulong numEntriesInput, ulong widthInput) : TABLE(numEntriesInput,widthInput)
{
    ulong weaklyTaken = ((1 << (width - 1)) - 1);
    TABLE::Initialize(weaklyTaken);
}

vector<dynamic_bitset<> > TABLE_CNTR::Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable)
{
    ulong numReads = out_idx.size();
    vector<dynamic_bitset<> > output;
    ulong index;
    
    for (ulong i = 0; i < numReads; i++)
    {
	//cout << "counter table read index\n" << endl;
	if (out_idx[i].size() > lengthlimit)
	    out_idx[i].resize(lengthlimit);
	assert (out_idx[i].size() <= 64);
	index = out_idx[i].to_ulong();
	index %= numEntries;
	output.push_back(table[index]);
    }

    assert (in_enable.size() > 0);
    if (in_enable.test(0))
    {
	//cout << "counter table write index\n" << endl;
	if (in_idx.size() > lengthlimit)
	    in_idx.resize(lengthlimit);
	assert (in_idx.size() <= 64);
	index = in_idx.to_ulong();
	index %= numEntries;

	bool increment = in_data.any();
	ulong max = ((1 << width) -1);
	//cout << "counter table value\n" << endl;
	if (table[index].size() > lengthlimit)
	    table[index].resize(lengthlimit);
	assert (table[index].size() <= 64);
	ulong counter = table[index].to_ulong();
	if (increment && (counter < max))
	    counter++;
	else if (!increment && (counter > 0))
	    counter--;

	table[index] = dynamic_bitset<>(width, counter);
	
    }

    return output;
}

dynamic_bitset<> TABLE_CNTR::Invocate(dynamic_bitset<> out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable)
{
    vector<dynamic_bitset<> > vector;
    vector.push_back(out_idx);
    return (TABLE_CNTR::Invocate(vector, in_data, in_idx, in_enable)[0]);
    
}

TABLE_2BITCNTR::TABLE_2BITCNTR(ulong numEntriesInput) : TABLE_CNTR(numEntriesInput, 2) {}

vector<dynamic_bitset<> > TABLE_2BITCNTR::Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data, dynamic_bitset<> in_idx, dynamic_bitset<> in_enable)
{
    vector<dynamic_bitset<> > output = TABLE_CNTR::Invocate(out_idx, in_data, in_idx, in_enable);
    for (ulong i = 0; i < output.size(); i++)
    {
	output[i] = MSB().Invocate(output[i]);
    }
    return output;
}

dynamic_bitset<> TABLE_2BITCNTR::Invocate(dynamic_bitset<> out_idx, dynamic_bitset<> in_data, dynamic_bitset<> in_idx, dynamic_bitset<> in_enable)
{
    vector<dynamic_bitset<> > input;
    input.push_back(out_idx);
    return (TABLE_2BITCNTR::Invocate(input, in_data, in_idx, in_enable)[0]);
}





