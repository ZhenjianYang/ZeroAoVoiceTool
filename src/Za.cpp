#include "Za.h"
#include "ZaSound.h"

using namespace Za;

#define MAX_ERRMSG_LEGNTH 200

#define MAX_TITLE_LENGTH 50
#define MAX_COMMENT_LEGNTH 200

static char buff_errmsg[MAX_ERRMSG_LEGNTH + 1];

#define CpyStrToArray(dst, src) StrCpyN(dst, src, sizeof(dst) / sizeof(*dst) - 1);
static char* StrCpyN(char* dst, const char* src, int maxlen) {
	for (int i = 0; i < maxlen && src[i]; ++i) {
		dst[i] = src[i];
	}
	dst[maxlen] = 0;
	return dst;
}
static void SetErrMsg(const char* errMsg = nullptr) {
	if (errMsg) { CpyStrToArray(buff_errmsg, errMsg); }
	else buff_errmsg[0] = '\0';
}

bool Za::Main::Init()
{
	if (Za::Sound::Init()) {
		SetErrMsg("³õÊ¼»¯ÒôÆµÏµÍ³Ê§°Ü£¡");
		return false;
	}
	return true;
}

bool Za::Main::End()
{
	if (Za::Sound::End()) {
		SetErrMsg("ÖÕÖ¹ÒôÆµÏµÍ³Ê§°Ü£¡");
		return false;
	}
	return true;
}

bool Za::Main::CheckGameStart(Data::GameOut & gameOut)
{
	return false;
}

bool Za::Main::CheckGameEnd()
{
	return false;
}

bool Za::Main::OpenGameThread(const Data::ThreadIn & threadIn)
{
	return false;
}

bool Za::Main::CloseGameThread()
{
	return false;
}

bool Za::Main::LoadVoiceTables(Data::VoiceTableOut & vtblOut)
{
	return false;
}

bool Za::Main::LoadVoiceTablesAsyn(Data::VoiceTableOut & vtblOut, const Data::VoiceTableIn & vtblIn)
{
	return false;
}

bool Za::Main::LoadVoiceTablesAsynCancle(Data::VoiceTableOut & vtblOut, const Data::VoiceTableIn & vtblIn)
{
	return false;
}

bool Za::Main::SetVoicePlayConfig(const Data::PlayConfigIn & playConfigIn)
{
	return false;
}

bool Za::Main::MessageRecived(Data::MessageOut & msgOut, Data::MessageIn & msgIn)
{
	return false;
}

const char * Za::Main::LastErr()
{
	return buff_errmsg;
}
