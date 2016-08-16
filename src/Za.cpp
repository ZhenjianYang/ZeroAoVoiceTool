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
	if (!zaConfigData.debuglog) {
		logparam = ZALOG_OUT_STDOUT | ZALOG_TYPE_INFO | ZALOG_TYPE_INFO | ZALOG_PARAM_NOPREINFO;
	}
	else {
		logparam = ZALOG_OUT_STDLOG | ZALOG_TYPE_ALL;
	}
	if (zaConfigData.uselogfile) {
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
	int amode = ZaRemoteInit(zaConfigData.mode);
	if (amode != MODE_AO && amode != MODE_ZERO) {
		ZALOG_ERROR("准备活动失败！");
		return;
	}
	ZALOG("就绪");

	ZaVoicePlayerInit(amode);
	ZALOG("已进入语音播放系统");
	ZALOG("语音文件目录为: %s", amode == MODE_AO ? zaConfigData.ao_dir_voice.c_str() : zaConfigData.zero_dir_voice.c_str());
	ZALOG("语音文件后缀为: %s", amode == MODE_AO ? zaConfigData.ao_ext_voice.c_str() : zaConfigData.zero_ext_voiceFile.c_str());
	ZALOG("语音表文件目录为: %s", amode == MODE_AO ? zaConfigData.ao_dir_voiceTable.c_str() : zaConfigData.zero_dir_voiceTable.c_str());
	ZALOG("语音表文件后缀为: %s", amode == MODE_AO ? zaConfigData.ao_ext_voiceTable.c_str() : zaConfigData.zero_ext_voiceTable.c_str());
	if (amode == MODE_AO && zaConfigData.ao_disableOriginalVoice) {
		ZALOG("启用了禁用原有剧情语音的功能");
	}

	int errc = 0;
	while (!errc)
	{
		errc = ZaVoicePlayerLoopOne();
		Sleep(zaConfigData.sleepTime);
	}
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

