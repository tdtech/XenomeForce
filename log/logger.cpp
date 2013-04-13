#include "logger.h"

#ifdef ENABLE_LOG

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>

#define LOG_FILE "logs\\main.txt"

FILE*  sLogFile = NULL;
HANDLE sMutex   = NULL;

CLogger CLogger::mLogger;

void CLogger::verbose(const char* text, ...)
{
	if (!lockMutex()) return;
	if (sLogFile)
	{
		va_list args;

		fprintf(sLogFile, "\n");

		va_start(args, text);
		vfprintf(sLogFile, text, args);
		va_end(args);

		fflush(sLogFile);
	}
	unlockMutex();
}

CLogger::CLogger()
{
	sLogFile = fopen(LOG_FILE, "a");
	sMutex = CreateMutex(NULL, FALSE, NULL);
	if (sLogFile)
	{
		SYSTEMTIME time;
		GetLocalTime(&time);
		fprintf(sLogFile, "\n--- Logging started at %d/%d/%d - %d:%d:%d ---",
                time.wDay, time.wMonth, time.wYear, time.wHour, time.wMinute, time.wSecond);
		fflush(sLogFile);
	}
}

CLogger::~CLogger()
{
	if (sLogFile) fclose(sLogFile);
	sLogFile = NULL;
	CloseHandle(sMutex);
	sMutex = NULL;
}

int CLogger::lockMutex()
{
	return (WaitForSingleObject(sMutex, INFINITE) != WAIT_ABANDONED);
}

int CLogger::unlockMutex()
{
	return (SUCCEEDED(ReleaseMutex(sMutex)));
}

#endif