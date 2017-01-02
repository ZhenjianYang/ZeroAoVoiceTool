#include "ZaErrorMsg.h"
#include "ZaCommonMethod.h"

static char buff_errmsg[MAX_ERRMSG_LEGNTH + 1];

const char * Za::Error::LastErrMsg()
{
	return buff_errmsg;
}

void Za::Error::SetErrMsg(const char * errMsg)
{
	if (errMsg) { CpyStrToArray(buff_errmsg, errMsg); }
	else buff_errmsg[0] = '\0';
}
