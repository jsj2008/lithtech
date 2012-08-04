
// LT timers use the microsecond counters and do time adjusting to make the timers
// stay at the desired update rate as much as possible.

#ifndef __TIMER_H__
#define __TIMER_H__

#ifndef __SYSCOUNTER_H__
#include "syscounter.h"
#endif


class LTTimer {
public:

    LTTimer();
    
    void SetUpdateRate(float rate);
    float GetUpdateRate();

    // Returns TRUE if the timer has 'fired'.
    LTBOOL Update();

protected:

    CounterFinal m_Counter;
    float m_UpdateRate;
    float m_Adjust;
};


#endif


