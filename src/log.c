

#include <stdio.h>
#include <unistd.h>


#include "log.h"


static FILE* logfile;
static int log_sync;


void initLog(int sync) {
	
	// need to convert to bare posix calls, if this code ever gets used one day
	//logfile = fopen("_eacsmb.log", "w");
	
	log_sync = sync;
}



void Log_internal(char* msg, const char* file, int line, const char* function) {
	
	fprintf(logfile, "%s [%s:%d (%s)]\n", msg, file, line, function);
	
	if(log_sync) {
	//	fsync(logfile);
	}
}


void Log_flush() {
// 	fsync(logfile);
}








