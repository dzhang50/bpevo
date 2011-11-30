#ifndef HELPER
#define HELPER
#include "helper.h"
#endif

class Table
{
    dynamic_bitset<> *table;
public:
    ulong numEntries; 
    Table (dynamic_bitset<> numEntriesInput);
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



