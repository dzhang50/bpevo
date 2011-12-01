#ifndef TABLES
#include "table.h"
#endif

#ifndef LOGIC
#include "logic.h"
#endif

#include <iostream>

ulong numEntries = 14ul;
TABLE bimodal = TABLE(numEntries);
EQUAL equalInst = EQUAL();
XOR test = XOR();

int main()
{
    //equalInst = EQUAL();
    vector<dynamic_bitset<> > out_idx;

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
//	cout << output[i] << ", MSB:" << MSB().Invocate(output[i])<< endl;
	cout << bimodal.Invocate(dynamic_bitset<> (4,i),in_data,  dynamic_bitset<> (4,0), dynamic_bitset<> (1,0ul)) << ", MSB:" << MSB().Invocate(output[i])<< endl;
    }
/*
    vector<dynamic_bitset<> > single;
    single.push_back(output[5]);
    single.push_back(output[10]);
*/
    
    dynamic_bitset<> XORed = XOR().Invocate(output[5], output[10]);
    cout << "XOR:" << endl << XORed << endl;

    dynamic_bitset<> equal = equalInst.Invocate(output[0], output[0]);
    cout << "EQUAL:" << endl << equal << endl;

    
    ulong width = 2ul;
    ulong twoBitEntries = 3ul;
    TABLE_2BITCNTR twoBitCounter(twoBitEntries);
    
    out_idx.clear();

    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,0ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,0ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,1ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,0ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul));
    /* twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,0ul),  dynamic_bitset<> (2,1ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,0ul),  dynamic_bitset<> (2,1ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,0ul),  dynamic_bitset<> (2,1ul), dynamic_bitset<> (1,1ul));
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul)); 
    twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul));
    //twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,0ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,1ul));
    */  
        
    out_idx.push_back(dynamic_bitset<> (1,0ul));
    out_idx.push_back(dynamic_bitset<> (1,1ul));
    output = twoBitCounter.Invocate(out_idx,dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,0ul));

    cout << endl;
    for (ulong i = 0; i < output.size(); i++)
    {
	cout << "2-bit counter[" << i << "]:" << output[i] << ", MSB:"<< MSB().Invocate(twoBitCounter.Invocate(dynamic_bitset<> (1,i),dynamic_bitset<> (1,1ul),  dynamic_bitset<> (2,0ul), dynamic_bitset<> (1,0ul))) << endl;
    }
    
    
}
