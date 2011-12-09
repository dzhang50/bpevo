#include <stdio.h>
#include <cassert>
#include <string.h>
#include <inttypes.h>

#include <cbp3_def.h>
#include <cbp3_framework.h>

#ifndef TABLES
#include <table.h>
#endif

#ifndef LOGIC
#include <logic.h>
#endif

#ifndef GHR
#include <ghr.h>
#endif

using namespace std;

int numFetch;
int numRetire;
uint32_t fe_ptr;
uint32_t rob_ptr;
const cbp3_uop_dynamic_t *fe_uop;
const cbp3_uop_dynamic_t *retire_uop;

dynamic_bitset<> readValid;
dynamic_bitset<> writeValid;
dynamic_bitset<> readPC;
dynamic_bitset<> writePC;
dynamic_bitset<> writeTaken;
dynamic_bitset<> writeMispredicted;


TABLE_CNTR module_table_cntr_0 = TABLE_CNTR(16384, 1);
GHR_FETCH module_ghr_fetch_0 = GHR_FETCH(32);
GHR_RETIRE module_ghr_retire_0 = GHR_RETIRE(32);
XOR module_xor_0 = XOR();
XOR module_xor_1 = XOR();
TABLE_2BITCNTR module_table_2bitcntr_0 = TABLE_2BITCNTR(16384);
HASH module_hash_0 = HASH(32);
HASH module_hash_1 = HASH(32);
TABLE module_table_0 = TABLE(16384, 32);
EQUAL module_equal_0 = EQUAL();
MUX module_prediction = MUX();


void PredictorInit() {}

void PredictorReset() {}

void PredictorRunACycle() {
  // get info about what uops are processed at each pipeline stage
  const cbp3_cycle_activity_t *cycle_info = get_cycle_info();
  numFetch = cycle_info->num_fetch;
  numRetire = cycle_info->num_retire;
  for(int i = 0; i < max(numFetch, numRetire); i++) {
    fe_ptr = cycle_info->fetch_q[i];
    rob_ptr = cycle_info->retire_q[i];
    fe_uop = &fetch_entry(fe_ptr)->uop;
    retire_uop = &rob_entry(rob_ptr)->uop;

    // Assign static variables
    readValid = dynamic_bitset<>(1, 0ul);
    writeValid = dynamic_bitset<>(1, 0ul);
    if((i < numFetch) && (fe_uop->type & IS_BR_CONDITIONAL)) {
      readValid[0] = true;
    }
    if((i < numRetire) && (retire_uop->type & IS_BR_CONDITIONAL)) {
      writeValid[0] = true;
    }

    readPC = dynamic_bitset<>(32, fe_uop->pc);
    writePC = dynamic_bitset<>(32, retire_uop->pc);
    writeTaken = dynamic_bitset<>(1, retire_uop->br_taken);
    writeMispredicted = dynamic_bitset<>(1, 0ul);
    writeMispredicted[0] = (rob_entry(rob_ptr)->last_pred == retire_uop->br_taken);

    // For special modules, some additional processing may be necessary
    // ---------------- GENERATED LOGIC HERE -----------------


    dynamic_bitset<> table_cntr_0 = module_table_cntr_0.Invocate(readPC, writeTaken, writePC, writeValid);
    dynamic_bitset<> ghr_fetch_0 = module_ghr_fetch_0.Invocate();
    dynamic_bitset<> ghr_retire_0 = module_ghr_retire_0.Invocate();
    dynamic_bitset<> xor_0 = module_xor_0.Invocate(ghr_fetch_0, readPC);
    dynamic_bitset<> xor_1 = module_xor_1.Invocate(ghr_retire_0, writePC);
    dynamic_bitset<> table_2bitcntr_0 = module_table_2bitcntr_0.Invocate(xor_0, writeTaken, xor_1, writeValid);
    dynamic_bitset<> hash_0 = module_hash_0.Invocate(readPC);
    dynamic_bitset<> hash_1 = module_hash_1.Invocate(writePC);
    dynamic_bitset<> table_0 = module_table_0.Invocate(xor_0, hash_1, xor_1, writeValid);
    dynamic_bitset<> equal_0 = module_equal_0.Invocate(table_0, hash_0);
    dynamic_bitset<> prediction = module_prediction.Invocate(equal_0, table_cntr_0, table_2bitcntr_0);



    // Report prediction
    if(readValid[0]) {
      assert(report_pred(fe_ptr, false, prediction[0]));
    }

    updateSpecialState();

  }
}


void PredictorRunEnd() {}

void PredictorExit() {}

