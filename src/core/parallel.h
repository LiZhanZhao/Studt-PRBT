#ifndef PBRT_CORE_PARALLEL_H
#define PBRT_CORE_PARALLEL_H

#include "pbrt.h"

typedef volatile LONG AtomicInt32;
typedef volatile LONGLONG AtomicInt64;


inline int64_t AtomicAdd(AtomicInt64 *v, int64_t delta) {
	return InterlockedAdd64(v, delta);
}

#endif