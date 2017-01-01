#include "ZaScenaAnalyzer.h"

#include "ZaConst.h"
#include "ZaVoiceTable.h"
#include "ZaSound.h"
#include "ZaRemote.h"
#include "ZaErrorMsg.h"
#include "ZaData.h"
#include "ZaVoicePlayer.h"

#include <string>
#include <vector>
#include <future>

#include <Windows.h>

struct MessageData {
	unsigned raScena[3];
	unsigned raBloack;
	unsigned raText;
	bool scena_loaded;
};
static MessageData _messageData;

struct ScenaData {
	unsigned aScenaX;
	char* scneaNameX;
	char* scenaDataX;

	bool operator<(const ScenaData& b) const { return aScenaX < b.aScenaX; }
};
static ScenaData _scenas[3];
static char _scenaNameX[3][MAX_SCENANAME_LENGTH + 1];
static char _scenaDataX[3][MAX_SCENA_SIZE];
#define NAME_BUFF_SIZE (sizeof(_scenaNameX[0]) / sizeof(*_scenaNameX[0]))
#define DATA_BUFF_SIZE (sizeof(_scenaDataX[0]) / sizeof(*_scenaDataX[0]))

#define BUFF_TEXT_SIZE 1024
static char buffText[BUFF_TEXT_SIZE];
//static unsigned char buffTextSrc[BUFF_TEXT_SIZE];

static unsigned _raBlock;
static unsigned _raFirstText;
static unsigned _raCurText;

static const ScenaData* _curScenaData;

static bool _checkScenaName(const char *scenaName);
static int _textAnalysisCN(char *dst, const char* src, int max_length = BUFF_TEXT_SIZE - 1);
static const char* _textAnalysisJP(const char* buff);
static void _loadNewVoiceTable(const char* scenaName);
static void _clearAllVoiceTable();
static void _reLoadAllVoiceTables(bool* cancle, int* count);

static bool LoadScenaX(unsigned raScena, int X);

static int _playEndCallBack(void*) {
	::PostMessageA((HWND)Za::Remote::CurGameProcessIn->hMainWindow, (UINT)Za::Remote::CurGameProcessIn->msgId,
		(WPARAM)MSGTYPE_PLAYWAIT, (LPARAM)0);
	return 0;
}

static void _getSubFiles(const std::string& dir, const std::string& searchName, std::vector<std::string> &subs)
{
	WIN32_FIND_DATA wfdp;
	HANDLE hFindp = FindFirstFile((dir + '\\' + searchName).c_str(), &wfdp);
	if (hFindp != NULL) {
		do
		{
			if (!(wfdp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				subs.push_back(wfdp.cFileName);
			}
		} while (FindNextFile(hFindp, &wfdp));
	}
}

bool _checkScenaName(const char *scenaName) {
	int count = 0;
	while (*scenaName != 0) {
		if (*scenaName < 0x20 || *scenaName >= 0x80) return false;
		++scenaName;
		++count;
		if (count > MAX_SCENANAME_LENGTH) break;
	}
	return count >= MIN_SCENANAME_LENGTH && count <= MAX_SCENANAME_LENGTH;
}
int _textAnalysisCN(char *dst, const char* src, int max_length) {
	int first = 0;
	while ((unsigned char)*src < 0x20 || *src == 0xFF) {
		src++; first++;
	}
	if (RM_FWD_CTRL_CH) {
		while (*src == '#') {
			++src;
			while ((unsigned char)*src >= '0' && (unsigned char)*src <= '9') ++src;
			++src;
		}
	}
	while (*src != 0 && max_length > 0)
	{
		if (*src == 1) {
			*dst = '\n';
		}
		else if ((unsigned char)*src >= 0x20 && (unsigned char)*src < 0xFF) {
			*dst = *src;
		}
		else {
			break;
		}
		++src; ++dst; --max_length;
	}
	*dst = 0;
	return first;
}
const char* _textAnalysisJP(const char* buff) {
	if (!buff) return buff;
	const char* abuff = buff;
	if (RM_FWD_CTRL_CH) {
		while (*abuff == '#') {
			++abuff;
			while ((unsigned char)*abuff >= '0' && (unsigned char)*abuff <= '9') ++abuff;
			++abuff;
		}
	}
	return abuff;
}

void _loadNewVoiceTable(const char* scenaName) {
	Za::VoiceTable::AllGroups::SetCurGroup(scenaName);
}

void _clearAllVoiceTable() {
	Za::VoiceTable::AllGroups::Clear();
}
void _reLoadAllVoiceTables(bool* cancle, int* count) {
	_clearAllVoiceTable();

	std::vector<std::string> subs;
	const std::string& dir = Za::Remote::CurGameData->VoiceTablesDir;
	std::string searchName = "*." VOICE_TABLE_ATTR;
	_getSubFiles(dir, searchName, subs);

	*count = 0;
	for (auto &sub : subs) {
		if (cancle && *cancle) break;

		std::string _scenaNameX = sub.substr(0, sub.rfind('.'));
		Za::VoiceTable::AllGroups::AddGroup(_scenaNameX.c_str(), (dir + '\\' + sub).c_str());

		*count = Za::VoiceTable::AllGroups::GroupsNum();
	}
}

bool LoadScenaX(unsigned raScena, int X) {
	unsigned addScenaName;
	char * nameBuff = _scenaNameX[X]; nameBuff[0] = 0;
	if (!Za::Remote::RemoteRead(raScena + OFF_OFF_SCENANAME, &addScenaName, sizeof(addScenaName))) {
		Za::Error::SetErrMsg("读取远程数据失败！");
		return false;
	}

	if (addScenaName >= DATA_BUFF_SIZE - NAME_BUFF_SIZE || addScenaName < OFF_OFF_SCENANAME) {
		Za::Error::SetErrMsg();
		return false;
	}

	char * dataBuff = _scenaDataX[X];
	if (!Za::Remote::RemoteRead(raScena, dataBuff, addScenaName + NAME_BUFF_SIZE)) {
		Za::Error::SetErrMsg("读取远程数据失败！");
		return false;
	}
	dataBuff[addScenaName - 1] = 0x00;
	dataBuff[addScenaName - 2] = 0x20;
	for (int i = 0; i < NAME_BUFF_SIZE - 1; ++i) nameBuff[i] = dataBuff[addScenaName + i];

	nameBuff[NAME_BUFF_SIZE - 1] = 0;
	if (!_checkScenaName(nameBuff)) {
		nameBuff[0] = 0;
	}

	return true;
}

static bool _stop;
std::future<bool> fut;
static bool _reload_thread(bool* cancle, int* count, bool * finished, Za::Data::CallBackType callback) {
	_reLoadAllVoiceTables(cancle, count);
	*finished = true;
	if (callback) callback(nullptr);
	return true;
}
bool Za::ScenaAnalyzer::Init(Data::VoicePlayerOut & vpOut, const Data::VoicePlayerIn & vpIn)
{
	_curScenaData = nullptr;
	_raBlock = 0;
	_scenaNameX[0][0] = 0;
	memset(_scenaDataX, 0, sizeof(_scenaDataX));
	memset(&_messageData, 0, sizeof(_messageData));

	_stop = true;
	if (fut.valid()) fut.get();
	_stop = false;
	vpOut.Count = 0;
	vpOut.Finished = false;

	if (vpIn.asyn) {
		fut = std::async(_reload_thread, &_stop, &vpOut.Count, &vpOut.Finished, vpIn.callBack);
	}
	else {
		_reload_thread(&_stop, &vpOut.Count, &vpOut.Finished, vpIn.callBack);
	}

	return Za::VoicePlayer::Init();
}

bool Za::ScenaAnalyzer::End()
{
	_curScenaData = nullptr;
	_scenaNameX[0][0] = 0;

	_stop = true;
	if (fut.valid()) fut.get();

	_clearAllVoiceTable();
	return Za::VoicePlayer::End();
}

bool Za::ScenaAnalyzer::MessageReceived(Data::MessageOut & msgOut, Data::MessageIn & msgIn)
{
	const char* out_scenaName;
	int out_voiceID;
	const char* &cnText = msgOut.CnText;
	const char* &jpText = msgOut.JpText;
	bool wait;
	static char buff_voicefile[MAX_LENGTH_VOICE_ID * 2];
	msgOut.VoiceFileName = buff_voicefile;
	buff_voicefile[0] = 0;
	cnText = jpText = nullptr;
	msgOut.Type = msgIn.wparam;

	switch (msgIn.wparam)
	{
	case MSGTYPE_LOADSCENA:
		_messageData.raScena[0] = msgIn.lparam;
		_messageData.raScena[1] = _messageData.raScena[2] = _messageData.raBloack = _messageData.raText = 0;
		_messageData.scena_loaded = false;
		Za::VoicePlayer::ClearWait();
		Za::Sound::SetStopCallBack();
		Za::Sound::Stop();
		return true;
	case MSGTYPE_LOADSCENA1:
		if (!_messageData.raScena[0]) return false;;
		if (_messageData.raScena[1]) _messageData.raScena[2] = msgIn.lparam;
		else  _messageData.raScena[1] = msgIn.lparam;
		return true;
	case MSGTYPE_LOADBLOCK:
		if (!_messageData.raScena[0]) return false;;
		_messageData.raBloack = msgIn.lparam;
		_messageData.raText = 0;

		if (!_messageData.scena_loaded) {
			if (!DLoadScena(_messageData.raScena[0], out_scenaName)) {
				_messageData.raScena[0] = 0;
				return false;
			}
			if (_messageData.raScena[1]) {
				if (!DLoadScena1(_messageData.raScena[1], out_scenaName)) return false;
			}
			if (_messageData.raScena[2]) {
				if (!DLoadScena1(_messageData.raScena[2], out_scenaName)) return false;
			}
			_messageData.scena_loaded = true;
		}
		if (!DLoadBlock(_messageData.raBloack, out_scenaName)) {
			_messageData.raBloack = 0;
			return false;
		}
		return true;
	case MSGTYPE_SHOWTEXT:
		if (!_messageData.raScena[0] || !_messageData.raBloack) return false;
		_messageData.raText = msgIn.lparam;
		if (!DShowText(_messageData.raText, out_voiceID, cnText, jpText, wait)) {
			return false;
		}
		if (out_voiceID != INVAILD_VOICE_ID) {
			if (!wait || Za::Sound::Status() == Za::Sound::Status::Stop) {
				Za::Sound::SetStopCallBack();
				Za::VoicePlayer::ClearWait();
				return Za::VoicePlayer::PlayVoice(out_voiceID, buff_voicefile);
			}
			else {
				Za::Sound::SetStopCallBack(_playEndCallBack);
				Za::VoicePlayer::AddToWait(out_voiceID);
			}
		}
		return true;
	case MSGTYPE_PLAYWAIT:
	default:
		bool ret = Za::VoicePlayer::PlayWait(buff_voicefile);
		if(Za::VoicePlayer::GetWaitingNum() == 0) Za::Sound::SetStopCallBack();
		return ret;
	}
}

bool Za::ScenaAnalyzer::DLoadScena(unsigned raScena, const char* &out_scenaName)
{
	out_scenaName = nullptr;
	_curScenaData = nullptr;
	_raBlock = 0;
	memset(_scenas, 0, sizeof(_scenas));

	if (!LoadScenaX(raScena, 0)) return false;

	if (_scenaNameX[0] != 0) {

		_scenas[0].aScenaX = raScena;
		_scenas[0].scneaNameX = _scenaNameX[0];
		_scenas[0].scenaDataX = _scenaDataX[0];

		out_scenaName = _scenaNameX[0];
	}

	return true;
}
bool Za::ScenaAnalyzer::DLoadScena1(unsigned raScena1, const char* &out_scenaName)
{
	out_scenaName = nullptr;

	if (_scenas[1].aScenaX == 0) {
		if (!LoadScenaX(raScena1, 1)) return false;

		_scenas[1].aScenaX = raScena1;
		_scenas[1].scneaNameX = _scenaNameX[1];
		_scenas[1].scenaDataX = _scenaDataX[1];

		if (_scenas[0] < _scenas[1]) std::swap(_scenas[0], _scenas[1]);

		out_scenaName = _scenaNameX[1];
	}
	else {
		if (!LoadScenaX(raScena1, 2)) return false;

		_scenas[2].aScenaX = raScena1;
		_scenas[2].scneaNameX = _scenaNameX[2];
		_scenas[2].scenaDataX = _scenaDataX[2];

		if (_scenas[1] < _scenas[2]) {
			std::swap(_scenas[1], _scenas[2]);
			if (_scenas[0] < _scenas[1]) std::swap(_scenas[0], _scenas[1]);
		}

		out_scenaName = _scenaNameX[2];
	}

	return true;
}

bool Za::ScenaAnalyzer::DLoadBlock(unsigned raBlock, const char* &out_scenaName)
{
	out_scenaName = nullptr;
	if (!_scenaNameX[0][0]) return false;

	_raFirstText = 0;
	_raBlock = raBlock;

	for (int i = 0; i <  sizeof(_scenas) / sizeof(*_scenas); ++i) {
		if (_raBlock > _scenas[i].aScenaX) {
			if (_raBlock - _scenas[i].aScenaX >= MAX_SCENA_SIZE) {
				_raBlock = 0;
				return 0;
			}

			if (_scenas + i != _curScenaData) {
				_curScenaData = _scenas + i;

				_loadNewVoiceTable(_curScenaData->scneaNameX);
			}

			out_scenaName = _curScenaData->scneaNameX;
			break;
		}
	}

	return true;
}
bool Za::ScenaAnalyzer::DShowText(unsigned raText,
	int & out_voiceID, const char* &cnText, const char* &jpText, bool & out_wait)
{
	out_voiceID = INVAILD_VOICE_ID; out_wait = false;
	cnText = jpText = nullptr;

	if (_curScenaData == nullptr || _raBlock == 0) return false;

	_raCurText = raText;
	if (_raFirstText == 0) _raFirstText = raText;

	unsigned offset = _raBlock - _curScenaData->aScenaX + _raCurText - _raFirstText;
	if (offset > MAX_SCENA_SIZE)
		return false;

	offset += _textAnalysisCN(buffText, _curScenaData->scenaDataX + offset, sizeof(buffText) - 1);
	cnText = buffText;

	if (buffText[0] != 0) {
		auto vinf = Za::VoiceTable::GetVoiceInfo(offset);
		if (vinf == nullptr) {
			vinf = Za::VoiceTable::GetVoiceInfo(offset + FAKE_OFFSET);
			if (vinf != nullptr) {
				offset += FAKE_OFFSET;
				out_wait = true;
			}
		}
		out_voiceID = vinf ? vinf->voiceId : INVAILD_VOICE_ID;

		const char* jpTextOri = vinf ? vinf->jpText : nullptr;
		jpText = _textAnalysisJP(jpTextOri);
	}//if (buffText[0] != 0)
	else return false;

	return true;
}

