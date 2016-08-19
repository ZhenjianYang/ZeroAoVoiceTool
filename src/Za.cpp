#include "Za.h"

#include "ZaConst.h"
#include "ZaLog.h"
#include "ZaConfig.h"
#include "ZaRemote.h"
#include "ZaVoicePlayer.h"

#include <Windows.h>

static void SetWorkPath(const char* exepath);
static void ZaMain();

void ZaVoiceStartup(int argc, char * argv[])
{
	SetWorkPath(argv[0]);
	ZaConfigLoad(DFT_CONFIG_FILE);

	int logparam = 0;
	if (!zaConfigData.General.OpenDebugLog) {
		logparam = ZALOG_OUT_STDOUT | ZALOG_TYPE_INFO | ZALOG_TYPE_INFO | ZALOG_PARAM_NOPREINFO;
	}
	else {
		logparam = ZALOG_OUT_STDLOG | ZALOG_TYPE_ALL;
	}
	if (zaConfigData.General.UseLogFile) {
		logparam |= ZALOG_OUT_FILE;
		ZALOG_SETLOGFILE(DFT_LOG_FILE);
	}

	ZALOG_OPEN_WITHPARAM(logparam);

	ZALOG("Zero Ao Voice Tool %s", ZA_VERSION);

	ZaMain();

	ZALOG("Zero Ao Voice Tool End!");

	ZALOG_CLOSE;
}

void ZaMain() {
	ZALOG("׼����...");
	int amode = ZaRemoteInit(zaConfigData.General.Mode);
	if (amode != MODE_AO && amode != MODE_ZERO) {
		ZALOG_ERROR("׼���ʧ�ܣ�");
		return;
	}
	ZALOG("����");

	ZaVoicePlayerInit(amode);
	ZALOG("�ѽ�����������ϵͳ");
	const ZaConfigDataGame* zaConfigGame = amode == MODE_AO ? &zaConfigData.Ao : &zaConfigData.Zero;
	ZALOG("�����ļ�Ŀ¼Ϊ: %s", zaConfigGame->VoiceDir.c_str());
	for (int i = 1; i <= zaConfigGame->VoiceExt.size(); ++i) {
		ZALOG("�����ļ���׺%d: %s", i, zaConfigGame->VoiceExt[i-1].c_str());
	}
	if (amode == MODE_AO && zaConfigGame->DisableOriginalVoice) {
		ZALOG("�����˽���ԭ�о��������Ĺ���");
	}

	int errc = 0;
	while (!errc)
	{
		errc = ZaVoicePlayerLoopOne();
		Sleep(zaConfigData.General.SleepTime);
	}
	ZALOG("���˳���������ϵͳ");

	ZaRemoteFinish();
}


void SetWorkPath(const char * exepath)
{
	char buff[1024];
	int p = -1;
	for(int i = 0; exepath[i] != 0; ++i)
	{
		if (exepath[i] == '\\') p = i;
		buff[i] = exepath[i];
	}
	if (p != -1) {
		buff[p] = 0;
	}
	else {
		buff[0] = '.'; buff[1] = 0;
	}

	SetCurrentDirectory(buff);
}

