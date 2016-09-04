#include "ZaScenaAnalyzer.h"

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
static unsigned _raBlock;
static unsigned _raFirstText;
static unsigned _raCurText;

static ZaVoiceTablesGroup _zaVoiceTablesGroup;
static const ZaVoiceTable *_zaVoiceTable;

#define NAME_BUFF_SIZE (MAX_SCENANAME_LENGTH + 1)
static char _scenaName[NAME_BUFF_SIZE];
static char _scenaName1[NAME_BUFF_SIZE];
static char _scenaName2[NAME_BUFF_SIZE];

static char* _pScenaName;
static unsigned _offset1;

#define BUFF_TEXT_SIZE 1024
static unsigned char buffText[BUFF_TEXT_SIZE];
static unsigned char buffTextSrc[BUFF_TEXT_SIZE];

static bool CheckScenaName(const char *_scenaName) {
	int count = 0;
	while (*_scenaName != 0) {
		if (*_scenaName < 0x20 || *_scenaName >= 0x80) return false;
		++_scenaName;
		++count;
		if (count > MAX_SCENANAME_LENGTH) break;
	}
	return count >= MIN_SCENANAME_LENGTH && count <= MAX_SCENANAME_LENGTH;
}
static int TextAnalysisCN(unsigned char *dst, const unsigned char* src) {
	int first = 0;
	while (*src < 0x20 || *src == 0xFF) {
		src++; first++;
	}
	if (g_zaConfig->General.RemoveFwdCtrlCh) {
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
static const unsigned char* TextAnalysisJP(const unsigned char* buff) {
	const unsigned char* abuff = buff;
	if (g_zaConfig->General.RemoveFwdCtrlCh) {
		while (*abuff == '#') {
			++abuff;
			while (*abuff >= '0' && *abuff <= '9') ++abuff;
			++abuff;
		}
	}
	return abuff;
}

static void LoadNewVoiceTable(const std::string& name) {
	_zaVoiceTable = _zaVoiceTablesGroup.GetVoiceTable(name);
	if (_zaVoiceTable == nullptr)
		_zaVoiceTable = &InvalidVoiceTable;
}

static void ClearAllVoiceTable() {
	_zaVoiceTablesGroup.Clear();
}
static void ReLoadAllVoiceTables() {
	ClearAllVoiceTable();

	std::vector<std::string> subs;
	const std::string& dir = g_zaConfig->ActiveGame->VtblDir;
	std::string searchName = "*." + g_zaConfig->ActiveGame->VtblExt;
	ZaGetSubFiles(dir, searchName, subs);

	for (auto sub : subs) {
		std::string _scenaName = sub.substr(0, sub.rfind('.'));
		_zaVoiceTablesGroup.AddVoiceTable(_scenaName, dir + '\\' + sub);
	}
}

int ZaScenaAnalyzerInit()
{
	_pScenaName = NULL;
	_scenaName[0] = 0;
	_zaVoiceTable = &InvalidVoiceTable;

	ZALOG_DEBUG("加载语音表...");
	ReLoadAllVoiceTables();
	ZALOG_DEBUG("已加载的语音表数: %d", _zaVoiceTablesGroup.Num());

	return 0;
}
int ZaScenaAnalyzerEnd()
{
	ClearAllVoiceTable();

	_pScenaName = NULL;
	_scenaName[0] = 0;
	_zaVoiceTable = &InvalidVoiceTable;

	return 0;
}

static int LoadScenaX(unsigned raScena, char* nameBuff) {
	unsigned addScenaName;
	if (!ZaRemoteRead(raScena + OFF_OFF_SCENANAME, &addScenaName, sizeof(addScenaName))
		|| !ZaRemoteRead(raScena + addScenaName, nameBuff, NAME_BUFF_SIZE)) {
		ZALOG_ERROR("访问远程数据失败: zaData.aScena");
		return 1;
	}
	nameBuff[NAME_BUFF_SIZE - 1] = 0;
	if (!CheckScenaName(nameBuff)) {
		nameBuff[0] = 0;
	}

	return 0;
}

int ZaDetected_LoadScena(unsigned raScena, const char* &out_scenaName)
{
	out_scenaName = nullptr;
	_pScenaName = nullptr;
	memset(_scenas, 0, sizeof(_scenas));

	int errc;
	errc = LoadScenaX(raScena, _scenaName);
	if (errc) return errc;

	if (_scenaName[0] != 0) {

		_scenas[0].aScenaX = raScena;
		_scenas[0].pScneaNameX = _scenaName;

		out_scenaName = _scenaName;
	}

	return 0;
}
int ZaDetected_LoadScena1(unsigned raScena1, const char* &out_scenaName)
{
	out_scenaName = nullptr;

	int errc;
	if (_scenas[1].aScenaX == 0) {
		errc = LoadScenaX(raScena1, _scenaName1);
		if (errc) return errc;

		_scenas[1].aScenaX = raScena1;
		_scenas[1].pScneaNameX = _scenaName1;

		if (_scenas[0] < _scenas[1]) std::swap(_scenas[0], _scenas[1]);

		out_scenaName = _scenaName1;
	}
	else {
		errc = LoadScenaX(raScena1, _scenaName2);
		if (errc) return errc;

		_scenas[2].aScenaX = raScena1;
		_scenas[2].pScneaNameX = _scenaName2;

		if (_scenas[1] < _scenas[2]) {
			std::swap(_scenas[1], _scenas[2]);
			if (_scenas[0] < _scenas[1]) std::swap(_scenas[0], _scenas[1]);
		}

		out_scenaName = _scenaName2;
	}

	return 0;
}

int ZaDetected_LoadBlock(unsigned raBlock, const char* &out_scenaName)
{
	out_scenaName = nullptr;
	if (_scenaName[0] == 0) return 0;

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
				LoadNewVoiceTable(_pScenaName);
				ZALOG_DEBUG("Voice Table Records：%d", _zaVoiceTable->Num());
			}

			out_scenaName = _scenas[i].pScneaNameX;
			break;
		}
	}

	return 0;
}
int ZaDetected_ShowText(unsigned raText, int & out_voiceID, bool & out_wait)
{
	out_voiceID = InValidVoiceId; out_wait = false;

	if (_scenaName[0] == 0 || _raBlock == 0) return 0;

	_raCurText = raText;
	if (_raFirstText == 0) _raFirstText = raText;

	unsigned offset = _offset1 + _raCurText - _raFirstText;
	if (offset > MAX_SCENA_SIZE)
		return 0;

	if (!ZaRemoteRead(_raCurText, buffTextSrc, sizeof(buffTextSrc))) {
		ZALOG_ERROR("访问远程数据失败: zaData.aCurText");
		return 1;
	}

	buffTextSrc[sizeof(buffTextSrc) - 1] = 0;
	offset += TextAnalysisCN(buffText, buffTextSrc);

	if (buffText[0] != 0) {
		const VoiceInfo* pvinf = _zaVoiceTable->GetVoiceInfo(offset);
		if (pvinf == nullptr) {
			pvinf = _zaVoiceTable->GetVoiceInfo(offset + FAKE_OFFSET);
			if (pvinf != nullptr) {
				offset += FAKE_OFFSET;
				out_wait = true;
			}
		}
		const VoiceInfo& vinf = pvinf == nullptr ? InvaildVoiceInfo : *pvinf;
		out_voiceID = vinf.voiceID;

		const unsigned char* buffTextJP = (const unsigned char*)vinf.jpText.c_str();
		const unsigned char* buffTextJPFixed = TextAnalysisJP(buffTextJP);
		char buffStrVoiceId[MAX_LENGTH_VOICE_ID + 1];
		GetStrVoiceID(vinf.voiceID, buffStrVoiceId);

		if (buffTextJPFixed[0] != 0) {
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

int GetStrVoiceID(int voiceID, char* buff_strVoiceId) {
	if (voiceID == InValidVoiceId) {
		for (int i = 0; i < g_zaConfig->ActiveGame->VoiceIdLength; ++i)
			buff_strVoiceId[i] = '-';
	}
	else {
		for (int i = g_zaConfig->ActiveGame->VoiceIdLength - 1; i >= 0; --i) {
			buff_strVoiceId[i] = voiceID % 10 + '0';
			voiceID /= 10;
		}
	}
	buff_strVoiceId[g_zaConfig->ActiveGame->VoiceIdLength] = 0;

	return g_zaConfig->ActiveGame->VoiceIdLength;
}


