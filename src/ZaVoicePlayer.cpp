#include "ZaVoicePlayer.h"

#include "ZaConst.h"
#include "ZaLog.h"
#include "ZaConfig.h"
#include "ZaSound.h"
#include "ZaVoiceTable.h"
#include "ZaRemote.h"

#include <Windows.h>
#include <string>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

static int mode;
static int length_voiceid;

static int voiceIdWait;

#define BUFF_TEXT_SIZE 1024
static ZaVoiceTable zaVoiceTable;
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

static int ZaTextAnalysis(unsigned char *dst, const unsigned char* src) {
	int first = 0;
	while (*src < 0x20 || *src > 0xFF) {
		src++; first++;
	}
	if (zaConfigData.removeFwdCtrlCh) {
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
static void ZaTextAnalysisJP(const unsigned char* &buff) {
	if (zaConfigData.removeFwdCtrlCh) {
		while (*buff == '#') {
			++buff;
			while (*buff >= '0' && *buff <= '9') ++buff;
			++buff;
		}
	}
}

static char voiceidbuff[MAX(Z_LENGTH_VOICE_ID, A_LENGTH_VOICE_ID) + 1];
static const char* StrVoiceID(int voiceID) {
	if (voiceID == InValidVoiceId) {
		for (int i = 0; i < length_voiceid; ++i)
			voiceidbuff[i] = '-';
	}
	else {
		for (int i = length_voiceid - 1; i >= 0; --i) {
			voiceidbuff[i] = voiceID % 10 + '0';
			voiceID /= 10;
		}
	}
	voiceidbuff[length_voiceid] = 0;
	return voiceidbuff;
}

static bool ZaPlayVoice(int voiceID, std::string &filename) {
	if (voiceID == InValidVoiceId) return false;

	filename = mode == MODE_AO ?
		zaConfigData.ao_name_voice + StrVoiceID(voiceID) + '.' + zaConfigData.ao_ext_voice
		: zaConfigData.zero_name_voice + StrVoiceID(voiceID) + '.' + zaConfigData.zero_ext_voiceFile;

	std::string vfileFull = mode == MODE_AO ?
		zaConfigData.ao_dir_voice + '\\' + filename :
		zaConfigData.zero_dir_voice + '\\' + filename;
	
	return ZaSoundPlay(vfileFull);
}

static void ZaLoadNewVoiceTable(const char* scenaName) {
	std::string pathVtbl = mode == MODE_AO ? 
		zaConfigData.ao_dir_voiceTable + "\\" + scenaName + "." + zaConfigData.ao_ext_voiceTable :
		zaConfigData.zero_dir_voiceTable + "\\" + scenaName + "." + zaConfigData.zero_ext_voiceTable;
	zaVoiceTable.LoadTblFile(pathVtbl.c_str());
}

static int ZaVoicePlayerLoopMain()
{
	std::string voiceFileName;
	if (voiceIdWait != InValidVoiceId && ZaSoundStatus() == ZASOUND_STATUS_STOP) {
		if (ZaPlayVoice(voiceIdWait, voiceFileName)) {
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
			ZaLoadNewVoiceTable(scenaName);
			ZALOG_DEBUG("Voice Table Records：%d", zaVoiceTable.NumInfo());
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
				ZaLoadNewVoiceTable(pScenaName);
				ZALOG_DEBUG("Voice Table Records：%d", zaVoiceTable.NumInfo());
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
				ZaLoadNewVoiceTable(pScenaName);
				ZALOG_DEBUG("Voice Table Records：%d", zaVoiceTable.NumInfo());
			}
		}
		else {
			offset1 = zaData.aCurBlock - zaData.aScena;

			if (pScenaName != scenaName) {
				pScenaName = scenaName;

				ZALOG_DEBUG("Scena:%s, Loading Voice Table...", pScenaName);
				ZaLoadNewVoiceTable(pScenaName);
				ZALOG_DEBUG("Voice Table Records：%d", zaVoiceTable.NumInfo());
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
	offset += ZaTextAnalysis(buffText, buffTextSrc);

	if (buffText[0] != 0) {
		const VoiceInfo* pvinf = zaVoiceTable[offset];
		if (pvinf == nullptr) {
			pvinf = zaVoiceTable[offset + FAKE_OFFSET];
			if (pvinf != nullptr) offset += FAKE_OFFSET;
		}
		const VoiceInfo& vinf = pvinf == nullptr ? InvaildVoiceInfo : *pvinf;

		const unsigned char* buffTextJP = (const unsigned char*)vinf.jpText.c_str();
		ZaTextAnalysisJP(buffTextJP);

		if (buffTextJP[0] != 0) {
			ZALOG_DEBUG("\nScena:%s,Offset:0x%06X,VoiceID:%s\n"
				"------------------------------------\n"
				"%s\n"
				"------------------------------------\n"
				"%s\n"
				"------------------------------------",
				pScenaName, offset, StrVoiceID(vinf.voiceID),
				buffText, buffTextJP);
		}
		else {
			ZALOG_DEBUG("\nScena:%s,Offset:0x%06X,VoiceID:%s\n"
				"------------------------------------\n"
				"%s\n"
				"------------------------------------",
				pScenaName, offset, StrVoiceID(vinf.voiceID),
				buffText);
		}

		if (vinf.voiceID != InValidVoiceId) {
			if (offset < FAKE_OFFSET  || ZaSoundStatus() == ZASOUND_STATUS_STOP || voiceIdWait != InValidVoiceId) {
				if (ZaPlayVoice(vinf.voiceID, voiceFileName)) {
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

static int ZaVoicePlayerLoopAfterOneLoop() {
	zaData_old = zaData;
	return 0;
}

int ZaVoicePlayerInit(int mode) {
	::mode = mode;
	length_voiceid = mode == MODE_AO ? A_LENGTH_VOICE_ID : Z_LENGTH_VOICE_ID;
	memset(&zaData, 0, sizeof(zaData));
	memset(&zaData_old, 0, sizeof(zaData_old));
	pScenaName = NULL;
	scenaName[0] = 0;
	voiceIdWait = InValidVoiceId;

	return 0;
}

int ZaVoicePlayerLoopOne()
{
	int errc = 0;
	errc |= ZaVoicePlayerLoopMain();
	errc |= ZaVoicePlayerLoopAfterOneLoop();
	return errc;
}

