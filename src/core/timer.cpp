
// core/timer.cpp*
#include "timer.h"

// Timer Method Definitions
Timer::Timer()
{
    // Windows Timer Initialization
    QueryPerformanceFrequency( &performance_frequency );
    one_over_frequency = 1.0/((double)performance_frequency.QuadPart);

    time0 = elapsed = 0;
    running = 0;
}




double Timer::GetTime()
{
    // Windows GetTime
    QueryPerformanceCounter( &performance_counter );
    return (double) performance_counter.QuadPart * one_over_frequency;

}



void Timer::Start()
{
    Assert( !running );
    running = 1;
    time0 = GetTime();
}



void Timer::Stop()
{
    Assert( running );
    running = 0;

    elapsed += GetTime() - time0;
}



void Timer::Reset()
{
    running = 0;
    elapsed = 0;
}



double Timer::Time()
{
    if (running) {
        Stop();
        Start();
    }
    return elapsed;
}


