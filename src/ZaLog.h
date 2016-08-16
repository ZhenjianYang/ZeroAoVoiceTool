#ifndef __ZALOG_H__
#define __ZALOG_H__

#define ZALOG_OUT_FILE 0x01
#define ZALOG_OUT_STDLOG 0x02
#define ZALOG_OUT_STDOUT 0x04
#define ZALOG_OUT_ALL (ZALOG_OUT_FILE | ZALOG_OUT_STDLOG | ZALOG_OUT_STDOUT)
#define ZALOG_OUT_DFT (ZALOG_OUT_FILE | ZALOG_OUT_STDLOG)

#define ZALOG_TYPE_INFO 0x10
#define ZALOG_TYPE_DEBUG 0x20
#define ZALOG_TYPE_ERROR 0x40
#define ZALOG_TYPE_ALL (ZALOG_TYPE_INFO | ZALOG_TYPE_DEBUG | ZALOG_TYPE_ERROR)
#define ZALOG_TYPE_DFT (ZALOG_TYPE_INFO | ZALOG_TYPE_ERROR)
#define ZALOG_TYPE_RUNMODE (ZALOG_TYPE_INFO | ZALOG_TYPE_ERROR)
#define ZALOG_TYPE_DEBUGMODE ZALOG_TYPE_ALL

#define ZALOG_PARAM_ALL (ZALOG_OUT_ALL | ZALOG_TYPE_ALL)
#define ZALOG_PARAM_DFT (ZALOG_OUT_DFT | ZALOG_TYPE_DFT)
#define ZALOG_PARAM_RUNMODE (ZALOG_OUT_DFT | ZALOG_TYPE_RUNMODE)
#define ZALOG_PARAM_DEBUGMODE (ZALOG_OUT_DFT | ZALOG_TYPE_DEBUGMODE)

#define ZALOG_PARAM_NOPREINFO 0x1000

#define ZALOG_STRMARK_INFO "INF"
#define ZALOG_STRMARK_DEBUG "DBG"
#define ZALOG_STRMARK_ERROR "ERR"

#define ZALOG_LOGFILE_DFT "log.txt"

#if !ZALOG_NOLOG

#define ZALOG_SETLOGFILE(filename) _zalog_setlogfile(filename)
#define ZALOG_SETPARAM(param) _zalog_setparam(param)
#define ZALOG_ADDPARAM(param) _zalog_addparam(param)
#define ZALOG_DELPARAM(param) _zalog_delparam(param)
#define ZALOG_OPEN _zalog_open(ZALOG_PARAM_DFT)
#define ZALOG_OPEN_WITHPARAM(param) _zalog_open(param)
#define ZALOG_CLOSE _zalog_close()

#define ZALOG_INFO(format, ...)  _zalog_print(ZALOG_TYPE_INFO, format , __VA_ARGS__)
#define ZALOG_DEBUG(format, ...) _zalog_print(ZALOG_TYPE_DEBUG, format, __VA_ARGS__)
#define ZALOG_ERROR(format, ...) _zalog_print(ZALOG_TYPE_ERROR, format, __VA_ARGS__)
#define ZALOG(format, ...) ZALOG_INFO(format, __VA_ARGS__)

#define ZALOG_EMPTY_LINE _zalog_empty_line()

void _zalog_addparam(int param);
void _zalog_delparam(int param);
void _zalog_setparam(int param);
void _zalog_setlogfile(const char* filename);
void _zalog_open(int param);
void _zalog_close();

void _zalog_print(int type, const char* format, ...);

void _zalog_empty_line();

#else 

#define ZALOG_SETLOGFILE(filename)
#define ZALOG_SETPARAM(param)
#define ZALOG_OPEN
#define ZALOG_OPEN_WITHPARAM(param)
#define ZALOG_CLOSE

#define ZALOG(format, ...)
#define ZALOG_INFO(format, ...) 
#define ZALOG_DEBUG(format, ...)
#define ZALOG_ERROR(format, ...)

#define ZALOG_EMPTY_LINE
	
#endif //ZALOG_NOLOG

#endif //__ZALOG_H__
