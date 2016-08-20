#include "ZaScenaAnalyzer.h"

#include "ZaLog.h"
#include "ZaConfig.h"
#include "ZaVoiceTable.h"
#include "ZaRemote.h"
#include "ZaIo.h"

#include <string>
#include <vector>

static ZAData zaData;

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
	if (zaConfig->General.RemoveFwdCtrlCh) {
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
	if (zaConfig->General.RemoveFwdCtrlCh) {
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
	const std::string& dir = zaConfig->ActiveGame->VtblDir;
	std::string searchName = "*." + zaConfig->ActiveGame->VtblExt;
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

	int errc;
	zaData.aScena = raScena;
	errc = LoadScenaX(zaData.aScena, scenaName);
	if (errc) return errc;

	zaData.aScena1 = zaData.aScena2 = 0;

	if (scenaName[0] != 0) {
		ZALOG_DEBUG("Scena:%s, Loading Voice Table...", scenaName);
		LoadNewVoiceTable(scenaName);
		ZALOG_DEBUG("Voice Table Records：%d", zaVoiceTable->Num());

		pScenaName = scenaName;
		out_scenaName = scenaName;
	}

	return 0;
}
int ZaDetected_LoadScena1(unsigned raScena1, const char* &out_scenaName)
{
	out_scenaName = nullptr;

	int errc;
	if (zaData.aScena1 == 0) {
		zaData.aScena1 = raScena1;
		errc = LoadScenaX(zaData.aScena1, scenaName1);
		out_scenaName = scenaName1;
	}
	else {
		zaData.aScena2 = raScena1;
		errc = LoadScenaX(zaData.aScena2, scenaName2);
		out_scenaName = scenaName2;
	}

	if (errc) return errc;

	return 0;
}

struct ScenaData {
	unsigned aScenaX;
	char* pScneaNameX;

	bool operator<(const ScenaData& b) { return aScenaX < b.aScenaX; }
};

int ZaDetected_LoadBlock(unsigned raBlock, const char* &out_scenaName)
{
	out_scenaName = nullptr;
	if (scenaName[0] == 0) return 0;

	zaData.aFirstText = 0;
	zaData.aCurBlock = raBlock;

	ScenaData scenas[] = { 
		{ zaData.aScena, scenaName },
		{ zaData.aScena1, scenaName1 },
		{ zaData.aScena2, scenaName2 }
	};
	if (scenas[1] < scenas[2]) std::swap(scenas[1], scenas[2]);
	if (scenas[0] < scenas[1]) {
		std::swap(scenas[0], scenas[1]);
		if (scenas[1] < scenas[2]) std::swap(scenas[1], scenas[2]);
	}

	for (int i = 0; i <  sizeof(scenas) / sizeof(*scenas); ++i) {
		if (zaData.aCurBlock > scenas[i].aScenaX) {
			if (zaData.aCurBlock - scenas[i].aScenaX >= MAX_SCENA_SIZE) {
				zaData.aCurBlock = 0;
				return 0;
			}

			offset1 = zaData.aCurBlock - scenas[i].aScenaX;

			if (pScenaName != scenas[i].pScneaNameX) {
				pScenaName = scenas[i].pScneaNameX;

				ZALOG_DEBUG("Scena:%s, Loading Voice Table...", pScenaName);
				LoadNewVoiceTable(pScenaName);
				ZALOG_DEBUG("Voice Table Records：%d", zaVoiceTable->Num());
			}

			out_scenaName = scenas[i].pScneaNameX;
			break;
		}
	}

	return 0;
}
int ZaDetected_ShowText(unsigned raText, int & out_voiceID, bool & out_wait)
{
	out_voiceID = InValidVoiceId; out_wait = false;

	if (scenaName[0] == 0 || zaData.aCurBlock == 0) return 0;

	zaData.aCurText = raText;
	if (zaData.aFirstText == 0) zaData.aFirstText = raText;

	unsigned offset = offset1 + zaData.aCurText - zaData.aFirstText;
	if (offset > MAX_SCENA_SIZE)
		return 0;

	if (!ZaRemoteRead(zaData.aCurText, buffTextSrc, sizeof(buffTextSrc))) {
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

		if (buffTextJPFixed[0] != 0) {
			ZALOG_DEBUG("\nScena:%s,Offset:0x%06X,VoiceID:%s\n"
				"------------------------------------\n"
				"%s\n"
				"------------------------------------\n"
				"%s\n"
				"------------------------------------",
				pScenaName, offset, StrVoiceID(vinf.voiceID),
				buffText, buffTextJPFixed);
		}
		else {
			ZALOG_DEBUG("\nScena:%s,Offset:0x%06X,VoiceID:%s\n"
				"------------------------------------\n"
				"%s\n"
				"------------------------------------",
				pScenaName, offset, StrVoiceID(vinf.voiceID),
				buffText);
		}
	}//if (buffText[0] != 0)

	return 0;
}

static char strIdBuff[MAX_LENGTH_VOICE_ID + 1];
const char* StrVoiceID(int voiceID) {
	if (voiceID == InValidVoiceId) {
		for (int i = 0; i < zaConfig->ActiveGame->VoiceIdLength; ++i)
			strIdBuff[i] = '-';
	}
	else {
		for (int i = zaConfig->ActiveGame->VoiceIdLength - 1; i >= 0; --i) {
			strIdBuff[i] = voiceID % 10 + '0';
			voiceID /= 10;
		}
	}
	strIdBuff[zaConfig->ActiveGame->VoiceIdLength] = 0;

	return strIdBuff;
}


