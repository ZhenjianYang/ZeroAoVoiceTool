#include "ZaConfig.h"
#include "ZaConst.h"

#include <fstream>
#include <iostream>

using namespace std;

static ZaConfig struZaConfig;
static ZaConfig* ptrZaConfig = &struZaConfig;

const ZaConfig* const &zaConfig = ptrZaConfig;

static void ZaConfigSetDefault();

static int Compare(const char* buff, const char* item) {
	int p;
	for (p = 0; item[p] != 0; ++p) {
		if (item[p] != buff[p])
			return 0;
	}
	return p;
}

static bool SetStrVaule(string& value, const char* buff) {
	while (*buff == 0x20 || *buff == '\t') ++buff;
	if (*buff != '=') return false;
	
	++buff;
	while (*buff == 0x20 || *buff == '\t') ++buff;

	if (*buff == 0) return true;
	value = buff;
	int size = value.size();
	while (value[size - 1] == 0x20 || value[size - 1] == '\t') --size;
	value.resize(size);
	return true;
}
static bool SetVStrVaule(vector<string>& value, const char* buff) {
	while (*buff == 0x20 || *buff == '\t') ++buff;
	if (*buff != '=') return false;

	++buff;
	while (*buff == 0x20 || *buff == '\t') ++buff;
	if (*buff == 0) return true;

	value.clear();
	while (*buff != 0)
	{
		int len = 0;
		while (buff[len] != 0x20 && buff[len] != '\t' && buff[len] != 0) ++len;

		if(len > 0) value.push_back(string(buff, len));
		buff += len;

		while (*buff == 0x20 || *buff == '\t') ++buff;
	}
	return true;
}
static bool SetIntVaule(int& value, const char* buff) {
	while (*buff == 0x20 || *buff == '\t') ++buff;
	if (*buff != '=') return false;

	++buff;
	while (*buff == 0x20 || *buff == '\t') ++buff;

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

#define BUFF_SIZE 1204
void ZaConfigLoad(const char * configFile)
{
	ZaConfigSetDefault();

	ifstream ifs(configFile);
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

		while (*pb == ' ' || *pb == '\t') ++pb;

		bool failed = false;
		int p = 0;
		if (Compare(pb, STR_general)) gameID = GAMEID_AO | GAMEID_ZERO;
		else if (Compare(pb, STR_Ao)) gameID = GAMEID_AO;
		else if (Compare(pb, STR_Zero)) gameID = GAMEID_ZERO;
		else if (p = Compare(pb, STR_VoiceFileDirectory)) {
			if(gameID & GAMEID_AO) failed = !SetStrVaule(struZaConfig.Ao.VoiceDir, pb + p);
			if(gameID & GAMEID_ZERO) failed = !SetStrVaule(struZaConfig.Zero.VoiceDir, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceFileExtension)) {
			if (gameID & GAMEID_AO) failed = !SetVStrVaule(struZaConfig.Ao.VoiceExt, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetVStrVaule(struZaConfig.Zero.VoiceExt, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceTableFileDirectory)) {
			if (gameID & GAMEID_AO) failed = !SetStrVaule(struZaConfig.Ao.VtblDir, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetStrVaule(struZaConfig.Zero.VtblDir, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceTableFileExtension)) {
			if (gameID & GAMEID_AO) failed = !SetStrVaule(struZaConfig.Ao.VtblExt, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetStrVaule(struZaConfig.Zero.VtblExt, pb + p);
		}
		else if (p = Compare(pb, STR_DisableOriginalVoice)) {
			if (gameID & GAMEID_AO) failed = !SetIntVaule(struZaConfig.Ao.DisableOriginalVoice, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetIntVaule(struZaConfig.Zero.DisableOriginalVoice, pb + p);
		}
		else if (p = Compare(pb, STR_Volume)) {
			if (gameID & GAMEID_AO) failed = !SetIntVaule(struZaConfig.Ao.Volume, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetIntVaule(struZaConfig.Zero.Volume, pb + p);
		}
		else if (p = Compare(pb, STR_OpenDebugLog)) {
			failed = !SetIntVaule(struZaConfig.General.OpenDebugLog, pb + p);
		}
		else if (p = Compare(pb, STR_UseLogFile)) {
			failed = !SetIntVaule(struZaConfig.General.UseLogFile, pb + p);
		}
		else failed = true;

		if (failed) {
			cout << "Config File Bad Line " << cnt << endl;
		}
	}
}

void ZaConfigSetActive(int gameID)
{
	switch (gameID)
	{
	case GAMEID_ZERO:
		ptrZaConfig->ActiveGameID = GAMEID_ZERO;
		ptrZaConfig->ActiveGame = &ptrZaConfig->Zero;
		break;
	case GAMEID_AO:
		ptrZaConfig->ActiveGameID = GAMEID_AO;
		ptrZaConfig->ActiveGame = &ptrZaConfig->Ao;
		break;
	default:
		break;
	}
	
}

void ZaConfigSetDefault() {
	struZaConfig.ActiveGameID = GAMEID_INVALID;
	struZaConfig.ActiveGame = nullptr;

	struZaConfig.General.OpenDebugLog = DFT_DEBUGLOG;
	struZaConfig.General.UseLogFile = DFT_USELOGFILE;

	struZaConfig.General.SleepTime = DFT_SLEEP_TIME;
	struZaConfig.General.RemoveFwdCtrlCh = DFT_RMFWDCTRLCH;
	struZaConfig.General.Mode = MODE_AUTO;


	struZaConfig.Zero.VoiceDir = Z_DFT_VOICE_DIR;
	struZaConfig.Zero.VoiceExt = Z_DFT_VOICE_EXT;
	struZaConfig.Zero.VoiceName = Z_DFT_VOICE_NAME;

	struZaConfig.Zero.VtblExt = Z_DFT_VOICETABLE_EXT;
	struZaConfig.Zero.VtblDir = Z_DFT_VOICETABLE_DIR;

	struZaConfig.Zero.Volume = Z_DFT_VOLUME;
	struZaConfig.Zero.DisableOriginalVoice = Z_DFT_DISABLE_ORIVOICE;

	struZaConfig.Zero.VoiceIdLength = Z_LENGTH_VOICE_ID;


	struZaConfig.Ao.VoiceDir = A_DFT_VOICE_DIR;
	struZaConfig.Ao.VoiceExt = A_DFT_VOICE_EXT;
	struZaConfig.Ao.VoiceName = A_DFT_VOICE_NAME;

	struZaConfig.Ao.VtblExt = A_DFT_VOICETABLE_EXT;
	struZaConfig.Ao.VtblDir = A_DFT_VOICETABLE_DIR;

	struZaConfig.Ao.Volume = A_DFT_VOLUME;
	struZaConfig.Ao.DisableOriginalVoice = A_DFT_DISABLE_ORIVOICE;

	struZaConfig.Ao.VoiceIdLength = A_LENGTH_VOICE_ID;
}
