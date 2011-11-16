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
#define GSELECT_SIZE 18    // 256K 2-bit counters  = 64 KB cost
#define META_SIZE 18

// predictor tables
int8_t   *gshare_table;
int8_t   *gselect_table;
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

// count number of runs
uint32_t runs;

uint32_t GetMetaIndex(uint32_t brh, uint32_t pc)
{
  return  (pc & ((1 << META_SIZE) - 1));
}

uint32_t GetGShareIndex(uint32_t brh, uint32_t pc)
{
  return ((brh ^ pc) & ((1 << GSHARE_SIZE) - 1)); 
}

uint32_t GetGSelectIndex(uint32_t brh, uint32_t pc)
{
  const uint32_t even_bits_mask = 0x55555555;
  const uint32_t odd_bits_mask = 0xAAAAAAAA;
  uint32_t index = (even_bits_mask & brh) | (odd_bits_mask & pc);
  index %= (1 << GSELECT_SIZE);
  return index;
}

void UpdateMetaTable(int8_t gShareresult, int8_t gSelectresult, int8_t* m_table, uint32_t midx)
{  
  int8_t diff = gShareresult - gSelectresult;
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

void PredictorInit() {
  runs = 0;
  gshare_table = new int8_t[1 << GSHARE_SIZE];
  gselect_table = new int8_t[1 << GSELECT_SIZE];
  meta_table = new int8_t[1 << META_SIZE];
  assert(gshare_table && gselect_table && meta_table);
}

void PredictorReset() {
  // this function is called before EVERY run
  // it is used to reset predictors and change configurations

 
  printf("Predictor:hybrid(gshare/gselect)\nconfig: %i gshare targets, %i gselect targets, %i meta-predictor targets,  %i KB total cost\n", 
	(1 << GSHARE_SIZE),
	(1 << GSELECT_SIZE),
	(1 << META_SIZE),
	((1 << GSHARE_SIZE) * 2 / 8 / 1024) + ((1 << GSELECT_SIZE) * 2 / 8 / 1024) +  ((1 << META_SIZE) * 2 / 8 / 1024));

  for (int i = 0; i < (1 << GSHARE_SIZE); i ++)
    gshare_table[i] = 0;
  for (int i = 0; i < (1 << GSELECT_SIZE); i ++)
    gselect_table[i] = 0;
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

    uint32_t gidx;
    bool gpred;
    int8_t   *g_table;

    if (uop->type & IS_BR_CONDITIONAL)
      {
	uint32_t meta_idx = GetMetaIndex(brh_fetch, uop->pc);
	bool useGshare = (meta_table[meta_idx] >= 0);

	if (useGshare) 
	  {
	    // get prediction
	    gidx =  GetGShareIndex(brh_fetch,uop->pc); 
	    g_table = gshare_table;
	  }
	else
	  {
	    gidx = GetGSelectIndex(brh_fetch,uop->pc);
	    g_table = gselect_table;
	  }
	gpred = (g_table[gidx] >= 0);
	assert(report_pred(fe_ptr, false, gpred));
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
    uint32_t gselect_idx;
    uint32_t meta_idx;

    if (uop->type & IS_BR_CONDITIONAL)
      {
	meta_idx = GetMetaIndex(brh_retire,uop->pc);
	gshare_idx = GetGShareIndex(brh_retire,uop->pc);
	gselect_idx = GetGSelectIndex(brh_retire,uop->pc);
	
	bool usedGShare = (meta_table[meta_idx] >= 0);
	bool gSharePrediction = (gshare_table[gshare_idx] >= 0);
	bool gSelectPrediction = (gselect_table[gselect_idx] >= 0);

	bool t = uop->br_taken;	
	// update meta predictor
	UpdateMetaTable((t == gSharePrediction)?1:0, (t == gSelectPrediction)?1:0, meta_table, meta_idx);

	// update both predictor 
	if (usedGShare)
	  UpdateGTable(t, gshare_table, gshare_idx);
	else
	  UpdateGTable(t, gselect_table, gselect_idx);
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
  delete [] gselect_table;
  delete [] meta_table;
}
