#ifndef CBP3_READER_H_INCLUDED
#define CBP3_READER_H_INCLUDED

// ************** The following code is used to interface trace reader **************
// **************        DO NOT use them in your predictor code        **************

// cycle activity and uops read from reader
class cbp3_cycle_info_t {
    public:
        cbp3_cycle_info_t (){ reset(); }
        void reset();

        cbp3_uop_dynamic_t uopinfo[PIPE_WIDTH];
        uint8_t  fetch_q[PIPE_WIDTH], allocate_q[PIPE_WIDTH], exe_q[15];
        uint8_t  retire_q[PIPE_WIDTH], agu_q[2], std_q[8];
        uint32_t cycle;
};

// reader interface
void ReaderLoadTrace(FILE *cbp3_raw_trace_fp, bool load, bool debug, int value_set, int mem_set);
bool ReaderRunACycle(uint16_t *num_stage);
void ReaderRewind();
uint32_t ReaderMaxMem();
uint32_t ReaderTraceInfoSize();
const cbp3_cycle_info_t * ReaderInfo();
void ReaderEnd();
bool ReaderTraceCheck();

#endif
