
#include "pbrt.h"
#include "progressreporter.h"

// Error Reporting Definitions
#define PBRT_ERROR_IGNORE 0
#define PBRT_ERROR_CONTINUE 1
#define PBRT_ERROR_ABORT 2

const char *findWordEnd(const char *buf) {
    while (*buf != '\0' && !isspace(*buf))
        ++buf;
    return buf;
}

// Error Reporting Functions
static void processError(const char *format, va_list args,
        const char *errorType, int disposition) {
    // Report error
    if (disposition == PBRT_ERROR_IGNORE)
        return;

    // Build up an entire formatted error string and print it all at once;
    // this way, if multiple threads are printing messages at once, they
    // don't get jumbled up...
    std::string errorString;

	/*
    // Print line and position in input file, if available
    extern int line_num;
    if (line_num != 0) {
        extern string current_file;
        errorString += current_file;
        char buf[16];
        sprintf(buf, "(%d): ", line_num);
        errorString += buf;
    }
	*/

    // PBRT_ERROR_CONTINUE, PBRT_ERROR_ABORT
    // Print formatted error message
    int width = max(20, TerminalWidth() - 2);
    errorString += errorType;
    errorString += ": ";
    int column = errorString.size();

    char errorBuf[2048];
    vsnprintf_s(errorBuf, sizeof(errorBuf), _TRUNCATE, format, args);

    const char *msgPos = errorBuf;
    while (true) {
        while (*msgPos != '\0' && isspace(*msgPos))
            ++msgPos;
        if (*msgPos == '\0')
            break;

        const char *wordEnd = findWordEnd(msgPos);
        if (column + wordEnd - msgPos > width) {
            errorString += "\n    ";
            column = 4;
        }
        while (msgPos != wordEnd) {
            errorString += *msgPos++;
            ++column;
        }
        errorString += ' ';
        ++column;
    }

    fprintf(stderr, "%s\n", errorString.c_str());

    if (disposition == PBRT_ERROR_ABORT) {
        __debugbreak();

    }
}


void Info(const char *format, ...) {
    //if (!PbrtOptions.verbose || PbrtOptions.quiet) return;
    va_list args;
    va_start(args, format);
    processError(format, args, "Notice", PBRT_ERROR_CONTINUE);
    va_end(args);
}


void Warning(const char *format, ...) {
    //if (PbrtOptions.quiet) return;
    va_list args;
    va_start(args, format);
    processError(format, args, "Warning", PBRT_ERROR_CONTINUE);
    va_end(args);
}


void Error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    processError(format, args, "Error", PBRT_ERROR_CONTINUE);
    va_end(args);
}


void Severe(const char *format, ...) {
    va_list args;
    va_start(args, format);
    processError(format, args, "Fatal Error", PBRT_ERROR_ABORT);
    va_end(args);
}


