#ifndef GHR
#define GHR
#endif

#ifndef HELPER
#include "helper.h"
#endif

#ifndef LOGIC
#include "logic.h"
#endif

#ifndef PREDICTOR_HEADERFILE_H
#include "predictor.h"
#endif

extern const uint MAX_HISTORY_LENGTH;
extern dynamic_bitset<> brh_fetch;
extern dynamic_bitset<> brh_retire;

class GHR_FETCH
{
protected:
    int length;
public:
    GHR_FETCH(int lengthInput);
    dynamic_bitset<> Invocate();
};

class GHR_RETIRE
{
protected:
    int length;
public:
    GHR_RETIRE(int lengthInput);
    dynamic_bitset<> Invocate();
};

GHR_FETCH::GHR_FETCH(int lengthInput): length(lengthInput){}
dynamic_bitset<> GHR_FETCH::Invocate()
{
    dynamic_bitset<> returnVal = brh_fetch;
    returnVal.resize(length);
    //assert(brh_fetch.size() == MAX_HISTORY_LENGTH);
    return returnVal;
}

GHR_RETIRE::GHR_RETIRE(int lengthInput): length(lengthInput){}
dynamic_bitset<> GHR_RETIRE::Invocate()
{
    dynamic_bitset<> returnVal = brh_retire;
    returnVal.resize(length);
    //assert(brh_retire.size() == MAX_HISTORY_LENGTH);
    return returnVal;
}









