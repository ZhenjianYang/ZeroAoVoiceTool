#include "ZaVoicePlayer.h"

#include "ZaConst.h"
#include "ZaLog.h"
#include "ZaConfig.h"
#include "ZaSound.h"
#include "ZaVoiceTable.h"
#include "ZaRemote.h"
#include "ZaScenaAnalyzer.h"

#include "ZaIo.h"

#include <io.h>
#include <Windows.h>
#include <string>
#include <queue>

static std::queue<int> _waitList;
static Za::Remote::RemoteData _zaData, _zaData_old;

static int VoicePlayerLoopMain()
{
	int errc = 0;
	char voiceFileName[MAX_LENGTH_VOICE_ID * 2 + 1];
	const char* _scenaNameX = nullptr;
	int voiceID = InValidVoiceId;
	bool wait = false;

	if (ZaWaitingNum() && ZaSoundStatus() == ZASOUND_STATUS_STOP) {
		ZaPlayWait();
	}

	if (!Za::Remote::RemoteRead(Za::Remote::RemoteDataAddr, &_zaData, sizeof(_zaData))) {
		ZALOG_ERROR("访问远程数据失败: zaData");
		return 1;
	}

	
	if (_zaData.cScena != _zaData_old.cScena) {
		Sleep(100);
		
		_zaData_old.aScena1 = _zaData_old.aScena2 = 0;
		_zaData_old.cBlock = 0;
		errc = Za::ScenaAnalyzer::DLoadScena(_zaData.aScena, _scenaNameX);
		if (errc) return errc;
	}

	if (_zaData.aScena1 && _zaData.aScena1 != _zaData_old.aScena1) {
		errc = Za::ScenaAnalyzer::DLoadScena1(_zaData.aScena1, _scenaNameX);
		if (errc) return errc;
	}
	if (_zaData.aScena2 && _zaData.aScena2 != _zaData_old.aScena2) {
		errc = Za::ScenaAnalyzer::DLoadScena1(_zaData.aScena2, _scenaNameX);
		if (errc) return errc;
	}

	if (_zaData.cBlock && _zaData.cBlock != _zaData_old.cBlock) {
		_zaData_old.cText = 0;
		errc = Za::ScenaAnalyzer::DLoadBlock(_zaData.aCurBlock, _scenaNameX);
		if (errc) return errc;
	}

	if (_zaData.cText && _zaData.cText != _zaData_old.cText) {
		errc = Za::ScenaAnalyzer::DShowText(_zaData.aCurText, voiceID, wait);
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
	_zaData_old = _zaData;
	return 0;
}

void ZaAddToWait(int voiceId) {
	_waitList.push(voiceId);
}

void ZaClearWait() {
	while (!_waitList.empty()) _waitList.pop();
}

int ZaWaitingNum() {
	return _waitList.size();
}

int ZaPlayWait() {
	if (_waitList.empty()) return 1;

	char voiceFileName[MAX_LENGTH_VOICE_ID * 2 + 1];
	int voiceId = _waitList.front();
	_waitList.pop();

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

	const std::string& dir = Za::Config::MainConfig->ActiveGame->VoiceDir;
	const std::string& preName = Za::Config::MainConfig->ActiveGame->VoiceName;
	int index = 0;
	for (; index < (int)preName.size(); ++index) out_filename[index] = preName[index];

	GetStrVoiceID(voiceID, Za::Config::MainConfig->ActiveGame->VoiceIdLength, out_filename + index);
	index += Za::Config::MainConfig->ActiveGame->VoiceIdLength;
	out_filename[index++] = '.';

	for (auto ext : Za::Config::MainConfig->ActiveGame->VoiceExt)
	{
		for (int i = 0; i < (int)ext.size(); ++i) out_filename[index + i] = ext[i];
		out_filename[index + ext.size()] = 0;

		std::string filePath = dir + '\\' + out_filename;

		if (_access(filePath.c_str(), 4) == 0)
			return ZaSoundPlay(filePath.c_str());
	}
	out_filename[index - 1] = 0;

	return false;
}

int ZaVoicePlayerInit(void* data /*= 0*/){
	memset(&_zaData, 0, sizeof(_zaData));
	memset(&_zaData_old, 0, sizeof(_zaData_old));
	ZaClearWait();

	Za::ScenaAnalyzer::Init(data);

	return 0;
}

int ZaVoicePlayerEnd()
{
	Za::ScenaAnalyzer::End();
	ZaClearWait();

	return 0;
}

int ZaVoicePlayerLoopOne()
{
	int errc = 0;
	errc |= VoicePlayerLoopMain();
	errc |= VoicePlayerLoopAfterOneLoop();
	return errc;
}

