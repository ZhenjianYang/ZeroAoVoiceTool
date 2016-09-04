#include "ZaConfig.h"
#include "ZaConst.h"

#include <fstream>
#include <iostream>

using namespace std;

static ZaConfig _struZaConfig;
static ZaConfig* _ptrZaConfig = &_struZaConfig;

const ZaConfig* const &g_zaConfig = _ptrZaConfig;

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

#define BUFF_SIZE 1024
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
			if(gameID & GAMEID_AO) failed = !SetStrVaule(_struZaConfig.Ao.VoiceDir, pb + p);
			if(gameID & GAMEID_ZERO) failed = !SetStrVaule(_struZaConfig.Zero.VoiceDir, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceFileExtension)) {
			if (gameID & GAMEID_AO) failed = !SetVStrVaule(_struZaConfig.Ao.VoiceExt, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetVStrVaule(_struZaConfig.Zero.VoiceExt, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceTableFileDirectory)) {
			if (gameID & GAMEID_AO) failed = !SetStrVaule(_struZaConfig.Ao.VtblDir, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetStrVaule(_struZaConfig.Zero.VtblDir, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceTableFileExtension)) {
			if (gameID & GAMEID_AO) failed = !SetStrVaule(_struZaConfig.Ao.VtblExt, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetStrVaule(_struZaConfig.Zero.VtblExt, pb + p);
		}
		else if (p = Compare(pb, STR_DisableOriginalVoice)) {
			if (gameID & GAMEID_AO) failed = !SetIntVaule(_struZaConfig.Ao.DisableOriginalVoice, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetIntVaule(_struZaConfig.Zero.DisableOriginalVoice, pb + p);
		}
		else if (p = Compare(pb, STR_Volume)) {
			if (gameID & GAMEID_AO) failed = !SetIntVaule(_struZaConfig.Ao.Volume, pb + p);
			if (gameID & GAMEID_ZERO) failed = !SetIntVaule(_struZaConfig.Zero.Volume, pb + p);
		}
		else if (p = Compare(pb, STR_OpenDebugLog)) {
			failed = !SetIntVaule(_struZaConfig.General.OpenDebugLog, pb + p);
		}
		else if (p = Compare(pb, STR_UseLogFile)) {
			failed = !SetIntVaule(_struZaConfig.General.UseLogFile, pb + p);
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
		_ptrZaConfig->ActiveGameID = GAMEID_ZERO;
		_ptrZaConfig->ActiveGame = &_ptrZaConfig->Zero;
		break;
	case GAMEID_AO:
		_ptrZaConfig->ActiveGameID = GAMEID_AO;
		_ptrZaConfig->ActiveGame = &_ptrZaConfig->Ao;
		break;
	default:
		break;
	}
	
}

void ZaConfigSetDefault() {
	_struZaConfig.ActiveGameID = GAMEID_INVALID;
	_struZaConfig.ActiveGame = nullptr;

	_struZaConfig.General.OpenDebugLog = DFT_DEBUGLOG;
	_struZaConfig.General.UseLogFile = DFT_USELOGFILE;

	_struZaConfig.General.SleepTime = DFT_SLEEP_TIME;
	_struZaConfig.General.RemoveFwdCtrlCh = DFT_RMFWDCTRLCH;
	_struZaConfig.General.Mode = MODE_AUTO;


	_struZaConfig.Zero.VoiceDir = Z_DFT_VOICE_DIR;
	_struZaConfig.Zero.VoiceExt = Z_DFT_VOICE_EXT;
	_struZaConfig.Zero.VoiceName = Z_DFT_VOICE_NAME;

	_struZaConfig.Zero.VtblExt = Z_DFT_VOICETABLE_EXT;
	_struZaConfig.Zero.VtblDir = Z_DFT_VOICETABLE_DIR;

	_struZaConfig.Zero.Volume = Z_DFT_VOLUME;
	_struZaConfig.Zero.DisableOriginalVoice = Z_DFT_DISABLE_ORIVOICE;

	_struZaConfig.Zero.VoiceIdLength = Z_LENGTH_VOICE_ID;


	_struZaConfig.Ao.VoiceDir = A_DFT_VOICE_DIR;
	_struZaConfig.Ao.VoiceExt = A_DFT_VOICE_EXT;
	_struZaConfig.Ao.VoiceName = A_DFT_VOICE_NAME;

	_struZaConfig.Ao.VtblExt = A_DFT_VOICETABLE_EXT;
	_struZaConfig.Ao.VtblDir = A_DFT_VOICETABLE_DIR;

	_struZaConfig.Ao.Volume = A_DFT_VOLUME;
	_struZaConfig.Ao.DisableOriginalVoice = A_DFT_DISABLE_ORIVOICE;

	_struZaConfig.Ao.VoiceIdLength = A_LENGTH_VOICE_ID;
}
