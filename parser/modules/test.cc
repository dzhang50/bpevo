#include "table.h"
#include <iostream>

int main()
{
    dynamic_bitset<> numEntries(4,5);
    vector<dynamic_bitset<> > out_idx;
    Table bimodal(numEntries);   

    dynamic_bitset<> in_data;
    for (int i = 0; i < numEntries.to_ulong(); i++)
    {
	in_data=dynamic_bitset<>(4,i);
	bimodal.Invocate(out_idx,in_data, dynamic_bitset<> (4,i),dynamic_bitset<> (1,1));
	out_idx.push_back( dynamic_bitset<> (4,i));
    }

    vector<dynamic_bitset<> > output = bimodal.Invocate(out_idx,in_data,  dynamic_bitset<> (4,0), dynamic_bitset<> (1,0));
 
    for (int i = 0; i < output.size(); i++)
    {
	cout << output[i] << ",";
    }
    cout << endl;
    
}
