#ifndef TABLE
#define TABLE
#endif

#ifndef HELPER
#include "helper.h"
#endif

#ifndef LOGIC
#include "logic.h"
#endif

class Table
{
protected:
    dynamic_bitset<> *table;
public:
    ulong numEntries; 
    Table (ulong numEntriesInput);
    vector<dynamic_bitset<> > Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
    dynamic_bitset<> Invocate(dynamic_bitset<> out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
    void Initialize();
};

class Table_Cntr : public Table
{
    ulong width;
public:
    Table_Cntr(ulong numEntriesInput, ulong widthInput);
    vector<dynamic_bitset<> > Invocate(vector<dynamic_bitset<> > out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
    dynamic_bitset<> Invocate(dynamic_bitset<> out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable);
    void Initialize();
};


Table::Table (ulong numEntriesInput)
{
    numEntries = numEntriesInput;
    table = new dynamic_bitset<>[numEntries];
    Initialize();
}

void Table::Initialize()
{
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

    if (MSB().Invocate(in_enable).test(0))
    {
	index = in_idx.to_ulong();
	index %= numEntries;
	table[index] = in_data;
    }

    return output;
}

dynamic_bitset<> Table::Invocate(dynamic_bitset<> out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable)
{
    vector<dynamic_bitset<> > vector;
    vector.push_back(out_idx);
    return (Table::Invocate(vector, in_data, in_idx, in_enable)[0]);
    
}

Table_Cntr::Table_Cntr(ulong numEntriesInput, ulong widthInput) : Table(numEntriesInput), width(widthInput)
{
    Table_Cntr::Initialize();   
}

void Table_Cntr::Initialize()
{
    ulong weaklyTaken = ((1 << (width - 1)) - 1);
    for (ulong i = 0; i < numEntries; i++)
    {
	table[i].resize(width);
	table[i] = dynamic_bitset<>(width, weaklyTaken);
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

dynamic_bitset<> Table_Cntr::Invocate(dynamic_bitset<> out_idx, dynamic_bitset<> in_data,
				       dynamic_bitset<> in_idx, dynamic_bitset<> in_enable)
{
    vector<dynamic_bitset<> > vector;
    vector.push_back(out_idx);
    return (Table_Cntr::Invocate(vector, in_data, in_idx, in_enable)[0]);
    
}





