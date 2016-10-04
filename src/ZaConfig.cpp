#include "ZaConfig.h"
#include "ZaConst.h"

#include <fstream>
#include <iostream>

using namespace std;

#define FailReturn(condition, dobefore, ret) if(!condition) { dobefore; return ret; }

static Za::Config::ConfigData _mainConfig;
static Za::Config::ConfigData* _ptrConfig = &_mainConfig;

const Za::Config::ConfigData* const Za::Config::MainConfig = _ptrConfig;

#define SKIPSPACE(p, next) while ((p) == ' ' || (p) == '\t') next;
#define SKIPSPACE_PTR(p) SKIPSPACE(*(p), ++(p))
#define SKIPSPACE_INDEX(p, index) SKIPSPACE((p)[index], ++(index))

static int Compare(const char* buff, const char* item, bool checkEqual = true) {
	int p;
	for (p = 0; item[p] != 0; ++p) {
		if (item[p] != buff[p])
			return 0;
	}
	SKIPSPACE_INDEX(buff, p);

	if (checkEqual) {
		if (buff[p] != CH_Equal) return 0;
		++p;
		SKIPSPACE_INDEX(buff, p);
	}
	else {
		if (buff[p] != 0) return 0;
	}
	return p;
}

static bool SetVaule(string& value, const char* buff) {
	if (*buff == 0) return true;

	value = buff;
	SKIPSPACE(value.back(), value.pop_back());
	return true;
}
static bool SetVaule(vector<string>& value, const char* buff) {
	if (*buff == 0) return true;

	value.clear();
	while (*buff != 0)
	{
		int len = 0;
		while (buff[len] != 0x20 && buff[len] != '\t' && buff[len] != 0) ++len;

		if(len > 0) value.push_back(string(buff, len));
		buff += len;

		SKIPSPACE_PTR(buff);
	}
	return true;
}
static bool SetVaule(int& value, const char* buff) {
	if (*buff == 0) return true;

	int t = 0;
	while (*buff != 0) {
		t *= 10;
		if (*buff < '0' || *buff > '9') return false;
		t += *buff - '0';
		++buff;
	}
	value = t;
	return true;
}

#define BUFF_SIZE 1024

bool Za::Config::LoadFromFile(const char* configFile, ConfigData* pConfigDo /*= nullptr*/) {
	if (pConfigDo == nullptr) pConfigDo = _ptrConfig;
	LoadDefault(pConfigDo);

	ifstream ifs(configFile); if (!ifs) return false;

	char buff[BUFF_SIZE + 1];
	int gameID = GAMEID_AO | GAMEID_ZERO;

	int cnt = 0;
	bool firstline = true;
	while (ifs.getline(buff, BUFF_SIZE))
	{
		++cnt;
		char *pb = buff;

		if (firstline) {
			firstline = false;
			if (pb[0] == (char)0xEF && pb[1] == (char)0xBB && pb[2] == (char)0xBF)
				pb += 3;
		}
		if (pb[0] == 0 || pb[0] == '#')
			continue;

		SKIPSPACE_PTR(pb);

		bool failed = false;
		int p = 0;
		if (Compare(pb, STR_general, false)) gameID = GAMEID_AO | GAMEID_ZERO;
		else if (Compare(pb, STR_Ao, false)) gameID = GAMEID_AO;
		else if (Compare(pb, STR_Zero, false)) gameID = GAMEID_ZERO;
		else if (p = Compare(pb, STR_VoiceFileDirectory)) {
			if (gameID & GAMEID_AO) failed = !SetVaule(pConfigDo->m_ao.VoiceDir, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetVaule(pConfigDo->m_zero.VoiceDir, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceFileExtension)) {
			if (gameID & GAMEID_AO) failed = !SetVaule(pConfigDo->m_ao.VoiceExt, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetVaule(pConfigDo->m_zero.VoiceExt, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceTableFileDirectory)) {
			if (gameID & GAMEID_AO) failed = !SetVaule(pConfigDo->m_ao.VoiceTableDir, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetVaule(pConfigDo->m_zero.VoiceTableDir, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceTableFileExtension)) {
			if (gameID & GAMEID_AO) failed = !SetVaule(pConfigDo->m_ao.VoiceTableExt, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetVaule(pConfigDo->m_zero.VoiceTableExt, pb + p);
		}
		else if (p = Compare(pb, STR_DisableOriginalVoice)) {
			if (gameID & GAMEID_AO) failed = !SetVaule(pConfigDo->m_ao.DisableOriginalVoice, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetVaule(pConfigDo->m_zero.DisableOriginalVoice, pb + p);
		}
		else if (p = Compare(pb, STR_Volume)) {
			if (gameID & GAMEID_AO) failed = !SetVaule(pConfigDo->m_ao.Volume, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetVaule(pConfigDo->m_zero.Volume, pb + p);
		}
		else if (p = Compare(pb, STR_OpenDebugLog)) {
			failed = !SetVaule(pConfigDo->m_general.OpenDebugLog, pb + p);
		}
		else if (p = Compare(pb, STR_UseLogFile)) {
			failed = !SetVaule(pConfigDo->m_general.UseLogFile, pb + p);
		}
		else if (p = Compare(pb, STR_AutoStart)) {
			failed = !SetVaule(pConfigDo->m_general.AutoStart, pb + p);
		}
		else failed = true;

		if (failed) {
			cerr << "Config File Bad Line " << cnt << endl;
		}
	}

	return true;
}

bool Za::Config::SaveToFile(const char* configFile, ConfigData* pConfigDo /*= nullptr*/) {
	if (pConfigDo == nullptr) pConfigDo = _ptrConfig;

	ofstream ofs(configFile); if (!ofs) return false;

	ofs << STR_general << '\n';
	ofs << STR_OpenDebugLog << " = " << pConfigDo->General->OpenDebugLog << '\n';
	ofs << STR_UseLogFile << " = " << pConfigDo->General->UseLogFile << '\n';
	ofs << STR_AutoStart << " = " << pConfigDo->General->AutoStart << '\n';
	ofs << '\n';

	string tmp;
	if (pConfigDo->Ao->VoiceExt.size() > 0) tmp = pConfigDo->Ao->VoiceExt[0];
	for (int i = 1; i < (int)pConfigDo->Ao->VoiceExt.size(); ++i) tmp.push_back(' '), tmp += pConfigDo->Ao->VoiceExt[i];

	ofs << STR_Ao << '\n';
	ofs << STR_VoiceFileDirectory << " = " << pConfigDo->Ao->VoiceDir << '\n';
	ofs << STR_VoiceFileExtension << " = " << tmp << '\n';
	ofs << STR_VoiceTableFileDirectory << " = " << pConfigDo->Ao->VoiceTableDir << '\n';
	ofs << STR_VoiceTableFileExtension << " = " << pConfigDo->Ao->VoiceTableExt << '\n';
	ofs << STR_DisableOriginalVoice << " = " << pConfigDo->Ao->DisableOriginalVoice << '\n';
	ofs << STR_Volume << " = " << pConfigDo->Ao->Volume << '\n';
	ofs << '\n';

	tmp.clear();
	if (pConfigDo->Zero->VoiceExt.size() > 0) tmp = pConfigDo->Zero->VoiceExt[0];
	for (int i = 1; i < (int)pConfigDo->Zero->VoiceExt.size(); ++i) tmp.push_back(' '), tmp += pConfigDo->Zero->VoiceExt[i];

	ofs << STR_Zero << '\n';
	ofs << STR_VoiceFileDirectory << " = " << pConfigDo->Zero->VoiceDir << '\n';
	ofs << STR_VoiceFileExtension << " = " << tmp << '\n';
	ofs << STR_VoiceTableFileDirectory << " = " << pConfigDo->Zero->VoiceTableDir << '\n';
	ofs << STR_VoiceTableFileExtension << " = " << pConfigDo->Zero->VoiceTableExt << '\n';
    //ofs << STR_DisableOriginalVoice << " = " << _struZaConfig.Zero->DisableOriginalVoice << '\n';
	ofs << STR_Volume << " = " << pConfigDo->Zero->Volume << '\n';
	ofs << '\n';

	if (!ofs) { ofs.close(); return false; }

	ofs.close();
	return true;
}

void Za::Config::SetActiveGame(int gameId, ConfigData* pConfigDo /*= nullptr*/) {
	if (pConfigDo == nullptr) pConfigDo = _ptrConfig;

	switch (gameId)
	{
	case GAMEID_ZERO:
		pConfigDo->m_ActiveGameID = GAMEID_ZERO;
		pConfigDo->m_pActiveGame = &pConfigDo->m_zero;
		break;
	case GAMEID_AO:
		pConfigDo->m_ActiveGameID = GAMEID_AO;
		pConfigDo->m_pActiveGame = &pConfigDo->m_ao;
		break;
	default:
		break;
	}
}

void Za::Config::LoadDefault(ConfigData* pConfigDo /*= nullptr*/) {
	if (pConfigDo == nullptr) pConfigDo = _ptrConfig;

	pConfigDo->m_ActiveGameID = GAMEID_INVALID;
	pConfigDo->m_pActiveGame = nullptr;

	/////////////////////////////////////////////

	pConfigDo->m_general.OpenDebugLog = DFT_DEBUGLOG;
	pConfigDo->m_general.UseLogFile = DFT_USELOGFILE;

	pConfigDo->m_general.SleepTime = DFT_SLEEP_TIME;
	pConfigDo->m_general.RemoveFwdCtrlCh = DFT_RMFWDCTRLCH;
	pConfigDo->m_general.Mode = MODE_AUTO;

	pConfigDo->m_general.AutoStart = DFT_AUTOSTART;

	/////////////////////////////////////////////

	pConfigDo->m_zero.VoiceDir = Z_DFT_VOICE_DIR;
	pConfigDo->m_zero.VoiceExt = Z_DFT_VOICE_EXT;
	pConfigDo->m_zero.VoiceName = Z_DFT_VOICE_NAME;

	pConfigDo->m_zero.VoiceTableExt = Z_DFT_VOICETABLE_EXT;
	pConfigDo->m_zero.VoiceTableDir = Z_DFT_VOICETABLE_DIR;

	pConfigDo->m_zero.Volume = Z_DFT_VOLUME;
	pConfigDo->m_zero.DisableOriginalVoice = Z_DFT_DISABLE_ORIVOICE;

	pConfigDo->m_zero.VoiceIdLength = Z_LENGTH_VOICE_ID;

	/////////////////////////////////////////////

	pConfigDo->m_ao.VoiceDir = A_DFT_VOICE_DIR;
	pConfigDo->m_ao.VoiceExt = A_DFT_VOICE_EXT;
	pConfigDo->m_ao.VoiceName = A_DFT_VOICE_NAME;

	pConfigDo->m_ao.VoiceTableExt = A_DFT_VOICETABLE_EXT;
	pConfigDo->m_ao.VoiceTableDir = A_DFT_VOICETABLE_DIR;

	pConfigDo->m_ao.Volume = A_DFT_VOLUME;
	pConfigDo->m_ao.DisableOriginalVoice = A_DFT_DISABLE_ORIVOICE;

	pConfigDo->m_ao.VoiceIdLength = A_LENGTH_VOICE_ID;
}

void Za::Config::Set(const ConfigData::GameConfig& gameConfig, int gameId, ConfigData* pConfigDo /*= nullptr*/) {
	if (pConfigDo == nullptr) pConfigDo = _ptrConfig;

	switch (gameId)
	{
	case GAMEID_ZERO:
		pConfigDo->m_zero = gameConfig;
		break;
	case GAMEID_AO:
		pConfigDo->m_ao = gameConfig;
		break;
	default:
		break;
	}
}
void Za::Config::Set(const ConfigData::GeneralConfig& generalConfig, ConfigData* pConfigDo /*= nullptr*/) {
	if (pConfigDo == nullptr) pConfigDo = _ptrConfig;

	pConfigDo->m_general = generalConfig;
}

