#pragma once

#include <ctime>
#include <stdio.h>
#include <string>
#include <string.h>

#define _stringfy(x) #x
#define stringfy(x) _stringfy(x)

extern std::string timenow();
extern void init_log(std::string filepath);
extern FILE *g_log_file;

#define SHORT_FILE strrchr(__FILE__, '/') ?  strrchr(__FILE__, '/') + 1 : __FILE__ 
#define _log(loglevel, fmts, ...) fprintf(g_log_file, "[" loglevel "] [%s"                                     \
										 "] [%s" ":" stringfy(__LINE__) ":%s] " fmts "\n", \
										 timenow().c_str(), SHORT_FILE, __FUNCTION__, ##__VA_ARGS__)

#define log_info(fmts, ...) _log("INFO ", fmts, ##__VA_ARGS__)
#define log_debug(fmts, ...) _log("DEBUG", fmts, ##__VA_ARGS__)
#define log_error(fmts, ...) _log("ERROR", fmts, ##__VA_ARGS__)

#define panicf(fmts, ...)                   \
	do {                                    \
		_log("FATAL", fmts, ##__VA_ARGS__); \
		fflush(g_log_file); \
		abort(); \
	} while (0);
