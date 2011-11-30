#ifndef HELPER
#define HELPER
#include "helper.h"
#endif

class Table
{
protected:
    dynamic_bitset<> *table;
public:
    ulong numEntries; 
    Table (dynamic_bitset<> numEntriesInput);
    vector<dynamic_bitset<> > Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
};

class Table_Cntr : public Table
{
    ulong width;
public:
    Table_Cntr(dynamic_bitset<> numEntriesInput, dynamic_bitset<> width);
    vector<dynamic_bitset<> > Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
};

Table::Table (dynamic_bitset<> numEntriesInput)
{
    numEntries = numEntriesInput.to_ulong();
    table = new dynamic_bitset<>[numEntries];
    for (ulong i = 0; i < numEntries; i++)
    {
	table[i].reset();
    }
}

vector<dynamic_bitset<> > Table::Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
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

    if (in_enable.to_ulong() != 0)
    {
	index = in_idx.to_ulong();
	index %= numEntries;
	table[index] = in_data;
    }

    return output;
}

Table_Cntr::Table_Cntr(dynamic_bitset<> numEntriesInput, dynamic_bitset<> width) : Table(numEntriesInput), width(width.to_ulong())
{
    for (ulong i = 0; i < numEntries; i++)
    {
	table[i].resize(this->width);
	table[i].reset();
    }
}

vector<dynamic_bitset<> > Table_Cntr::Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
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

    if (in_enable.any())
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





