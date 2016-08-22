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
static ScenaData s_scenas[3];
static unsigned s_raBlock;
static unsigned s_raFirstText;
static unsigned s_raCurText;

static ZaVoiceTablesGroup zaVoiceTablesGroup;
static const ZaVoiceTable *zaVoiceTable;

#define NAME_BUFF_SIZE (MAX_SCENANAME_LENGTH + 1)
static char scenaName[NAME_BUFF_SIZE];
static char scenaName1[NAME_BUFF_SIZE];
static char scenaName2[NAME_BUFF_SIZE];

static char* pScenaName;
static unsigned offset1;

#define BUFF_TEXT_SIZE 1024
static unsigned char buffText[BUFF_TEXT_SIZE];
static unsigned char buffTextSrc[BUFF_TEXT_SIZE];

static bool CheckScenaName(const char *scenaName) {
	int count = 0;
	while (*scenaName != 0) {
		if (*scenaName < 0x20 || *scenaName >= 0x80) return false;
		++scenaName;
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
	zaVoiceTable = zaVoiceTablesGroup.GetVoiceTable(name);
	if (zaVoiceTable == nullptr)
		zaVoiceTable = &InvalidVoiceTable;
}
static void ReLoadAllVoiceTables() {
	zaVoiceTablesGroup.Clear();

	std::vector<std::string> subs;
	const std::string& dir = g_zaConfig->ActiveGame->VtblDir;
	std::string searchName = "*." + g_zaConfig->ActiveGame->VtblExt;
	ZaGetSubFiles(dir, searchName, subs);

	for (auto sub : subs) {
		std::string scenaName = sub.substr(0, sub.rfind('.'));
		zaVoiceTablesGroup.AddVoiceTable(scenaName, dir + '\\' + sub);
	}
}

int ZaScenaAnalyzerInit()
{
	pScenaName = NULL;
	scenaName[0] = 0;
	zaVoiceTable = &InvalidVoiceTable;

	ZALOG_DEBUG("加载语音表...");
	ReLoadAllVoiceTables();
	ZALOG_DEBUG("已加载的语音表数: %d", zaVoiceTablesGroup.Num());

	return 0;
}
int ZaScenaAnalyzerFinish()
{
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
	pScenaName = nullptr;
	memset(s_scenas, 0, sizeof(s_scenas));

	int errc;
	errc = LoadScenaX(raScena, scenaName);
	if (errc) return errc;

	if (scenaName[0] != 0) {

		s_scenas[0].aScenaX = raScena;
		s_scenas[0].pScneaNameX = scenaName;

		out_scenaName = scenaName;
	}

	return 0;
}
int ZaDetected_LoadScena1(unsigned raScena1, const char* &out_scenaName)
{
	out_scenaName = nullptr;

	int errc;
	if (s_scenas[1].aScenaX == 0) {
		errc = LoadScenaX(raScena1, scenaName1);
		if (errc) return errc;

		s_scenas[1].aScenaX = raScena1;
		s_scenas[1].pScneaNameX = scenaName1;

		if (s_scenas[0] < s_scenas[1]) std::swap(s_scenas[0], s_scenas[1]);

		out_scenaName = scenaName1;
	}
	else {
		errc = LoadScenaX(raScena1, scenaName2);
		if (errc) return errc;

		s_scenas[2].aScenaX = raScena1;
		s_scenas[2].pScneaNameX = scenaName2;

		if (s_scenas[1] < s_scenas[2]) {
			std::swap(s_scenas[1], s_scenas[2]);
			if (s_scenas[0] < s_scenas[1]) std::swap(s_scenas[0], s_scenas[1]);
		}

		out_scenaName = scenaName2;
	}

	return 0;
}

int ZaDetected_LoadBlock(unsigned raBlock, const char* &out_scenaName)
{
	out_scenaName = nullptr;
	if (scenaName[0] == 0) return 0;

	s_raFirstText = 0;
	s_raBlock = raBlock;

	for (int i = 0; i <  sizeof(s_scenas) / sizeof(*s_scenas); ++i) {
		if (s_raBlock > s_scenas[i].aScenaX) {
			if (s_raBlock - s_scenas[i].aScenaX >= MAX_SCENA_SIZE) {
				s_raBlock = 0;
				return 0;
			}

			offset1 = s_raBlock - s_scenas[i].aScenaX;

			if (pScenaName != s_scenas[i].pScneaNameX) {
				pScenaName = s_scenas[i].pScneaNameX;

				ZALOG_DEBUG("Scena:%s, Loading Voice Table...", pScenaName);
				LoadNewVoiceTable(pScenaName);
				ZALOG_DEBUG("Voice Table Records：%d", zaVoiceTable->Num());
			}

			out_scenaName = s_scenas[i].pScneaNameX;
			break;
		}
	}

	return 0;
}
int ZaDetected_ShowText(unsigned raText, int & out_voiceID, bool & out_wait)
{
	out_voiceID = InValidVoiceId; out_wait = false;

	if (scenaName[0] == 0 || s_raBlock == 0) return 0;

	s_raCurText = raText;
	if (s_raFirstText == 0) s_raFirstText = raText;

	unsigned offset = offset1 + s_raCurText - s_raFirstText;
	if (offset > MAX_SCENA_SIZE)
		return 0;

	if (!ZaRemoteRead(s_raCurText, buffTextSrc, sizeof(buffTextSrc))) {
		ZALOG_ERROR("访问远程数据失败: zaData.aCurText");
		return 1;
	}

	buffTextSrc[sizeof(buffTextSrc) - 1] = 0;
	offset += TextAnalysisCN(buffText, buffTextSrc);

	if (buffText[0] != 0) {
		const VoiceInfo* pvinf = zaVoiceTable->GetVoiceInfo(offset);
		if (pvinf == nullptr) {
			pvinf = zaVoiceTable->GetVoiceInfo(offset + FAKE_OFFSET);
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
				pScenaName, offset, buffStrVoiceId,
				buffText, buffTextJPFixed);
		}
		else {
			ZALOG_DEBUG("\nScena:%s,Offset:0x%06X,VoiceID:%s\n"
				"------------------------------------\n"
				"%s\n"
				"------------------------------------",
				pScenaName, offset, buffStrVoiceId,
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


