#include "ZaScenaAnalyzer.h"

#include "ZaConst.h"
#include "ZaLog.h"
#include "ZaConfig.h"
#include "ZaVoiceTable.h"
#include "ZaRemote.h"
#include "ZaIo.h"

#include <string>
#include <vector>

struct ScenaData {
	unsigned aScenaX;
	char* scneaNameX;
	unsigned char* scenaDataX;

	bool operator<(const ScenaData& b) const { return aScenaX < b.aScenaX; }
};
static ScenaData _scenas[3];
static char _scenaNameX[3][MAX_SCENANAME_LENGTH + 1];
static unsigned char _scenaDataX[3][MAX_SCENA_SIZE];
#define NAME_BUFF_SIZE (sizeof(_scenaNameX[0]) / sizeof(*_scenaNameX[0]))
#define DATA_BUFF_SIZE (sizeof(_scenaDataX[0]) / sizeof(*_scenaDataX[0]))

#define BUFF_TEXT_SIZE 1024
static unsigned char buffText[BUFF_TEXT_SIZE];
//static unsigned char buffTextSrc[BUFF_TEXT_SIZE];

static unsigned _raBlock;
static unsigned _raFirstText;
static unsigned _raCurText;

static const ScenaData* _curScenaData;

static bool _checkScenaName(const char *scenaName);
static int _textAnalysisCN(unsigned char *dst, const unsigned char* src, int max_length = BUFF_TEXT_SIZE - 1);
static const unsigned char* _textAnalysisJP(const unsigned char* buff);
static void _loadNewVoiceTable(const char* scenaName);
static void _clearAllVoiceTable();
static void _reLoadAllVoiceTables(void * data);

static int LoadScenaX(unsigned raScena, int X);

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
int _textAnalysisCN(unsigned char *dst, const unsigned char* src, int max_length) {
	int first = 0;
	while (*src < 0x20 || *src == 0xFF) {
		src++; first++;
	}
	if (Za::Config::MainConfig->General->RemoveFwdCtrlCh) {
		while (*src == '#') {
			++src;
			while (*src >= '0' && *src <= '9') ++src;
			++src;
		}
	}
	while (*src != 0 && max_length > 0)
	{
		if (*src == 1) {
			*dst = '\n';
		}
		else if (*src >= 0x20 && *src < 0xFF) {
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
const unsigned char* _textAnalysisJP(const unsigned char* buff) {
	if (!buff) return buff;
	const unsigned char* abuff = buff;
	if (Za::Config::MainConfig->General->RemoveFwdCtrlCh) {
		while (*abuff == '#') {
			++abuff;
			while (*abuff >= '0' && *abuff <= '9') ++abuff;
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
void _reLoadAllVoiceTables(void * data) {
	_clearAllVoiceTable();

	std::vector<std::string> subs;
	const std::string& dir = Za::Config::MainConfig->ActiveGame->VoiceTableDir;
	std::string searchName = "*." + Za::Config::MainConfig->ActiveGame->VoiceTableExt;
	ZaGetSubFiles(dir, searchName, subs);

	for (auto sub : subs) {
		if (data && *(unsigned*)data) break;

		std::string _scenaNameX = sub.substr(0, sub.rfind('.'));
		Za::VoiceTable::AllGroups::AddGroup(_scenaNameX.c_str(), (dir + '\\' + sub).c_str());
	}
}

int LoadScenaX(unsigned raScena, int X) {
	unsigned addScenaName;
	char * nameBuff = _scenaNameX[X]; nameBuff[0] = 0;
	if (!Za::Remote::RemoteRead(raScena + OFF_OFF_SCENANAME, &addScenaName, sizeof(addScenaName))) {
		ZALOG_ERROR("访问远程数据失败: zaData.aScena");
		return 1;
	}

	if (addScenaName >= DATA_BUFF_SIZE - NAME_BUFF_SIZE || addScenaName < OFF_OFF_SCENANAME) { return 1; }

	unsigned char * dataBuff = _scenaDataX[X];
	if (!Za::Remote::RemoteRead(raScena, dataBuff, addScenaName + NAME_BUFF_SIZE)) {
		ZALOG_ERROR("访问远程数据失败: zaData.aScena");
		return 1;
	}
	dataBuff[addScenaName - 1] = 0x00;
	dataBuff[addScenaName - 2] = 0x20;
	for (int i = 0; i < NAME_BUFF_SIZE - 1; ++i) nameBuff[i] = dataBuff[addScenaName + i];

	nameBuff[NAME_BUFF_SIZE - 1] = 0;
	if (!_checkScenaName(nameBuff)) {
		nameBuff[0] = 0;
	}

	return 0;
}

int Za::ScenaAnalyzer::Init(void* data /*= 0*/)
{
	_curScenaData = nullptr;
	_raBlock = 0;
	_scenaNameX[0][0] = 0;
	memset(_scenaDataX, 0, sizeof(_scenaDataX));

	ZALOG_DEBUG("加载语音表...");
	_reLoadAllVoiceTables(data);
	ZALOG_DEBUG("已加载的语音表数: %d", Za::VoiceTable::AllGroups::GroupsNum());

	return 0;
}
int Za::ScenaAnalyzer::End()
{
	_clearAllVoiceTable();

	_curScenaData = nullptr;
	_scenaNameX[0][0] = 0;

	return 0;
}

int Za::ScenaAnalyzer::DLoadScena(unsigned raScena, const char* &out_scenaName)
{
	out_scenaName = nullptr;
	_curScenaData = nullptr;
	_raBlock = 0;
	memset(_scenas, 0, sizeof(_scenas));

	int errc;
	errc = LoadScenaX(raScena, 0);
	if (errc) return errc;

	if (_scenaNameX[0] != 0) {

		_scenas[0].aScenaX = raScena;
		_scenas[0].scneaNameX = _scenaNameX[0];
		_scenas[0].scenaDataX = _scenaDataX[0];

		out_scenaName = _scenaNameX[0];
	}

	return 0;
}
int Za::ScenaAnalyzer::DLoadScena1(unsigned raScena1, const char* &out_scenaName)
{
	out_scenaName = nullptr;

	int errc;
	if (_scenas[1].aScenaX == 0) {
		errc = LoadScenaX(raScena1, 1);
		if (errc) return errc;

		_scenas[1].aScenaX = raScena1;
		_scenas[1].scneaNameX = _scenaNameX[1];
		_scenas[1].scenaDataX = _scenaDataX[1];

		if (_scenas[0] < _scenas[1]) std::swap(_scenas[0], _scenas[1]);

		out_scenaName = _scenaNameX[1];
	}
	else {
		errc = LoadScenaX(raScena1, 2);
		if (errc) return errc;

		_scenas[2].aScenaX = raScena1;
		_scenas[2].scneaNameX = _scenaNameX[2];
		_scenas[2].scenaDataX = _scenaDataX[2];

		if (_scenas[1] < _scenas[2]) {
			std::swap(_scenas[1], _scenas[2]);
			if (_scenas[0] < _scenas[1]) std::swap(_scenas[0], _scenas[1]);
		}

		out_scenaName = _scenaNameX[2];
	}

	return 0;
}

int Za::ScenaAnalyzer::DLoadBlock(unsigned raBlock, const char* &out_scenaName)
{
	out_scenaName = nullptr;
	if (!_scenaNameX[0][0]) return 0;

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

				ZALOG_DEBUG("Scena:%s, Loading Voice Table...", _curScenaData->scneaNameX);
				_loadNewVoiceTable(_curScenaData->scneaNameX);
				ZALOG_DEBUG("Voice Table Records：%d", Za::VoiceTable::Num());
			}

			out_scenaName = _curScenaData->scneaNameX;
			break;
		}
	}

	return 0;
}
int Za::ScenaAnalyzer::DShowText(unsigned raText, int & out_voiceID, bool & out_wait)
{
	out_voiceID = INVAILD_VOICE_ID; out_wait = false;

	if (_curScenaData == nullptr || _raBlock == 0) return 0;

	_raCurText = raText;
	if (_raFirstText == 0) _raFirstText = raText;

	unsigned offset = _raBlock - _curScenaData->aScenaX + _raCurText - _raFirstText;
	if (offset > MAX_SCENA_SIZE)
		return 0;

	offset += _textAnalysisCN(buffText, _curScenaData->scenaDataX + offset, sizeof(buffText) - 1);

	if (buffText[0] != 0 || 1) {
		auto vinf = Za::VoiceTable::GetVoiceInfo(offset);
		if (vinf == nullptr) {
			vinf = Za::VoiceTable::GetVoiceInfo(offset + FAKE_OFFSET);
			if (vinf != nullptr) {
				offset += FAKE_OFFSET;
				out_wait = true;
			}
		}
		out_voiceID = vinf ? vinf->voiceId : INVAILD_VOICE_ID;

		const unsigned char* buffTextJP = vinf ? (const unsigned char*)vinf->jpText : nullptr;
		const unsigned char* buffTextJPFixed = _textAnalysisJP(buffTextJP);
		char buffStrVoiceId[MAX_LENGTH_VOICE_ID + 1];
		GetStrVoiceID(out_voiceID, Za::Config::MainConfig->ActiveGame->VoiceIdLength, buffStrVoiceId);

		if (buffTextJPFixed && buffTextJPFixed[0] != 0) {
			ZALOG_DEBUG("\nScena:%s,Offset:0x%06X,VoiceID:%s\n"
				"------------------------------------\n"
				"%s\n"
				"------------------------------------\n"
				"%s\n"
				"------------------------------------",
				_curScenaData->scneaNameX, offset, buffStrVoiceId,
				buffText, buffTextJPFixed);
		}
		else {
			ZALOG_DEBUG("\nScena:%s,Offset:0x%06X,VoiceID:%s\n"
				"------------------------------------\n"
				"%s\n"
				"------------------------------------",
				_curScenaData->scneaNameX, offset, buffStrVoiceId,
				buffText);
		}
	}//if (buffText[0] != 0)

	return 0;
}

