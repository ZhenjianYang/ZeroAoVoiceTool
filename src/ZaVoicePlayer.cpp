#include "ZaVoicePlayer.h"

#include "ZaConst.h"
#include "ZaSound.h"
#include "ZaVoiceTable.h"
#include "ZaRemote.h"
#include "ZaScenaAnalyzer.h"
#include "ZaErrorMsg.h"
#include "ZaCommonMethod.h"

#include <io.h>
#include <Windows.h>
#include <string>
#include <queue>

static std::queue<int> _waitList;

void Za::VoicePlayer::AddToWait(int voiceId) {
	_waitList.push(voiceId);
}

void Za::VoicePlayer::ClearWait() {
	while (!_waitList.empty()) _waitList.pop();
}

int Za::VoicePlayer::GetWaitingNum() {
	return _waitList.size();
}

bool Za::VoicePlayer::PlayWait(char *out_filename /*= nullptr*/) {
	if (_waitList.empty()) return false;

	int voiceId = _waitList.front();
	_waitList.pop();

	return Za::VoicePlayer::PlayVoice(voiceId, out_filename);
}

bool Za::VoicePlayer::PlayVoice(int voiceId, char *out_filename /*= nullptr*/) {
	if (voiceId == INVAILD_VOICE_ID) return false;
	static char buff_voiceFileName[MAX_LENGTH_VOICE_ID * 2 + 1];

	char* buff_filename = out_filename == nullptr ? buff_voiceFileName : out_filename;

	const std::string& dir = Za::Remote::CurGameData->VoiceFileDir;
	const std::string& preName = Za::Remote::CurGameData->VoiceFileName;
	const int voiceIdLength = Za::Remote::CurGameData->VoiceIdLegnth;

	int index = 0;
	for (; index < (int)preName.size(); ++index) buff_filename[index] = preName[index];

	for (int i = voiceIdLength - 1; i >= 0; --i) {
		buff_filename[index + i] = voiceId % 10 + '0';
		voiceId /= 10;
	}
	index += voiceIdLength;
	buff_filename[index++] = '.';

	static const char* attrs[] = VOICE_FILE_ATTRS;
	for (int j = 0; j < sizeof(attrs) / sizeof(*attrs); ++j)
	{
		CpyStrToArray(buff_filename + index, attrs[j]);
		std::string filePath = dir + '\\' + buff_filename;

		if (_access(filePath.c_str(), 4) == 0)
			return Za::Sound::Play(filePath.c_str());
	}
	buff_filename[index - 1] = 0;

	Za::Error::SetErrMsg("语音文件不存在！");
	return false;
}

bool Za::VoicePlayer::Init(){
	Za::VoicePlayer::ClearWait();
	return true;
}

bool Za::VoicePlayer::End()
{
	Za::VoicePlayer::ClearWait();
	return true;
}

