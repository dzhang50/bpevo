// Author: Hongliang Gao;   Created: Jan 27 2011
// Description: sample predictors for cbp3.

#include <stdio.h>
#include <cassert>
#include <string.h>
#include <inttypes.h>

using namespace std;
#include "cbp3_def.h"
#include "cbp3_framework.h"

// this file includes two sample predictors
// one is a 64 KB gshare conditional predictor
// the other is a 64 KB indirect predictor indexed by (pc ^ history)

// rewind_marked is also used to show how to rewind the reader for multiple runs
// the predictor will use gshare in the first run
// the indirect predictor will be used in the second run

// NOTE: rewind_marked is only provided to help tuning work. the final
// submitted code should only include one run.


#define GSHARE_SIZE 18 // 256K 2-bit counters = 64 KB cost
#define BIMODAL_SIZE 19    // 512K 1-bit counters  = 64 KB cost
#define META_SIZE 18

// predictor tables
int8_t   *gshare_table;
bool   *bimodal_table;
int8_t   *meta_table;

// two branch history registers:
// the framework provids real branch results at fetch stage to simplify branch history
// update for predicting later branches. however, they are not available until execution stage
// in a real machine. therefore, you can only use them to update predictors at or after the
// branch is executed.
// in this sample code, we update predictors at retire stage where uops are processed
// in order to enable easy regneration of branch history.

// cost: depending on predictor size
uint32_t brh_fetch;
uint32_t brh_retire;

uint32_t GetMetaIndex(uint32_t brh, uint32_t pc)
{
  return  (pc & ((1 << META_SIZE) - 1));
}

uint32_t GetGShareIndex(uint32_t brh, uint32_t pc)
{
  return ((brh ^ pc) & ((1 << GSHARE_SIZE) - 1)); 
}

uint32_t GetBimodalIndex(uint32_t brh, uint32_t pc)
{
  return  (pc & ((1 << BIMODAL_SIZE) - 1));
}

void UpdateMetaTable(int8_t bimodalResult, int8_t gshareResult, int8_t* m_table, uint32_t midx)
{  
  int8_t diff = bimodalResult - gshareResult;
  if ((diff > 0) && m_table[midx] < 1)
    m_table[midx] += diff;
  else if ((diff < 0) && m_table[midx] > -2)
    m_table[midx] += diff;
}

void UpdateGTable(bool result, int8_t* table, uint32_t idx)
{  
  if (result && table[idx] < 1)
    table[idx] ++;
  else if (!result && table[idx] > -2)
    table[idx] --;
}

void UpdateBimodalTable(bool result, bool* table, uint32_t idx)
{
  table[idx] = result;
}

void PredictorInit() {
  //runs = 0;
  gshare_table = new int8_t[1 << GSHARE_SIZE];
  bimodal_table = new bool[1 << BIMODAL_SIZE];
  meta_table = new int8_t[1 << META_SIZE];
  assert(gshare_table && bimodal_table && meta_table);
}

void PredictorReset() {
  // this function is called before EVERY run
  // it is used to reset predictors and change configurations

 
  printf("Predictor:hybrid(bimodal/gshare)\nconfig: %i bimodal targets, %i gshare targets, %i meta-predictor targets,  %i KB total cost\n", 
	(1 << BIMODAL_SIZE),
	(1 << GSHARE_SIZE),
	(1 << META_SIZE),
	((1 << BIMODAL_SIZE) / 8 / 1024) + ((1 << GSHARE_SIZE) * 2 / 8 / 1024) +  ((1 << META_SIZE) * 2 / 8 / 1024));

  for (int i = 0; i < (1 << GSHARE_SIZE); i ++)
    gshare_table[i] = 0;
  for (int i = 0; i < (1 << BIMODAL_SIZE); i ++)
    bimodal_table[i] = false;
  for (int i = 0; i < (1 << META_SIZE); i ++)
    meta_table[i] = 0;

  brh_fetch = 0;
  brh_retire = 0;
}

void PredictorRunACycle() {
  // get info about what uops are processed at each pipeline stage
  const cbp3_cycle_activity_t *cycle_info = get_cycle_info();

  // make prediction at fetch stage
  for (int i = 0; i < cycle_info->num_fetch; i++) {
    uint32_t fe_ptr = cycle_info->fetch_q[i];
    const cbp3_uop_dynamic_t *uop = &fetch_entry(fe_ptr)->uop;

    bool pred;

    if (uop->type & IS_BR_CONDITIONAL)
      {
	uint32_t meta_idx = GetMetaIndex(brh_fetch, uop->pc);
	bool useBimodal = (meta_table[meta_idx] >= 0);

	if (useBimodal) 
	  {
	    pred = bimodal_table[GetBimodalIndex(brh_fetch, uop->pc)];
	  }
	else
	  {
	    pred = (gshare_table[GetGShareIndex(brh_fetch,uop->pc)] >= 0); 
	  }

	assert(report_pred(fe_ptr, false, pred));
      }

    // update fetch branch history
    if (uop->type & IS_BR_CONDITIONAL)
      brh_fetch = (brh_fetch << 1) | (uop->br_taken ? 1 : 0);
    else if (uop_is_branch(uop->type))
      brh_fetch = (brh_fetch << 1) | 1;
  }

  for (int i = 0; i < cycle_info->num_retire; i++) {
    uint32_t rob_ptr = cycle_info->retire_q[i];
    const cbp3_uop_dynamic_t *uop = &rob_entry(rob_ptr)->uop;

    uint32_t gshare_idx;
    uint32_t meta_idx;
    uint32_t bimodal_idx;

    bool bimodalPrediction;
    bool gsharePrediction;

    if (uop->type & IS_BR_CONDITIONAL)
      {
	meta_idx = GetMetaIndex(brh_retire,uop->pc);
	gshare_idx = GetGShareIndex(brh_retire,uop->pc);
	bimodal_idx = GetBimodalIndex(brh_retire,uop->pc);
	
	bool usedBimodal = (meta_table[meta_idx] >= 0);
	bimodalPrediction =  bimodal_table[bimodal_idx];
	gsharePrediction = (gshare_table[gshare_idx] >= 0);

	bool t = uop->br_taken;	
	// update meta predictor
	UpdateMetaTable((t == bimodalPrediction)?1:0, (t == gsharePrediction)?1:0, meta_table, meta_idx);

	// update both predictor 
	if (usedBimodal)
	  UpdateBimodalTable(t, bimodal_table, bimodal_idx);
	else
	  UpdateGTable(t, gshare_table, gshare_idx);
      }

    // update retire branch history
    if (uop->type & IS_BR_CONDITIONAL)
      brh_retire = (brh_retire << 1) | (uop->br_taken ? 1 : 0);
    else if (uop_is_branch(uop->type))
      brh_retire = (brh_retire << 1) | 1;
  }
}

void PredictorRunEnd() {
  //assert(brh_fetch == brh_retire);
}

void PredictorExit() {
  delete [] gshare_table;
  delete [] bimodal_table;
  delete [] meta_table;
}
