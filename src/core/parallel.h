#ifndef PBRT_CORE_PARALLEL_H
#define PBRT_CORE_PARALLEL_H

#include "pbrt.h"

typedef volatile LONG AtomicInt32;
typedef volatile LONGLONG AtomicInt64;

struct MutexLock {
	MutexLock(Mutex &m);
	~MutexLock();
private:
	Mutex &mutex;
	MutexLock(const MutexLock &);
	MutexLock &operator=(const MutexLock &);
};

struct MutexLock;
class Mutex {
public:
	static Mutex *Create();
	static void Destroy(Mutex *m);
private:
	// Mutex Private Methods
	Mutex();
	~Mutex();
	friend struct MutexLock;
	Mutex(Mutex &);
	Mutex &operator=(const Mutex &);

	// System-dependent mutex implementation
	CRITICAL_SECTION criticalSection;

};

void TasksInit();
class Task {
public:
	virtual ~Task();
	virtual void Run() = 0;
};

class ConditionVariable {
public:
	// ConditionVariable Public Methods
	ConditionVariable();
	~ConditionVariable();
	void Lock();
	void Unlock();
	void Wait();
	void Signal();
private:
	// ConditionVariable Private Data

	// Count of the number of waiters.
	uint32_t waitersCount;
	// Serialize access to <waitersCount>.
	CRITICAL_SECTION waitersCountMutex, conditionMutex;
	// Signal and broadcast event HANDLEs.
	enum { SIGNAL = 0, BROADCAST = 1, NUM_EVENTS = 2 };
	HANDLE events[NUM_EVENTS];
};

class Semaphore {
public:
	// Semaphore Public Methods
	Semaphore();
	~Semaphore();
	void Post(int count = 1);
	void Wait();
	bool TryWait();
private:
	// Semaphore Private Data
	HANDLE handle;

};

void EnqueueTasks(const vector<Task *> &tasks);
void WaitForAllTasks();
int NumSystemCores();

//inline int64_t AtomicAdd(AtomicInt64 *v, int64_t delta) {
//	return InterlockedAdd64(v, delta);
//}

inline int32_t AtomicAdd(AtomicInt32 *v, int32_t delta){
	return InterlockedAdd(v, delta);
}

#endif