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
#include <queue>

static std::queue<int> waitList;
static ZAData zaData, zaData_old;

static int VoicePlayerLoopMain()
{
	int errc = 0;
	char voiceFileName[MAX_LENGTH_VOICE_ID * 2 + 1];
	const char* scenaName = nullptr;
	int voiceID = InValidVoiceId;
	bool wait = false;

	if (ZaWaitingNum() && ZaSoundStatus() == ZASOUND_STATUS_STOP) {
		ZaPlayWait();
	}

	if (!ZaRemoteRead(g_rAddData, &zaData, sizeof(zaData))) {
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
		if (!wait) {
			ZaClearWait();
			if (ZaPlayVoice(voiceID, voiceFileName)) {
				ZALOG_DEBUG("Playing %s ...", voiceFileName);
			}
			else {
				ZALOG_ERROR("Play %s failed.", voiceFileName);
			}
		}
		else 
		{
			ZaAddToWait(voiceID);
		}
	}

	return 0;
}

static int VoicePlayerLoopAfterOneLoop() {
	zaData_old = zaData;
	return 0;
}

void ZaAddToWait(int voiceId) {
	waitList.push(voiceId);
}

void ZaClearWait() {
	while (!waitList.empty()) waitList.pop();
}

int ZaWaitingNum() {
	return waitList.size();
}

int ZaPlayWait() {
	if (waitList.empty()) return 1;

	char voiceFileName[MAX_LENGTH_VOICE_ID * 2 + 1];
	int voiceId = waitList.front();
	waitList.pop();

	if (ZaPlayVoice(voiceId, voiceFileName)) {
		ZALOG("Playing %s ...", voiceFileName);
		return 0;
	}
	else {
		ZALOG_ERROR("Play %s Failed.", voiceFileName);
		return 1;
	}
}

bool ZaPlayVoice(int voiceID, char *out_filename) {
	if (voiceID == InValidVoiceId) return false;

	const std::string& dir = g_zaConfig->ActiveGame->VoiceDir;
	const std::string& preName = g_zaConfig->ActiveGame->VoiceName;
	int index = 0;
	for (; index < preName.size(); ++index) out_filename[index] = preName[index];

	index += GetStrVoiceID(voiceID, out_filename + index);
	out_filename[index++] = '.';

	for (auto ext : g_zaConfig->ActiveGame->VoiceExt)
	{
		for (int i = 0; i < ext.size(); ++i) out_filename[index + i] = ext[i];
		out_filename[index + ext.size()] = 0;

		std::string filePath = dir + '\\' + out_filename;

		if (_access(filePath.c_str(), 4) == 0)
			return ZaSoundPlay(filePath.c_str());
	}
	out_filename[index - 1] = 0;

	return false;
}

int ZaVoicePlayerInit() {
	ZaSoundInit(g_zaConfig->ActiveGame->Volume);
	
	memset(&zaData, 0, sizeof(zaData));
	memset(&zaData_old, 0, sizeof(zaData_old));
	ZaClearWait();

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

