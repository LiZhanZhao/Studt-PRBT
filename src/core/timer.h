#ifndef PBRT_CORE_TIMER_H
#define PBRT_CORE_TIMER_H

// core/timer.h*
#include "pbrt.h"

#include <windows.h>
#if (_MSC_VER >= 1400)
#include <stdio.h>
#define snprintf _snprintf
#endif


// Timer Declarations
class Timer {
public:
    // Public Timer Methods
    Timer();
    
    void Start();
    void Stop();
    void Reset();
    
    double Time();
private:
    // Private Timer Data
    double time0, elapsed;
    bool running;
    double GetTime();
    // Private Windows Timer Data
    LARGE_INTEGER performance_counter, performance_frequency;
    double one_over_frequency;

};



#endif // PBRT_CORE_TIMER_H
