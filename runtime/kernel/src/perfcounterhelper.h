#ifndef __PERFCOUNTERHELPER_H__
#define __PERFCOUNTERHELPER_H__

//
//This file contains macros that make using the performance counters
//look a little nicer.
//

#ifndef NO_PROFILE
    #define PROFILE_start_define(group, name) uint32 __perf_var##group##_##name##__ = AddandStartProfileCounter(MAKEFOURCC_PC((#group)[0],(#group)[1],(#group)[2],(#group)[3]), #name);
    #define PROFILE_start(group, name) StartProfileCounter(__perf_var##group##_##name##__);
    #define PROFILE_stop(group, name) StopProfileCounter(__perf_var##group##_##name##__);
#else
    #define PROFILE_start_define(group, name)
    #define PROFILE_start(group, name)
    #define PROFILE_stop(group, name)
#endif


#endif