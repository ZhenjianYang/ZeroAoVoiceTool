#include "ZALog.h"

#include <time.h>
#include <stdarg.h>
#include <string.h>
#include <iostream>
#include <fstream>

#include <Windows.h>

const char* const _strmark_info = ZALOG_STRMARK_INFO;
const char* const _strmark_debug = ZALOG_STRMARK_DEBUG;
const char* const _strmark_error = ZALOG_STRMARK_ERROR;

static int _logopened = 0;
static int _param;
static std::ofstream _log_ofs;
static std::ostream &_log_stdout = std::cout;
static std::ostream &_log_stdlog = std::clog;

static char filename_buf[1024];

void _zalog_setparam(int _param) {
	if (_logopened) {
		if (!(ZALOG_OUT_FILE & ::_param) && (ZALOG_OUT_FILE & _param)) {
			_log_ofs.open(filename_buf, std::ofstream::out | std::ofstream::app);
		}
		else if ((ZALOG_OUT_FILE & ::_param) && !(ZALOG_OUT_FILE & _param)) {
			_log_ofs.close();
		}
	}

	::_param = _param;
}
void _zalog_addparam(int _param) {
	_zalog_setparam(::_param | _param);
}
void _zalog_delparam(int _param) {
	_zalog_setparam(::_param & (~_param));
}
void _zalog_setlogfile(const char* filename) {
	strcpy_s(filename_buf, filename);
	if (_logopened && (ZALOG_OUT_FILE & _param)) {
		_log_ofs.close();
		_log_ofs.open(filename_buf, std::ofstream::out | std::ofstream::app);
	}
}
void _zalog_open(int _param) {
	_zalog_setparam(_param);
	if (filename_buf[0] == 0) strcpy_s(filename_buf, ZALOG_LOGFILE_DFT);
	if (ZALOG_OUT_FILE & _param) _log_ofs.open(filename_buf, std::ofstream::out | std::ofstream::app);
	_logopened = 1;
}
void _zalog_close() {
	_log_ofs.close();
	_logopened = 0;
}

void _zalog_print(int type, const char* format, ...) {
	if (!_logopened || !(type & _param)) return;

	const char* strmark;
	switch (type)
	{
	case ZALOG_TYPE_INFO:
		strmark = _strmark_info;
		break;
	case ZALOG_TYPE_DEBUG:
		strmark = _strmark_debug;
		break;
	case ZALOG_TYPE_ERROR:
		strmark = _strmark_error;
		break;
	default:
		return;
	}

	char tbuff[2048];

	time_t tt;
	tm ltime, *ptm = &ltime;
	time(&tt);
	localtime_s(ptm, &tt);

	int len = 0;
	if (!(_param & ZALOG_PARAM_NOPREINFO)) {
		len = sprintf_s(tbuff, "[%02d-%02d-%02d %02d:%02d:%02d][%s]",
			ptm->tm_year - 100, ptm->tm_mon + 1, ptm->tm_mday,
			ptm->tm_hour, ptm->tm_min, ptm->tm_sec,
			strmark);
	}

	va_list argptr;
	va_start(argptr, format);
	vsnprintf(tbuff + len, sizeof(tbuff) - len, format, argptr);
	va_end(argptr);

	if (ZALOG_OUT_FILE & _param) _log_ofs << tbuff << std::endl;
	if (ZALOG_OUT_STDOUT & _param) _log_stdout << tbuff << std::endl;
	if (ZALOG_OUT_STDLOG & _param) _log_stdlog << tbuff << std::endl;
}

void _zalog_empty_line()
{
	if (!_logopened) return;
	if (ZALOG_OUT_FILE & _param) _log_ofs << std::endl;
	if (ZALOG_OUT_STDOUT & _param) _log_stdout << std::endl;
	if (ZALOG_OUT_STDLOG & _param) _log_stdlog << std::endl;
}
