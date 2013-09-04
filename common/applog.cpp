#include "applog.h"
#include <cstdarg>
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

namespace applog
{
	Flush flush;
	Endl endl;

	Logger Logger::_syslog("sys", applog::INFO);

#define DEFINE_STREAM_OUT(type) \
	LogStream& LogStream::operator << (type data)\
	{\
	if (level == applog::TERM)\
	{\
	std::cerr << data;\
	}\
	else\
	{\
	buffer << data;\
	}\
	return *this;\
	}

	DEFINE_STREAM_OUT(char);
	DEFINE_STREAM_OUT(short);
	DEFINE_STREAM_OUT(int);
	DEFINE_STREAM_OUT(unsigned char);
	DEFINE_STREAM_OUT(unsigned short);
	DEFINE_STREAM_OUT(unsigned int);
	DEFINE_STREAM_OUT(float);
	DEFINE_STREAM_OUT(double);
	DEFINE_STREAM_OUT(char *);
	DEFINE_STREAM_OUT(const std::string &);

	LogStream& LogStream::operator << (CharClr clr)
	{
#ifdef _WIN32
		if (level == applog::TERM)
		{
			static short _clrmap[] = {
				FOREGROUND_RED,
				FOREGROUND_GREEN,
				FOREGROUND_BLUE,
				FOREGROUND_RED | FOREGROUND_GREEN,
				FOREGROUND_RED | FOREGROUND_BLUE,
				FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
				FOREGROUND_INTENSITY,
				0
			};
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), _clrmap[clr]);
		}
#endif // _WIN32
		return *this;
	}

	LogStream& LogStream::operator << (Endl)
	{
		*this << "\n";
		return *this;
	}

	LogStream& LogStream::operator << (Flush)
	{
		if (logger.loglvl >= level && level != applog::TERM)
		{
			int len = (int)buffer.gcount();
			if (len > 0)
			{
				logger.log(level, buffer.str().c_str(), len);
				buffer.clear();
			}
		}
		return *this;
	}

	void Logger::set_buffer(const std::string &file)
	{
		fp = fopen(file.c_str(), "w");
		if (!fp) fp = stderr;
	}

	int Logger::log(LogLvl level, const char *text, int len /* = -1 */)
	{
		static const char *_lvlstr[] = {
			"verbose",
			"debug",
			"info",
			"warning",
			"error",
			"fatal"
		};

		if (len == -1)
		{
			len = strlen(text);
		}
		if (level == applog::TERM)
		{
			//return (int)fprintf(stdout, "%s", text);
#ifdef WIN32
			DWORD nWritten = 0;
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			WriteConsole(hOut, text, len, &nWritten, NULL);
			return (int)nWritten;
#endif
		}
		else if (fp)
		{
			return (int)fprintf(fp, "%s:%s: %s", logname.c_str(), _lvlstr[level], text);
		}
		return 0;
	}

	// not thread safe
	static char _shared_buffer[1<<20];

	int Logger::logf(LogLvl level, const char *format, ...)
	{
		if (level >= loglvl)
		{
			va_list args;
			va_start(args, format);
			int len = vsprintf(_shared_buffer, format, args);
			va_end(args);
			return log(level, _shared_buffer, len);
		}
		return 0;
	}

#define DEFINE_FORMAT_OUT(LVL) \
	int Logger::LVL(const char *format, ...)\
	{\
	if (applog::LVL >= loglvl)\
	{\
	va_list args;\
	va_start(args, format);\
	int len = vsprintf(_shared_buffer, format, args);\
	va_end(args);\
	return log(applog::LVL, _shared_buffer, len);\
	}\
	return 0;\
	}

	DEFINE_FORMAT_OUT(VERB);
	DEFINE_FORMAT_OUT(DEBVG);
	DEFINE_FORMAT_OUT(INFO);
	DEFINE_FORMAT_OUT(WARN);
	DEFINE_FORMAT_OUT(ERR);
	DEFINE_FORMAT_OUT(FATAL);
	DEFINE_FORMAT_OUT(TERM);
}
