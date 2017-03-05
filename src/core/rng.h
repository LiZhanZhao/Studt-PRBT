
#ifndef PBRT_CORE_RNG_H
#define PBRT_CORE_RNG_H

// core/rng.h*
#include "pbrt.h"


// Random Number Declarations
class RNG {
public:
    RNG(uint32_t seed = 5489UL) {
        mti = N+1; /* mti==N+1 means mt[N] is not initialized */
        Seed(seed);
    }

    void Seed(uint32_t seed) const;
    float RandomFloat() const;
    uint32_t RandomUInt() const;

private:
    static const int N = 624;
    mutable unsigned long mt[N]; /* the array for the state vector  */
    mutable int mti;
};



#endif // PBRT_CORE_RNG_H
