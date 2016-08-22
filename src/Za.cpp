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
	if (!g_zaConfig->General.OpenDebugLog) {
		logparam = ZALOG_OUT_STDOUT | ZALOG_TYPE_INFO | ZALOG_TYPE_INFO | ZALOG_PARAM_NOPREINFO;
	}
	else {
		logparam = ZALOG_OUT_STDLOG | ZALOG_TYPE_ALL;
	}
	if (g_zaConfig->General.UseLogFile) {
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
	ZALOG("准备中...");
	int gameID = ZaRemoteInit(g_zaConfig->General.Mode);
	if (gameID != GAMEID_AO && gameID != GAMEID_ZERO) {
		ZALOG_ERROR("准备活动失败！");
		return;
	}
	ZaConfigSetActive(gameID);
	ZALOG("就绪");

	ZaVoicePlayerInit();
	ZALOG("已进入语音播放系统");
	
	ZALOG("语音文件目录为: %s", g_zaConfig->ActiveGame->VoiceDir.c_str());
	for (int i = 1; i <= g_zaConfig->ActiveGame->VoiceExt.size(); ++i) {
		ZALOG("语音文件后缀%d: %s", i, g_zaConfig->ActiveGame->VoiceExt[i-1].c_str());
	}
	if (gameID == GAMEID_AO && g_zaConfig->ActiveGame->DisableOriginalVoice) {
		ZALOG("启用了禁用原有剧情语音的功能");
	}

	while (!ZaCheckGameEnd())
	{
		int errc = ZaVoicePlayerLoopOne();
		Sleep(g_zaConfig->General.SleepTime);
	}
	ZALOG("游戏已退出！");
	ZALOG("已退出语音播放系统");

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

