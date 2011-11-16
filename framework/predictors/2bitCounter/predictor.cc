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


#define BASE_PREDICTION_TABLE_SIZE 15 // 32K 1-bit counters = 4 KB cost
#define BASE_HYSTERESIS_TABLE_SIZE 13 // 8K 2-bit counters = 2 KB cost

// predictor tables
bool   *base_prediction_table;
int8_t *base_hysteresis_table;

// two branch history registers:
// the framework provids real branch results at fetch stage to simplify branch history
// update for predicting later branches. however, they are not available until execution stage
// in a real machine. therefore, you can only use them to update predictors at or after the
// branch is executed.
// in this sample code, we update predictors at retire stage where uops are processed
// in order to enable easy regneration of branch history.

// cost: depending on predictor size
//uint32_t brh_fetch;
//uint32_t brh_retire;

void PredictorInit() {
  base_prediction_table = new bool[1 << BASE_PREDICTION_TABLE_SIZE];
  base_hysteresis_table= new int8_t[1 << BASE_HYSTERESIS_TABLE_SIZE];
  assert(base_prediction_table && base_hysteresis_table);
}

void PredictorReset() {
  // this function is called before EVERY run
  // it is used to reset predictors and change configurations

  printf("Predictor:2-bit counter indexed by PC, shared hysteresis\nconfig: %i prediction table entries, %i shared hysteresis entries, %i KB cost\n",
	 1 << BASE_PREDICTION_TABLE_SIZE,
	 1 << BASE_HYSTERESIS_TABLE_SIZE,
	 ((1 << BASE_PREDICTION_TABLE_SIZE)/ 8 / 1024) + ((1 << BASE_HYSTERESIS_TABLE_SIZE)/ 4 / 1024)
      );
 
  for (int i = 0; i < (1 << BASE_PREDICTION_TABLE_SIZE); i ++)
    base_prediction_table[i] = false;

  for (int i = 0; i < (1 << BASE_HYSTERESIS_TABLE_SIZE); i++)
      base_hysteresis_table[i] = -1; // initial bias slightly to not taken
}

void PredictorRunACycle() {
  // get info about what uops are processed at each pipeline stage
  const cbp3_cycle_activity_t *cycle_info = get_cycle_info();

  // make prediction at fetch stage
  for (int i = 0; i < cycle_info->num_fetch; i++) {
    uint32_t fe_ptr = cycle_info->fetch_q[i];
    const cbp3_uop_dynamic_t *uop = &fetch_entry(fe_ptr)->uop;

    if (uop->type & IS_BR_CONDITIONAL) {
      // get prediction
      uint32_t idx = uop->pc & ((1 << BASE_PREDICTION_TABLE_SIZE) - 1);
      bool pred = base_prediction_table[idx];

      // report prediction:
      // you need to provide direction predictions for conditional branches,
      // targets of conditional branches are available at fetch stage.
      // for indirect branches, you need to provide target predictions.
      // you can report multiple predictions for the same branch
      // the framework will use the last reported prediction to calculate 
      // misprediction penalty
      assert(report_pred(fe_ptr, false, pred));
    }
  }

  for (int i = 0; i < cycle_info->num_retire; i++) {
    uint32_t rob_ptr = cycle_info->retire_q[i];
    const cbp3_uop_dynamic_t *uop = &rob_entry(rob_ptr)->uop;

    if (uop->type & IS_BR_CONDITIONAL) {
      uint32_t base_pred_idx = uop->pc & ((1 << BASE_PREDICTION_TABLE_SIZE) - 1);
      uint32_t base_hys_idx = uop->pc & ((1 << BASE_HYSTERESIS_TABLE_SIZE) - 1);

      // update predictor
      bool taken = uop->br_taken;
      bool prediction = base_prediction_table[base_pred_idx];

      if (taken && (base_hysteresis_table[base_hys_idx] < 1))
	  base_hysteresis_table[base_hys_idx]++;
      else if (!taken && (base_hysteresis_table[base_hys_idx] > -2))
	  base_hysteresis_table[base_hys_idx]--;

      if (taken != prediction)
	  base_prediction_table[base_pred_idx] = (base_hysteresis_table[base_hys_idx] >= 0);
    }
  }
}

void PredictorRunEnd() {
}

void PredictorExit() {
  delete [] base_prediction_table;
  delete [] base_hysteresis_table;
}
