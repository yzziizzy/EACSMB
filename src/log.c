

#include "stdio.h"


#include "log.h"


static FILE* logfile;
static int log_sync;


void initLog(int sync) {
	
	logfile = fopen("_eacsmb.log", "w");
	
	log_sync = sync;
}



void Log_internal(char* msg, const char* file, int line, const char* function) {
	
	fprintf(logfile, "%s [%s:%d (%s)]\n", msg, file, line, function);
	
	if(log_sync) {
		fsync(logfile);
	}
}











