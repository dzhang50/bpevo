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
    dynamic_bitset<> *table;
    ulong numEntries; 
public:
    TABLE (ulong numEntriesInput);
    vector<dynamic_bitset<> > Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
    dynamic_bitset<> Invocate(dynamic_bitset<> out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
    void Initialize();
};

class TABLE_CNTR : public TABLE
{
    ulong width;
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


TABLE::TABLE (ulong numEntriesInput)
{
    numEntries = numEntriesInput;
    table = new dynamic_bitset<>[numEntries];
    Initialize();
}

void TABLE::Initialize()
{
    for (ulong i = 0; i < numEntries; i++)
    {
	table[i].reset();
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
	index = out_idx[i].to_ulong();
	index %= numEntries;
	output.push_back(table[index]);
    }

    if (MSB().Invocate(in_enable).test(0))
    {
	index = in_idx.to_ulong();
	index %= numEntries;
	table[index] = in_data;
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

TABLE_CNTR::TABLE_CNTR(ulong numEntriesInput, ulong widthInput) : TABLE(numEntriesInput), width(widthInput)
{
    TABLE_CNTR::Initialize();   
}

void TABLE_CNTR::Initialize()
{
    ulong weaklyTaken = ((1 << (width - 1)) - 1);
    for (ulong i = 0; i < numEntries; i++)
    {
	table[i].resize(width);
	table[i] = dynamic_bitset<>(width, weaklyTaken);
    }
}

vector<dynamic_bitset<> > TABLE_CNTR::Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable)
{
    ulong numReads = out_idx.size();
    vector<dynamic_bitset<> > output;
    ulong index;
    
    for (ulong i = 0; i < numReads; i++)
    {
	index = out_idx[i].to_ulong();
	index %= numEntries;
	output.push_back(table[index]);
    }

    if (MSB().Invocate(in_enable).test(0))
    {
	index = in_idx.to_ulong();
	index %= numEntries;

	bool increment = in_data.any();
	ulong max = ((1 << width) -1);

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





