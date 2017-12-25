#ifndef __EACSMB_log_h__
#define __EACSMB_log_h__


#define Log(msg) Log_internal(msg, __FILE__, __LINE__, __func__)
#define Logf(msg) Log_internal(msg, __FILE__, __LINE__, __func__)

void Log_internal(char* msg, const char* file, int line, const char* function);
void Log_flush();





#endif // __EACSMB_log_h__
