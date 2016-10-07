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
	char* pScneaNameX;

	bool operator<(const ScenaData& b) { return aScenaX < b.aScenaX; }
};
static ScenaData _scenas[3];
static char _scenaNameX[3][MAX_SCENANAME_LENGTH + 1];
#define NAME_BUFF_SIZE (sizeof(_scenaNameX[0]) / sizeof(*_scenaNameX[0]))

#define BUFF_TEXT_SIZE 1024
static unsigned char buffText[BUFF_TEXT_SIZE];
static unsigned char buffTextSrc[BUFF_TEXT_SIZE];

static unsigned _raBlock;
static unsigned _raFirstText;
static unsigned _raCurText;

static char* _pScenaName;
static unsigned _offset1;

static bool _checkScenaName(const char *scenaName);
static int _textAnalysisCN(unsigned char *dst, const unsigned char* src);
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
int _textAnalysisCN(unsigned char *dst, const unsigned char* src) {
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
	while (*src != 0)
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
		++src; ++dst;
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
	char * nameBuff = _scenaNameX[X];
	if (!Za::Remote::RemoteRead(raScena + OFF_OFF_SCENANAME, &addScenaName, sizeof(addScenaName))
		|| !Za::Remote::RemoteRead(raScena + addScenaName, nameBuff, NAME_BUFF_SIZE)) {
		ZALOG_ERROR("访问远程数据失败: zaData.aScena");
		return 1;
	}
	nameBuff[NAME_BUFF_SIZE - 1] = 0;
	if (!_checkScenaName(nameBuff)) {
		nameBuff[0] = 0;
	}

	return 0;
}

int Za::ScenaAnalyzer::Init(void* data /*= 0*/)
{
	_pScenaName = NULL;
	_scenaNameX[0][0] = 0;

	ZALOG_DEBUG("加载语音表...");
	_reLoadAllVoiceTables(data);
	ZALOG_DEBUG("已加载的语音表数: %d", Za::VoiceTable::AllGroups::GroupsNum());

	return 0;
}
int Za::ScenaAnalyzer::End()
{
	_clearAllVoiceTable();

	_pScenaName = NULL;
	_scenaNameX[0][0] = 0;

	return 0;
}

int Za::ScenaAnalyzer::DLoadScena(unsigned raScena, const char* &out_scenaName)
{
	out_scenaName = nullptr;
	_pScenaName = nullptr;
	memset(_scenas, 0, sizeof(_scenas));

	int errc;
	errc = LoadScenaX(raScena, 0);
	if (errc) return errc;

	if (_scenaNameX[0] != 0) {

		_scenas[0].aScenaX = raScena;
		_scenas[0].pScneaNameX = _scenaNameX[0];

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
		_scenas[1].pScneaNameX = _scenaNameX[1];

		if (_scenas[0] < _scenas[1]) std::swap(_scenas[0], _scenas[1]);

		out_scenaName = _scenaNameX[1];
	}
	else {
		errc = LoadScenaX(raScena1, 2);
		if (errc) return errc;

		_scenas[2].aScenaX = raScena1;
		_scenas[2].pScneaNameX = _scenaNameX[2];

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
	if (_scenaNameX[0] == 0) return 0;

	_raFirstText = 0;
	_raBlock = raBlock;

	for (int i = 0; i <  sizeof(_scenas) / sizeof(*_scenas); ++i) {
		if (_raBlock > _scenas[i].aScenaX) {
			if (_raBlock - _scenas[i].aScenaX >= MAX_SCENA_SIZE) {
				_raBlock = 0;
				return 0;
			}

			_offset1 = _raBlock - _scenas[i].aScenaX;

			if (_pScenaName != _scenas[i].pScneaNameX) {
				_pScenaName = _scenas[i].pScneaNameX;

				ZALOG_DEBUG("Scena:%s, Loading Voice Table...", _pScenaName);
				_loadNewVoiceTable(_pScenaName);
				ZALOG_DEBUG("Voice Table Records：%d", Za::VoiceTable::Num());
			}

			out_scenaName = _scenas[i].pScneaNameX;
			break;
		}
	}

	return 0;
}
int Za::ScenaAnalyzer::DShowText(unsigned raText, int & out_voiceID, bool & out_wait)
{
	out_voiceID = INVAILD_VOICE_ID; out_wait = false;

	if (_scenaNameX[0] == 0 || _raBlock == 0) return 0;

	_raCurText = raText;
	if (_raFirstText == 0) _raFirstText = raText;

	unsigned offset = _offset1 + _raCurText - _raFirstText;
	if (offset > MAX_SCENA_SIZE)
		return 0;

	if (!Za::Remote::RemoteRead(_raCurText, buffTextSrc, sizeof(buffTextSrc))) {
		ZALOG_ERROR("访问远程数据失败: zaData.aCurText");
		return 1;
	}

	buffTextSrc[sizeof(buffTextSrc) - 1] = 0;
	offset += _textAnalysisCN(buffText, buffTextSrc);

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
				_pScenaName, offset, buffStrVoiceId,
				buffText, buffTextJPFixed);
		}
		else {
			ZALOG_DEBUG("\nScena:%s,Offset:0x%06X,VoiceID:%s\n"
				"------------------------------------\n"
				"%s\n"
				"------------------------------------",
				_pScenaName, offset, buffStrVoiceId,
				buffText);
		}
	}//if (buffText[0] != 0)

	return 0;
}

