#ifndef __applog_h_
#define __applog_h_

#include <string>
#include <sstream>
#include <cstdio>

#pragma warning(disable:4355)

namespace applog
{
	enum LogLvl
	{
		VERB,
		DEBVG,
		INFO,
		WARN,
		ERR,
		FATAL,
		TERM
	};

	enum CharClr
	{
		RED,
		GREEN,
		BLUE,
		YELLOW,
		PINK,
		WHITE,
		GREY,
		BLACK,
	};

	class Logger;

	class Flush {};
	class Endl {};

	extern Flush flush;
	extern Endl endl;

	class LogStream
	{
	public:
		LogStream(Logger &logger, LogLvl level) : logger(logger), level(level) {}
		Logger &logger;
		std::stringstream buffer;

#define DECLARE_STREAM_OUT(type) \
	LogStream& operator << (type data);

		DECLARE_STREAM_OUT(char);
		DECLARE_STREAM_OUT(short);
		DECLARE_STREAM_OUT(int);
		DECLARE_STREAM_OUT(unsigned char);
		DECLARE_STREAM_OUT(unsigned short);
		DECLARE_STREAM_OUT(unsigned int);
		DECLARE_STREAM_OUT(float);
		DECLARE_STREAM_OUT(double);
		DECLARE_STREAM_OUT(char *);
		DECLARE_STREAM_OUT(const std::string &);
		DECLARE_STREAM_OUT(CharClr);
		DECLARE_STREAM_OUT(Endl);
		DECLARE_STREAM_OUT(Flush);

		LogLvl level;
	};

	class Logger
	{
	public:
		Logger(const std::string &name, LogLvl level) : logname(name), loglvl(level), fp(0),
			verbose(*this, applog::VERB),
			debug(*this, applog::DEBVG),
			info(*this, applog::INFO),
			warning(*this, applog::WARN),
			error(*this, applog::ERR),
			fatal(*this, applog::FATAL),
			term(*this, applog::TERM)
		{
			fp = stdout;
		}

		FILE *fp;

		std::string logname;
		LogLvl loglvl;

		LogStream verbose;
		LogStream debug;
		LogStream info;
		LogStream warning;
		LogStream error;
		LogStream fatal;
		LogStream term;

		void set_level(LogLvl level);
		void set_buffer(const std::string &file);
		int log(LogLvl level, const char *text, int len = -1);
		int logf(LogLvl level, const char *format, ...);

#define DECLARE_FORMAT_OUT(LVL) \
	int LVL(const char *foramt, ...)

		DECLARE_FORMAT_OUT(VERB);
		DECLARE_FORMAT_OUT(DEBVG);
		DECLARE_FORMAT_OUT(INFO);
		DECLARE_FORMAT_OUT(WARN);
		DECLARE_FORMAT_OUT(ERR);
		DECLARE_FORMAT_OUT(FATAL);
		DECLARE_FORMAT_OUT(TERM);

		static Logger _syslog;
	};
};

#define syslog applog::Logger::_syslog
#define sysout applog::Logger::_syslog.TERM
#define sysclr(c) applog::Logger::_syslog.term << applog::c

#endif  //__applog_h_
