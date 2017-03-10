#ifndef PBRT_CORE_PROGRESSREPORTER_H
#define PBRT_CORE_PROGRESSREPORTER_H


// core/progressreporter.h*
#include "pbrt.h"
#include "timer.h"
#include "parallel.h"

// ProgressReporter Declarations
class ProgressReporter {
public:
	// ProgressReporter Public Methods
	ProgressReporter(int totalWork, const string &title,
		int barLength = -1);
	~ProgressReporter();
	void Update(int num = 1);
	void Done();
private:
	// ProgressReporter Private Data
	const int totalWork;
	int workDone, plussesPrinted, totalPlusses;
	Timer *timer;
	FILE *outFile;
	char *buf, *curSpace;
	Mutex *mutex;
};

// core/progressreporter.h*
#include "pbrt.h"
extern int TerminalWidth();

#endif // PBRT_CORE_PROGRESSREPORTER_H
