#include "ZaVoicePlayer.h"

#include "ZaConst.h"
#include "ZaLog.h"
#include "ZaConfig.h"
#include "ZaSound.h"
#include "ZaVoiceTable.h"
#include "ZaRemote.h"
#include "ZaScenaAnalyzer.h"

#include <io.h>
#include <Windows.h>
#include <string>

static int voiceIdWait;
static ZAData zaData, zaData_old;

static bool PlayVoice(int voiceID, std::string &filename) {
	if (voiceID == InValidVoiceId) return false;

	const std::string& dir = zaConfig->ActiveGame->VoiceDir;
	const std::string strVoiceID = StrVoiceID(voiceID);

	for (auto ext : zaConfig->ActiveGame->VoiceExt)
	{
		filename = zaConfig->ActiveGame->VoiceName + strVoiceID + '.' + ext;
		std::string filePath = dir + '\\' + filename;
		
		if (_access(filePath.c_str(), 4) == 0)
			return ZaSoundPlay(filePath);
	}

	return false;
}

static int VoicePlayerLoopMain()
{
	int errc = 0;
	std::string voiceFileName;
	const char* scenaName = nullptr;
	int voiceID = InValidVoiceId;
	bool wait = false;

	if (voiceIdWait != InValidVoiceId && ZaSoundStatus() == ZASOUND_STATUS_STOP) {
		if (PlayVoice(voiceIdWait, voiceFileName)) {
			ZALOG("Playing %s ...", voiceFileName.c_str());
		}
		else {
			ZALOG_ERROR("Play %s Failed.", voiceFileName.c_str());
		}
		voiceIdWait = InValidVoiceId;
	}


	if (!ZaRemoteRead(rAddData, &zaData, sizeof(zaData))) {
		ZALOG_ERROR("访问远程数据失败: zaData");
		return 1;
	}

	
	if (zaData.cScena != zaData_old.cScena) {
		Sleep(100);
		
		zaData_old.aScena1 = zaData_old.aScena2 = 0;
		zaData_old.cBlock = 0;
		errc = ZaDetected_LoadScena(zaData.aScena, scenaName);
		if (errc) return errc;
	}

	if (zaData.aScena1 && zaData.aScena1 != zaData_old.aScena1) {
		errc = ZaDetected_LoadScena1(zaData.aScena1, scenaName);
		if (errc) return errc;
	}
	if (zaData.aScena2 && zaData.aScena2 != zaData_old.aScena2) {
		errc = ZaDetected_LoadScena1(zaData.aScena2, scenaName);
		if (errc) return errc;
	}

	if (zaData.cBlock && zaData.cBlock != zaData_old.cBlock) {
		zaData_old.cText = 0;
		errc = ZaDetected_LoadBlock(zaData.aCurBlock, scenaName);
		if (errc) return errc;
	}

	if (zaData.cText && zaData.cText != zaData_old.cText) {
		errc = ZaDetected_ShowText(zaData.aCurText, voiceID, wait);
		if (errc) return errc;
	}

	if (voiceID != InValidVoiceId) {
		if (!wait || ZaSoundStatus() == ZASOUND_STATUS_STOP || voiceIdWait != InValidVoiceId) {
			if (PlayVoice(voiceID, voiceFileName)) {
				ZALOG_DEBUG("Playing %s ...", voiceFileName.c_str());
			}
			else {
				ZALOG_ERROR("Play %s failed.", voiceFileName.c_str());
			}
			voiceIdWait = InValidVoiceId;
		}
		else {
			voiceIdWait = voiceID;
		}
	}

	return 0;
}

static int VoicePlayerLoopAfterOneLoop() {
	zaData_old = zaData;
	return 0;
}

int ZaVoicePlayerInit() {
	ZaSoundInit(zaConfig->ActiveGame->Volume);
	
	memset(&zaData, 0, sizeof(zaData));
	memset(&zaData_old, 0, sizeof(zaData_old));
	voiceIdWait = InValidVoiceId;

	ZaScenaAnalyzerInit();

	return 0;
}

int ZaVoicePlayerLoopOne()
{
	int errc = 0;
	errc |= VoicePlayerLoopMain();
	errc |= VoicePlayerLoopAfterOneLoop();
	return errc;
}

