#ifndef PREDICTOR_HEADERFILE_H
#define PREDICTOR_HEADERFILE_H

#include "../readerlib/cbp3_def.h"
#include "../cbp3_framework.h"

// Global variables

//extern cbp3_cycle_activity_t *cycle_info;
extern int numFetch;
extern int numRetire;
extern uint32_t fe_ptr;
extern uint32_t retire_ptr;
extern const cbp3_uop_dynamic_t *fe_uop;
extern const cbp3_uop_dynamic_t *retire_uop;
extern dynamic_bitset<> readValid;
extern dynamic_bitset<> writeValid;

const uint MAX_HISTORY_LENGTH = 128;

dynamic_bitset<> brh_fetch(MAX_HISTORY_LENGTH);
dynamic_bitset<> brh_retire(MAX_HISTORY_LENGTH);


// Global functions

int max(int a, int b) {
  if(a > b) {
    return a;
  }
  else {
    return b;
  }
}

void updateSpecialState()
{
    if (readValid[0])
    {
	if (fe_uop->type & IS_BR_CONDITIONAL)
	{
	    brh_fetch <<=1;
	    brh_fetch.set(0, fe_uop->br_taken);
	}
	else if (uop_is_branch(fe_uop->type))
	{
	    brh_fetch <<=1;
	    brh_fetch.set(0, true);
	}
    }

    if (writeValid[0])
    {
	if (retire_uop->type & IS_BR_CONDITIONAL)
	{
	    brh_retire <<=1;
	    brh_retire.set(0, retire_uop->br_taken);
	}
	else if (uop_is_branch(retire_uop->type))
	{
	    brh_retire <<=1;
	    brh_retire.set(0, true);
	}
    }
}

#endif
