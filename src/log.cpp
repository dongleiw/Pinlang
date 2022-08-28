#include "log.h"

FILE *g_log_file=nullptr;
void init_log(std::string filepath){
	g_log_file = fopen(filepath.c_str(), "w");
	if(g_log_file==nullptr){
		perror("failed to open file:");
		abort();
	}
}

std::string timenow() {
	char buffer[64];
	time_t curtime;
	struct tm* timeinfo;

	time(&curtime);
	timeinfo = localtime(&curtime);

	strftime(buffer, 64, "%Y-%m-%d %H:%M:%S", timeinfo);

	return std::string(buffer);
}
