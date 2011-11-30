#include "table.h"
#include "logic.h"
#include <iostream>

int main()
{
    dynamic_bitset<> numEntries(4,14ul);
    vector<dynamic_bitset<> > out_idx;

    Table bimodal(numEntries);  

    dynamic_bitset<> in_data;
    for (ulong i = 0ul; i < numEntries.to_ulong(); i++)
    {
	in_data=dynamic_bitset<>(4,i);
	bimodal.Invocate(out_idx,in_data, dynamic_bitset<> (4,i),dynamic_bitset<> (1,1ul));
	out_idx.push_back( dynamic_bitset<> (4,i));
    }

    vector<dynamic_bitset<> > output = bimodal.Invocate(out_idx,in_data,  dynamic_bitset<> (4,0), dynamic_bitset<> (1,0ul));
 
    for (ulong i = 0; i < output.size(); i++)
    {
	cout << output[i] << endl;
    }
/*
    vector<dynamic_bitset<> > single;
    single.push_back(output[5]);
    single.push_back(output[10]);
*/
    
    dynamic_bitset<> XORed = XOR(output);
    cout << "XOR:" << endl << XORed << endl;

    

    dynamic_bitset<> equal = EQUAL(output);
    cout << "EQUAL:" << endl << equal << endl;
    
    
}
