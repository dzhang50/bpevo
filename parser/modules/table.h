#include "helper.h"

class Table
{
    dynamic_bitset<> *table;
public:
    int numEntries; 
    Table (dynamic_bitset<> numEntriesInput);
    vector<dynamic_bitset<> > Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
};

Table::Table (dynamic_bitset<> numEntriesInput)
{
    ulong numEntries = numEntriesInput.to_ulong();
    table = new dynamic_bitset<>[numEntries];
    for (int i = 0; i < numEntries; i++)
    {
	table[i].reset();
    }
}

vector<dynamic_bitset<> > Table::Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable)
{
    int numReads = out_idx.size();
    vector<dynamic_bitset<> > output;
    int index;
    
    for (int i = 0; i < numReads; i++)
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



