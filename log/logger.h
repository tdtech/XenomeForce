#pragma once

#define ENABLE_LOG // Uncomment this to enable log

#ifdef ENABLE_LOG
#define LOGTEXT(a)           CLogger::instance().verbose(a)
#define LOGTEXT2(a, b)       CLogger::instance().verbose(a, b)
#define LOGTEXT3(a, b, c)    CLogger::instance().verbose(a, b, c)
#define LOGTEXT4(a, b, c, d) CLogger::instance().verbose(a, b, c, d)

class CLogger
{
public:
	static CLogger& instance() { return mLogger; }

	void verbose(const char* text, ...);

private:
	CLogger();
	~CLogger();

	static int lockMutex();
	static int unlockMutex();

private:
	static CLogger mLogger;
};

#else
#define LOGTEXT(a)
#define LOGTEXT2(a, b)
#define LOGTEXT3(a, b, c)
#define LOGTEXT4(a, b, c, d)
#endif