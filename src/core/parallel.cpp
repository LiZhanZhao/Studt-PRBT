#include "parallel.h"
#include "memory.h"

static HANDLE *threads;
static Mutex *taskQueueMutex = Mutex::Create();
static std::vector<Task *> taskQueue;
static ConditionVariable *tasksRunningCondition;
static uint32_t numUnfinishedTasks;
static Semaphore *workerSemaphore;
DWORD WINAPI taskEntry(LPVOID arg);

Options PbrtOptions;

void EnqueueTasks(const vector<Task *> &tasks) {
	if (PbrtOptions.nCores == 1) {
		for (unsigned int i = 0; i < tasks.size(); ++i)
			tasks[i]->Run();
		return;
	}

	if (!threads)
		TasksInit();

	{ MutexLock lock(*taskQueueMutex);
	for (unsigned int i = 0; i < tasks.size(); ++i)
		taskQueue.push_back(tasks[i]);
	}
	tasksRunningCondition->Lock();
	numUnfinishedTasks += tasks.size();
	tasksRunningCondition->Unlock();

	workerSemaphore->Post(tasks.size());
}

void WaitForAllTasks() {
	if (PbrtOptions.nCores == 1)
		return; // enqueue just runs them immediately in this case
#ifdef PBRT_USE_GRAND_CENTRAL_DISPATCH
	dispatch_group_wait(gcdGroup, DISPATCH_TIME_FOREVER);
#else
	if (!tasksRunningCondition)
		return;  // no tasks have been enqueued, so TasksInit() never called
	tasksRunningCondition->Lock();
	while (numUnfinishedTasks > 0)
		tasksRunningCondition->Wait();
	tasksRunningCondition->Unlock();
#endif
}

void TasksInit() {
	if (PbrtOptions.nCores == 1)
		return;
	static const int nThreads = NumSystemCores();
	workerSemaphore = new Semaphore;
	tasksRunningCondition = new ConditionVariable;

	threads = new HANDLE[nThreads];
	for (int i = 0; i < nThreads; ++i) {
		threads[i] = CreateThread(NULL, 0, taskEntry, reinterpret_cast<void *>(i), 0, NULL);
		if (threads[i] == NULL)
			Severe("Error from CreateThread");
	}


}

int NumSystemCores() {
	if (PbrtOptions.nCores > 0) return PbrtOptions.nCores;
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;

}



static DWORD WINAPI taskEntry(LPVOID arg) {

	while (true) {
		workerSemaphore->Wait();
		// Try to get task from task queue
		Task *myTask = NULL;
		{ MutexLock lock(*taskQueueMutex);
		if (taskQueue.size() == 0)
			break;
		myTask = taskQueue.back();
		taskQueue.pop_back();
		}

		// Do work for _myTask_
		
		myTask->Run();
		
		tasksRunningCondition->Lock();
		int unfinished = --numUnfinishedTasks;
		if (unfinished == 0)
			tasksRunningCondition->Signal();
		tasksRunningCondition->Unlock();
	}
	
	return 0;
}

// http://www.cs.wustl.edu/\~schmidt/win32-cv-1.html

ConditionVariable::ConditionVariable() {
	waitersCount = 0;
	InitializeCriticalSection(&waitersCountMutex);
	InitializeCriticalSection(&conditionMutex);

	events[SIGNAL] = CreateEvent(NULL,  // no security
		FALSE, // auto-reset event
		FALSE, // non-signaled initially
		NULL); // unnamed
	events[BROADCAST] = CreateEvent(NULL,  // no security
		TRUE,  // manual-reset
		FALSE, // non-signaled initially
		NULL); // unnamed

}


ConditionVariable::~ConditionVariable() {
	CloseHandle(events[SIGNAL]);
	CloseHandle(events[BROADCAST]);
}


void ConditionVariable::Lock() {
	EnterCriticalSection(&conditionMutex);
}


void ConditionVariable::Unlock() {
	LeaveCriticalSection(&conditionMutex);
}


void ConditionVariable::Wait() {
	// Avoid race conditions.
	EnterCriticalSection(&waitersCountMutex);
	waitersCount++;
	LeaveCriticalSection(&waitersCountMutex);

	// It's ok to release the <external_mutex> here since Win32
	// manual-reset events maintain state when used with
	// <SetEvent>.  This avoids the "lost wakeup" bug...
	LeaveCriticalSection(&conditionMutex);

	// Wait for either event to become signaled due to <pthread_cond_signal>
	// being called or <pthread_cond_broadcast> being called.
	int result = WaitForMultipleObjects(2, events, FALSE, INFINITE);

	EnterCriticalSection(&waitersCountMutex);
	waitersCount--;
	int last_waiter = (result == WAIT_OBJECT_0 + BROADCAST) &&
		(waitersCount == 0);
	LeaveCriticalSection(&waitersCountMutex);

	// Some thread called <pthread_cond_broadcast>.
	if (last_waiter)
		// We're the last waiter to be notified or to stop waiting, so
		// reset the manual event.
		ResetEvent(events[BROADCAST]);

	EnterCriticalSection(&conditionMutex);
}



void ConditionVariable::Signal() {
	EnterCriticalSection(&waitersCountMutex);
	int haveWaiters = (waitersCount > 0);
	LeaveCriticalSection(&waitersCountMutex);

	if (haveWaiters)
		SetEvent(events[SIGNAL]);
}


MutexLock::MutexLock(Mutex &m) : mutex(m) {
	EnterCriticalSection(&mutex.criticalSection);
}



MutexLock::~MutexLock() {
	LeaveCriticalSection(&mutex.criticalSection);
}

Mutex *Mutex::Create() {
	return new Mutex;
}



void Mutex::Destroy(Mutex *m) {
	delete m;
}



Mutex::Mutex() {
	InitializeCriticalSection(&criticalSection);
}



Mutex::~Mutex() {
	DeleteCriticalSection(&criticalSection);
}

Semaphore::Semaphore() {
	handle = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
	if (!handle)
		Severe("Error from CreateSemaphore: %d", GetLastError());
}

Semaphore::~Semaphore() {
	CloseHandle(handle);
}

void Semaphore::Wait() {
	if (WaitForSingleObject(handle, INFINITE) == WAIT_FAILED)
		Severe("Error from WaitForSingleObject: %d", GetLastError());

}

bool Semaphore::TryWait() {
	return (WaitForSingleObject(handle, 0L) == WAIT_OBJECT_0);
}

void Semaphore::Post(int count) {
	if (!ReleaseSemaphore(handle, count, NULL))
		Severe("Error from ReleaseSemaphore: %d", GetLastError());
}

Task::~Task() {
}