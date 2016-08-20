#include "ZaVoicePlayer.h"

#include "ZaConst.h"
#include "ZaLog.h"
#include "ZaConfig.h"
#include "ZaSound.h"
#include "ZaVoiceTable.h"
#include "ZaRemote.h"
#include "ZaIo.h"

#include <io.h>
#include <Windows.h>
#include <string>

static int voiceIdWait;

#define BUFF_TEXT_SIZE 1024
static ZaVoiceTablesGroup zaVoiceTablesGroup;
static const ZaVoiceTable *zaVoiceTable;
static ZAData zaData, zaData_old;
static char scenaName[MAX_SCENANAME_LENGTH + 1];
static char scenaName1[MAX_SCENANAME_LENGTH + 1];
static char scenaName2[MAX_SCENANAME_LENGTH + 1];
static char* pScenaName;
static unsigned offset1;

static unsigned char buffText[BUFF_TEXT_SIZE + 1];
static unsigned char buffTextSrc[BUFF_TEXT_SIZE + 1];

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
	while (*src < 0x20 || *src > 0xFF) {
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
static void TextAnalysisJP(const unsigned char* &buff) {
	if (zaConfig->General.RemoveFwdCtrlCh) {
		while (*buff == '#') {
			++buff;
			while (*buff >= '0' && *buff <= '9') ++buff;
			++buff;
		}
	}
}

static std::string StrVoiceID(int voiceID) {
	std::string strVoiceId;
	strVoiceId.resize(zaConfig->ActiveGame->VoiceIdLength);

	if (voiceID == InValidVoiceId) {
		for (int i = 0; i < zaConfig->ActiveGame->VoiceIdLength; ++i)
			strVoiceId[i] = '-';
	}
	else {
		for (int i = zaConfig->ActiveGame->VoiceIdLength - 1; i >= 0; --i) {
			strVoiceId[i] = voiceID % 10 + '0';
			voiceID /= 10;
		}
	}

	return strVoiceId;
}

static bool PlayVoice(int voiceID, std::string &filename) {
	if (voiceID == InValidVoiceId) return false;

	const std::string& dir = zaConfig->ActiveGame->VoiceDir;
	const std::string strVoiceID = StrVoiceID(voiceID);

	for (auto ext : zaConfig->ActiveGame->VoiceExt)
	{
		filename = zaConfig->ActiveGame->VoiceName + strVoiceID + '.' + ext;
		std::string filePath = dir + '\\' + filename;
		
		if (_access(filePath.c_str(), 4) == 0)
			return ZaSoundPlay(filePath);
	}

	return false;
}

static void LoadNewVoiceTable(const std::string& name) {
	zaVoiceTable = zaVoiceTablesGroup.GetVoiceTable(name);
	if (zaVoiceTable == nullptr)
		zaVoiceTable = &InvalidVoiceTable;
}
static void LoadAllVoiceTables() {
	std::vector<std::string> subs;
	
	const std::string& dir = zaConfig->ActiveGame->VtblDir;

	std::string searchName = "*." + zaConfig->ActiveGame->VtblExt;

	ZaGetSubFiles(dir, searchName, subs);

	for (auto sub : subs) {
		std::string scenaName = sub.substr(0, sub.rfind('.'));
		zaVoiceTablesGroup.AddVoiceTable(scenaName, dir + '\\' + sub);
	}
}

static int VoicePlayerLoopMain()
{
	std::string voiceFileName;
	if (voiceIdWait != InValidVoiceId && ZaSoundStatus() == ZASOUND_STATUS_STOP) {
		if (PlayVoice(voiceIdWait, voiceFileName)) {
			ZALOG("Playing %s ...", voiceFileName.c_str());
		}
		else {
			ZALOG_ERROR("Play %s Failed.", voiceFileName.c_str());
		}
		voiceIdWait = InValidVoiceId;
	}


	if (!ZaRemoteRead(rAddData, &zaData, sizeof(zaData))) {
		ZALOG_ERROR("访问远程数据失败: zaData");
		return 1;
	}

	if (zaData.cScena != zaData_old.cScena) {
		Sleep(100);
		unsigned addScenaName;
		if (!ZaRemoteRead(zaData.aScena + OFF_OFF_SCENANAME, &addScenaName, sizeof(addScenaName))
			|| !ZaRemoteRead(zaData.aScena + addScenaName, scenaName, sizeof(scenaName))) {
			ZALOG_ERROR("访问远程数据失败: zaData.aScena");
			return 1;
		}
		scenaName[sizeof(scenaName) - 1] = 0;
		if (CheckScenaName(scenaName)) {
			ZALOG_DEBUG("Scena:%s, Loading Voice Table...", scenaName);
			LoadNewVoiceTable(scenaName);
			ZALOG_DEBUG("Voice Table Records：%d", zaVoiceTable->Num());
		}
		else {
			scenaName[0] = 0;
		}

		zaData_old.cBlock = zaData_old.cText = 0;
		pScenaName = scenaName;
		offset1 = zaData.aCurBlock - zaData.aScena;
	}

	if (scenaName[0] == 0) return 0;
	 
	if (zaData.cBlock != zaData_old.cBlock) {
		zaData_old.cText = 0;
		if (zaData.aScena2 < zaData.aScena1) { 
			unsigned tmp = zaData.aScena1; 
			zaData.aScena1 = zaData.aScena2;
			zaData.aScena2 = tmp;
		}

		if (zaData.aScena2 > 0 && zaData.aScena2 < zaData.aCurBlock) {
			offset1 = zaData.aCurBlock - zaData.aScena2;
			if (pScenaName != scenaName2) {
				pScenaName = scenaName2;

				unsigned offScenaName2;
				if (!ZaRemoteRead(zaData.aScena2 + OFF_OFF_SCENANAME, &offScenaName2, sizeof(offScenaName2))
					|| !ZaRemoteRead(zaData.aScena2 + offScenaName2, scenaName2, sizeof(scenaName2))) {
					ZALOG_ERROR("访问远程数据失败: zaData.aScena2");
					return 1;
				}
				ZALOG_DEBUG("Scena:%s, Loading Voice Table...", pScenaName);
				LoadNewVoiceTable(pScenaName);
				ZALOG_DEBUG("Voice Table Records：%d", zaVoiceTable->Num());
			}
		}
		else if (zaData.aScena1 > 0 && zaData.aScena1 < zaData.aCurBlock) {
			offset1 = zaData.aCurBlock - zaData.aScena1;

			if (pScenaName != scenaName1) {
				pScenaName = scenaName1;

				unsigned offScenaName1;
				if (!ZaRemoteRead(zaData.aScena1 + OFF_OFF_SCENANAME, &offScenaName1, sizeof(offScenaName1))
					|| !ZaRemoteRead(zaData.aScena1 + offScenaName1, scenaName1, sizeof(scenaName1))) {
					ZALOG_ERROR("访问远程数据失败: zaData.aScena1");
					return 1;
				}
				ZALOG_DEBUG("Scena:%s, Loading Voice Table...", pScenaName);
				LoadNewVoiceTable(pScenaName);
				ZALOG_DEBUG("Voice Table Records：%d", zaVoiceTable->Num());
			}
		}
		else {
			offset1 = zaData.aCurBlock - zaData.aScena;

			if (pScenaName != scenaName) {
				pScenaName = scenaName;

				ZALOG_DEBUG("Scena:%s, Loading Voice Table...", pScenaName);
				LoadNewVoiceTable(pScenaName);
				ZALOG_DEBUG("Voice Table Records：%d", zaVoiceTable->Num());
			}
		}
	}

	if (zaData.cText == zaData_old.cText)
		return 0;

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
			if (pvinf != nullptr) offset += FAKE_OFFSET;
		}
		const VoiceInfo& vinf = pvinf == nullptr ? InvaildVoiceInfo : *pvinf;

		const unsigned char* buffTextJP = (const unsigned char*)vinf.jpText.c_str();
		TextAnalysisJP(buffTextJP);

		if (buffTextJP[0] != 0) {
			ZALOG_DEBUG("\nScena:%s,Offset:0x%06X,VoiceID:%s\n"
				"------------------------------------\n"
				"%s\n"
				"------------------------------------\n"
				"%s\n"
				"------------------------------------",
				pScenaName, offset, StrVoiceID(vinf.voiceID).c_str(),
				buffText, buffTextJP);
		}
		else {
			ZALOG_DEBUG("\nScena:%s,Offset:0x%06X,VoiceID:%s\n"
				"------------------------------------\n"
				"%s\n"
				"------------------------------------",
				pScenaName, offset, StrVoiceID(vinf.voiceID).c_str(),
				buffText);
		}

		if (vinf.voiceID != InValidVoiceId) {
			if (offset < FAKE_OFFSET  || ZaSoundStatus() == ZASOUND_STATUS_STOP || voiceIdWait != InValidVoiceId) {
				if (PlayVoice(vinf.voiceID, voiceFileName)) {
					ZALOG_DEBUG("Playing %s ...", voiceFileName.c_str());
				}
				else {
					ZALOG_ERROR("Play %s failed.", voiceFileName.c_str());
				}
				voiceIdWait = InValidVoiceId;
			}
			else {
				voiceIdWait = vinf.voiceID;
			}
		}//if (vinf.voiceID != InValidVoiceId)
	}//if (buffText[0] != 0)

	return 0;
}

static int VoicePlayerLoopAfterOneLoop() {
	zaData_old = zaData;
	return 0;
}

int ZaVoicePlayerInit() {
	ZaSoundInit(zaConfig->ActiveGame->Volume);
	
	memset(&zaData, 0, sizeof(zaData));
	memset(&zaData_old, 0, sizeof(zaData_old));
	pScenaName = NULL;
	scenaName[0] = 0;
	voiceIdWait = InValidVoiceId;

	ZALOG_DEBUG("加载语音表...");
	LoadAllVoiceTables();
	ZALOG_DEBUG("已加载的语音表数: %d", zaVoiceTablesGroup.Num());

	return 0;
}

int ZaVoicePlayerLoopOne()
{
	int errc = 0;
	errc |= VoicePlayerLoopMain();
	errc |= VoicePlayerLoopAfterOneLoop();
	return errc;
}

