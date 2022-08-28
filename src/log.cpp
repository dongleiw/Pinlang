#include "log.h"

FILE *g_log_file=nullptr;
void init_log(std::string filepath){
	g_log_file = fopen(filepath.c_str(), "w");
	if(g_log_file==nullptr){
		perror("failed to open file:");
		abort();
	}
}
