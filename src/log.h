#pragma once

#include <ctime>
#include <stdio.h>
#include <string>
#include <string.h>

#define _stringfy(x) #x
#define stringfy(x) _stringfy(x)

static std::string timenow() {
	char buffer[64];
	time_t curtime;
	struct tm* timeinfo;

	time(&curtime);
	timeinfo = localtime(&curtime);

	strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", timeinfo);

	return std::string(buffer);
}

#define SHORT_FILE strrchr(__FILE__, '/') ?  strrchr(__FILE__, '/') + 1 : __FILE__ 
#define _log(loglevel, fmts, ...) printf("[" loglevel "] [%s"                                     \
										 "] [%s" ":" stringfy(__LINE__) ":%s] " fmts "\n", \
										 timenow().c_str(), SHORT_FILE, __FUNCTION__, ##__VA_ARGS__)

#define log_info(fmts, ...) _log("INFO", fmts, ##__VA_ARGS__)
#define log_debug(fmts, ...) _log("DEBUG", fmts, ##__VA_ARGS__)
#define log_error(fmts, ...) _log("ERROR", fmts, ##__VA_ARGS__)

#define panicf(fmts, ...)                   \
	do {                                    \
		_log("FATAL", fmts, ##__VA_ARGS__); \
		abort(); \
	} while (0);
