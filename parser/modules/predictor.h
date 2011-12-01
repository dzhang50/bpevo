#ifndef PREDICTOR_HEADERFILE_H
#define PREDICTOR_HEADERFILE_H

// Global variables

extern cbp3_cycle_activity_t *cycle_info;
extern int numFetch;
extern int numRetire;
extern uint32_t fe_ptr;
extern uint32_t retire_ptr;
extern cbp3_uop_dynamic_t *fe_uop;
extern cbp3_uop_dynamic_t *retire_uop;


// Global functions

int max(int a, int b) {
  if(a > b) {
    return a;
  }
  else {
    return b;
  }
}

void updateSpecialState();

#endif
