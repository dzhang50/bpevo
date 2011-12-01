#ifndef TABLE
#include "table.h"
#endif

#ifndef LOGIC
#include "logic.h"
#endif

#include <iostream>

int main()
{
    ulong numEntries = 14ul;
    vector<dynamic_bitset<> > out_idx;

    Table bimodal(numEntries);  

    dynamic_bitset<> in_data;
    for (ulong i = 0ul; i < numEntries; i++)
    {
	in_data=dynamic_bitset<>(4,i);
	bimodal.Invocate(out_idx,in_data, dynamic_bitset<> (4,i),dynamic_bitset<> (1,1ul));
	out_idx.push_back( dynamic_bitset<> (4,i));
    }

    vector<dynamic_bitset<> > output = bimodal.Invocate(out_idx,in_data,  dynamic_bitset<> (4,0), dynamic_bitset<> (1,0ul));

 
    for (ulong i = 0; i < output.size(); i++)
    {
	cout << output[i] << ", MSB:" << MSB(output[i])<< endl;
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

    
    ulong width = 2ul;
    ulong twoBitEntries = 3ul;
    Table_Cntr twoBitCounter(twoBitEntries, width);
    
    out_idx.clear();

    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,1ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,1ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,0ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,0ul),  dynamic_bitset<> (2,1ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,0ul),  dynamic_bitset<> (2,1ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,0ul),  dynamic_bitset<> (2,1ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,0ul),  dynamic_bitset<> (2,1ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul)); 
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul));
    //twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,0ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul));
        
    out_idx.push_back(dynamic_bitset<> (1,0ul));
    out_idx.push_back(dynamic_bitset<> (1,1ul));
    output = twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,0ul));

    cout << endl;
    for (ulong i = 0; i < output.size(); i++)
    {
	cout << "2-bit counter[" << i << "]:" << output[i] << ", MSB:"<<MSB(output[i]) << endl;
    }
    
    
}
