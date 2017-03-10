
#include "progressreporter.h"

int TerminalWidth() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    if (h == INVALID_HANDLE_VALUE || h == NULL) {
        fprintf(stderr, "GetStdHandle() call failed");
        return 80;
    }
    CONSOLE_SCREEN_BUFFER_INFO bufferInfo = { 0 };
    GetConsoleScreenBufferInfo(h, &bufferInfo);
    return bufferInfo.dwSize.X;

}


// ProgressReporter Method Definitions
ProgressReporter::ProgressReporter(int tw, const string &title, int barLength)
: totalWork(tw) {
	if (barLength <= 0)
		barLength = TerminalWidth() - 28;
	totalPlusses = max(2, barLength - (int)title.size());
	mutex = Mutex::Create();
	plussesPrinted = 0;
	workDone = 0;
	timer = new Timer;
	timer->Start();
	outFile = stdout;
	// Initialize progress string
	const int bufLen = title.size() + totalPlusses + 64;
	buf = new char[bufLen];
	snprintf(buf, bufLen, "\r%s: [", title.c_str());
	curSpace = buf + strlen(buf);
	char *s = curSpace;
	for (int i = 0; i < totalPlusses; ++i)
		*s++ = ' ';
	*s++ = ']';
	*s++ = ' ';
	*s++ = '\0';
	if (!PbrtOptions.quiet) {
		fputs(buf, outFile);
		fflush(outFile);
	}
}


ProgressReporter::~ProgressReporter() {
	delete[] buf;
	delete timer;
	Mutex::Destroy(mutex);
}


void ProgressReporter::Update(int num) {
	if (num == 0 || PbrtOptions.quiet) return;
	MutexLock lock(*mutex);
	workDone += num;
	float percentDone = float(workDone) / float(totalWork);
	int plussesNeeded = Round2Int(totalPlusses * percentDone);
	if (plussesNeeded > totalPlusses) plussesNeeded = totalPlusses;
	while (plussesPrinted < plussesNeeded) {
		*curSpace++ = '+';
		++plussesPrinted;
	}
	fputs(buf, outFile);
	// Update elapsed time and estimated time to completion
	float seconds = (float)timer->Time();
	float estRemaining = seconds / percentDone - seconds;
	if (percentDone == 1.f)
		fprintf(outFile, " (%.1fs)       ", seconds);
	else
		fprintf(outFile, " (%.1fs|%.1fs)  ", seconds, max(0.f, estRemaining));
	fflush(outFile);
}


void ProgressReporter::Done() {
	if (PbrtOptions.quiet) return;
	MutexLock lock(*mutex);
	while (plussesPrinted++ < totalPlusses)
		*curSpace++ = '+';
	fputs(buf, outFile);
	float seconds = (float)timer->Time();
	fprintf(outFile, " (%.1fs)       \n", seconds);
	fflush(outFile);
}
