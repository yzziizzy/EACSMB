#ifndef __EACSMB_log_h__
#define __EACSMB_log_h__


#define Log(msg) Log_internal(msg, __FILE__, __LINE__, __function__)

void Log_internal(char* msg, const char* file, int line, const char* function);





#endif // __EACSMB_log_h__
