// Author: Hongliang Gao;   Created: Jan 27 2011
// Description: sample predictors for cbp3.

#include <stdio.h>
#include <cassert>
#include <string.h>
#include <stdlib.h>
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

#define TABLE_SIZE 14 // 64K 8-bit counters = 64 KB cost
#define PATH_TABLE_SIZE 5
#define BIAS_SIZE 10

// predictor tables
int8_t   *weight_table;
int8_t   *bias_table;

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

uint32_t * path_table_fetch;
uint32_t * path_table_retire;

uint32_t hash(uint32_t a, uint32_t b, uint32_t c)
{
    uint32_t H1 =  511387;
    uint32_t H2 =  660509;
    uint32_t H3 =  1289381;

    uint32_t ha = a * H1;
    uint32_t hb = b * H2;
    uint32_t hc = c * H3;

    return (ha ^ hb ^ hc) % (1 << TABLE_SIZE);
}

int predict(uint32_t pc, uint32_t brh, int8_t * b_table, int8_t * w_table, uint32_t * p_table)
{
    int GHL = sizeof(brh) * 8;
    assert(sizeof(brh) == 4);
    int output = b_table[pc % (1 << BIAS_SIZE)];
  
    int brh_mask = 1;
    bool taken;
  
    for (int i = 0; i < GHL; i++)
    {
	taken = brh_mask & brh;
	if (taken)
	    output += w_table[hash(pc, p_table[i % (1 << PATH_TABLE_SIZE)], i)];
	else
	    output -= w_table[hash(pc, p_table[i % (1 << PATH_TABLE_SIZE)], i)];
    
	brh_mask = (brh_mask << 1);
    }

    return output;
}

void updateWeights(uint32_t pc, uint32_t brh, int8_t * w_table, uint32_t * p_table, bool taken)
{
    int GHL = sizeof(brh) * 8;
    assert(sizeof(brh) == 4);
  
    int brh_mask = 1;
    bool result;
  
    for (int i = 0; i < GHL; i++)
    {
	result = brh_mask & brh;
	if (result == taken)
	    w_table[hash(pc, p_table[i % (1 << PATH_TABLE_SIZE)], i)]++;
	else
	    w_table[hash(pc, p_table[i % (1 << PATH_TABLE_SIZE)], i)]--;
    
	brh_mask = (brh_mask << 1);
    }
}


void PredictorInit() {
    weight_table = new int8_t[1 << TABLE_SIZE];
    bias_table = new int8_t[1 << BIAS_SIZE];
    path_table_fetch = new uint32_t[1 << PATH_TABLE_SIZE];
    path_table_retire = new uint32_t[1 << PATH_TABLE_SIZE];
    assert(weight_table && path_table_fetch && path_table_retire && bias_table);
}

void PredictorReset() {
    // this function is called before EVERY run
    // it is used to reset predictors and change configurations

    printf("Predictor:perceptron\nconfig: %i weights, %i path histories,  %li KB cost\n",
	   1 << TABLE_SIZE,
	   1 << PATH_TABLE_SIZE,
	   ((1 << TABLE_SIZE) / 1024) + ((1 << PATH_TABLE_SIZE) *sizeof(uint32_t) / 1024) +((1<< BIAS_SIZE) / 1024));
 
    for (int i = 0; i < (1 << TABLE_SIZE); i ++)
	weight_table[i] = 0;

    for (int i = 0; i < (1 << BIAS_SIZE); i++)
	bias_table[i] = 0;

    for (int i = 0; i < (1 << PATH_TABLE_SIZE); i++)
    {
	path_table_fetch[i] = 0;
	path_table_retire[i] = 0;
    }
      

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
	uint32_t pc = uop-> pc;

	if (uop->type & IS_BR_CONDITIONAL) {
	    int output = predict(pc, brh_fetch, bias_table, weight_table, path_table_fetch);
	    bool pred = (output >= 0);
	    assert(report_pred(fe_ptr, false, pred));
	}

	// update fetch branch history
	if (uop->type & IS_BR_CONDITIONAL)
	    brh_fetch = (brh_fetch << 1) | (uop->br_taken ? 1 : 0);
	else if (uop_is_branch(uop->type))
	    brh_fetch = (brh_fetch << 1) | 1;

	if (uop_is_branch(uop->type)) {
	    for(int i = ((1 << PATH_TABLE_SIZE) -1); i > 0; i--)
		path_table_fetch[i] = path_table_fetch[i-1];

	    path_table_fetch[0] = pc;
	}    
    }

    for (int i = 0; i < cycle_info->num_retire; i++) {
	const uint32_t THETA = 70;
	uint32_t rob_ptr = cycle_info->retire_q[i];
	const cbp3_uop_dynamic_t *uop = &rob_entry(rob_ptr)->uop;
	uint32_t pc = uop->pc;

	if (uop->type & IS_BR_CONDITIONAL) {
	    int output = predict(pc,brh_retire,bias_table, weight_table, path_table_retire);
	    bool prediction = (output >= 0);
	    bool taken = uop->br_taken;

	    if ((abs(output) < THETA) || (prediction != taken))
	    {
		if (taken)
		    bias_table[pc % (1 << BIAS_SIZE)]++;
		else
		    bias_table[pc % (1 << BIAS_SIZE)]--;
		// update predictor
		
		updateWeights(pc, brh_retire, weight_table, path_table_retire, taken);
	    }
	}

	// update retire branch history
	if (uop->type & IS_BR_CONDITIONAL)
	    brh_retire = (brh_retire << 1) | (uop->br_taken ? 1 : 0);
	else if (uop_is_branch(uop->type))
	    brh_retire = (brh_retire << 1) | 1;

	if (uop_is_branch(uop->type)) {
	    for(int i = ((1 << PATH_TABLE_SIZE) -1); i > 0; i--)
		path_table_retire[i] = path_table_retire[i-1];
	    
	    path_table_retire[0] = pc;
	}
    }
}

void PredictorRunEnd() {
}

void PredictorExit() {
    delete [] weight_table;
    delete [] bias_table;
    delete [] path_table_fetch;
    delete [] path_table_retire;
}
