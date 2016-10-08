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

static char buff_voiceFileName[MAX_LENGTH_VOICE_ID * 2 + 1];

static int VoicePlayerLoopMain()
{
	int errc = 0;
	
	const char* _scenaNameX = nullptr;
	int voiceID = INVAILD_VOICE_ID;
	bool wait = false;

	if (Za::VoicePlayer::GetWaitingNum() && Za::Sound::GetStatus() == Za::Sound::Status::Stop) {
		Za::VoicePlayer::PlayWait();
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

	if (voiceID != INVAILD_VOICE_ID) {
		if (!wait) {
			Za::VoicePlayer::ClearWait();
			if (Za::VoicePlayer::PlayVoice(voiceID, buff_voiceFileName)) {
				ZALOG_DEBUG("Playing %s ...", buff_voiceFileName);
			}
			else {
				ZALOG_ERROR("Play %s failed.", buff_voiceFileName);
			}
		}
		else 
		{
			Za::VoicePlayer::AddToWait(voiceID);
		}
	}

	return 0;
}

static int VoicePlayerLoopAfterOneLoop() {
	_zaData_old = _zaData;
	return 0;
}

void Za::VoicePlayer::AddToWait(int voiceId) {
	_waitList.push(voiceId);
}

void Za::VoicePlayer::ClearWait() {
	while (!_waitList.empty()) _waitList.pop();
}

int Za::VoicePlayer::GetWaitingNum() {
	return _waitList.size();
}

int Za::VoicePlayer::PlayWait() {
	if (_waitList.empty()) return 1;

	char buff_voiceFileName[MAX_LENGTH_VOICE_ID * 2 + 1];
	int voiceId = _waitList.front();
	_waitList.pop();

	if (Za::VoicePlayer::PlayVoice(voiceId, buff_voiceFileName)) {
		ZALOG("Playing %s ...", buff_voiceFileName);
		return 0;
	}
	else {
		ZALOG_ERROR("Play %s Failed.", buff_voiceFileName);
		return 1;
	}
}

bool Za::VoicePlayer::PlayVoice(int voiceId, char *out_filename /*= nullptr*/) {
	if (voiceId == INVAILD_VOICE_ID) return false;
	
	char* buff_filename = out_filename == nullptr ? buff_voiceFileName : out_filename;

	const std::string& dir = Za::Config::MainConfig->ActiveGame->VoiceDir;
	const std::string& preName = Za::Config::MainConfig->ActiveGame->VoiceName;
	int index = 0;
	for (; index < (int)preName.size(); ++index) buff_filename[index] = preName[index];

	GetStrVoiceID(voiceId, Za::Config::MainConfig->ActiveGame->VoiceIdLength, buff_filename + index);
	index += Za::Config::MainConfig->ActiveGame->VoiceIdLength;
	buff_filename[index++] = '.';

	for (auto &ext : Za::Config::MainConfig->ActiveGame->VoiceExt)
	{
		for (int i = 0; i < (int)ext.size(); ++i) buff_filename[index + i] = ext[i];
		buff_filename[index + ext.size()] = 0;

		std::string filePath = dir + '\\' + buff_filename;

		if (_access(filePath.c_str(), 4) == 0)
			return Za::Sound::Play(filePath.c_str());
	}
	buff_filename[index - 1] = 0;

	return false;
}

int Za::VoicePlayer::Init(void* data /*= 0*/){
	memset(&_zaData, 0, sizeof(_zaData));
	memset(&_zaData_old, 0, sizeof(_zaData_old));
	Za::VoicePlayer::ClearWait();

	Za::ScenaAnalyzer::Init(data);

	return 0;
}

int Za::VoicePlayer::End()
{
	Za::ScenaAnalyzer::End();
	Za::VoicePlayer::ClearWait();

	return 0;
}

int Za::VoicePlayer::LoopOne()
{
	int errc = 0;
	errc |= VoicePlayerLoopMain();
	errc |= VoicePlayerLoopAfterOneLoop();
	return errc;
}

