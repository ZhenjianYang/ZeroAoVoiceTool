#include "Za.h"

#include "ZaConst.h"
#include "ZaLog.h"
#include "ZaConfig.h"
#include "ZaRemote.h"
#include "ZaVoicePlayer.h"
#include "ZaSound.h"

#include <Windows.h>

static void SetWorkPath(const char* exepath);
static void ZaMain();

void ZaVoiceStartup(int argc, char * argv[])
{
	SetWorkPath(argv[0]);
	Za::Config::LoadFromFile(DFT_CONFIG_FILE);

	int logparam = 0;
	if (!Za::Config::MainConfig->General->OpenDebugLog) {
		logparam = ZALOG_OUT_STDOUT | ZALOG_TYPE_INFO | ZALOG_TYPE_ERROR | ZALOG_PARAM_NOPREINFO;
	}
	else {
		logparam = ZALOG_OUT_STDLOG | ZALOG_TYPE_ALL;
	}
	if (Za::Config::MainConfig->General->UseLogFile) {
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
	int errc;
	ZALOG("准备中...");

	Za::Sound::Init();
	ZALOG("音频系统已启动");

	ZALOG("等待游戏运行...");
	int gameID = Za::Remote::WaitGameStart(Za::Config::MainConfig->General->Mode);
	ZALOG("游戏已启动！");

	errc = Za::Remote::Init(gameID);
	if (errc) {
		ZALOG_ERROR("准备活动失败！");
		return;
	}

	Za::Config::SetActiveGame(gameID);
	Za::Sound::SetVolumn(Za::Config::MainConfig->ActiveGame->Volume);
	ZALOG("就绪");

	ZaVoicePlayerInit();
	ZALOG("已进入语音播放系统");
	
	ZALOG("语音文件目录为: %s", Za::Config::MainConfig->ActiveGame->VoiceDir.c_str());
	for (int i = 1; i <= (int)Za::Config::MainConfig->ActiveGame->VoiceExt.size(); ++i) {
		ZALOG("语音文件后缀%d: %s", i, Za::Config::MainConfig->ActiveGame->VoiceExt[i-1].c_str());
	}
	if (gameID == GAMEID_AO && Za::Config::MainConfig->ActiveGame->DisableOriginalVoice) {
		ZALOG("启用了禁用原有剧情语音的功能");
	}

	while (!Za::Remote::CheckGameEnd())
	{
		errc = ZaVoicePlayerLoopOne();
		Sleep(Za::Config::MainConfig->General->SleepTime);
	}
	ZALOG("游戏已退出！");
	
	ZaVoicePlayerEnd();
	ZALOG("已退出语音播放系统");
	Za::Remote::End();
	ZALOG_DEBUG("已关闭远程进程句柄");
	Za::Sound::End();
	ZALOG("已关闭音频系统");
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

