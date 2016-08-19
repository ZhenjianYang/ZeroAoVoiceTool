#include "ZaConfig.h"
#include "ZaConst.h"

#include <fstream>
#include <iostream>

using namespace std;

ZaConfigData _zaConfigData;
const ZaConfigData &zaConfigData = _zaConfigData;

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

	int mode = MODE_AO | MODE_ZERO;

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
		if (Compare(pb, STR_general)) mode = MODE_AO | MODE_ZERO;
		else if (Compare(pb, STR_Ao)) mode = MODE_AO;
		else if (Compare(pb, STR_Zero)) mode = MODE_ZERO;
		else if (p = Compare(pb, STR_VoiceFileDirectory)) {
			if(mode & MODE_AO) failed = !SetStrVaule(_zaConfigData.Ao.VoiceDir, pb + p);
			if(mode & MODE_ZERO) failed = !SetStrVaule(_zaConfigData.Zero.VoiceDir, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceFileExtension)) {
			if (mode & MODE_AO) failed = !SetVStrVaule(_zaConfigData.Ao.VoiceExt, pb + p);
			if (mode & MODE_ZERO) failed = !SetVStrVaule(_zaConfigData.Zero.VoiceExt, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceTableFileDirectory)) {
			if (mode & MODE_AO) failed = !SetStrVaule(_zaConfigData.Ao.VtblDir, pb + p);
			if (mode & MODE_ZERO) failed = !SetStrVaule(_zaConfigData.Zero.VtblDir, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceTableFileExtension)) {
			if (mode & MODE_AO) failed = !SetStrVaule(_zaConfigData.Ao.VtblExt, pb + p);
			if (mode & MODE_ZERO) failed = !SetStrVaule(_zaConfigData.Zero.VtblExt, pb + p);
		}
		else if (p = Compare(pb, STR_DisableOriginalVoice)) {
			if (mode & MODE_AO) failed = !SetIntVaule(_zaConfigData.Ao.DisableOriginalVoice, pb + p);
			if (mode & MODE_ZERO) failed = !SetIntVaule(_zaConfigData.Zero.DisableOriginalVoice, pb + p);
		}
		else if (p = Compare(pb, STR_OpenDebugLog)) {
			failed = !SetIntVaule(_zaConfigData.General.OpenDebugLog, pb + p);
		}
		else if (p = Compare(pb, STR_UseLogFile)) {
			failed = !SetIntVaule(_zaConfigData.General.UseLogFile, pb + p);
		}
		else failed = true;

		if (failed) {
			cout << "Config File Bad Line " << cnt << endl;
		}
	}
}

void ZaConfigSetDefault() {
	_zaConfigData.General.OpenDebugLog = DFT_DEBUGLOG;
	_zaConfigData.General.UseLogFile = DFT_USELOGFILE;

	_zaConfigData.General.SleepTime = DFT_SLEEP_TIME;
	_zaConfigData.General.RemoveFwdCtrlCh = DFT_RMFWDCTRLCH;
	_zaConfigData.General.Mode = MODE_AUTO;


	_zaConfigData.Zero.VoiceDir = Z_DFT_VOICE_DIR;
	_zaConfigData.Zero.VoiceExt = Z_DFT_VOICE_EXT;
	_zaConfigData.Zero.VoiceName = Z_DFT_VOICE_NAME;

	_zaConfigData.Zero.VtblExt = Z_DFT_VOICETABLE_EXT;
	_zaConfigData.Zero.VtblDir = Z_DFT_VOICETABLE_DIR;

	_zaConfigData.Zero.DisableOriginalVoice = Z_DFT_DISABLE_ORIVOICE;

	_zaConfigData.Zero.VoiceIdLength = Z_LENGTH_VOICE_ID;



	_zaConfigData.Ao.VoiceDir = A_DFT_VOICE_DIR;
	_zaConfigData.Ao.VoiceExt = A_DFT_VOICE_EXT;
	_zaConfigData.Ao.VoiceName = A_DFT_VOICE_NAME;

	_zaConfigData.Ao.VtblExt = A_DFT_VOICETABLE_EXT;
	_zaConfigData.Ao.VtblDir = A_DFT_VOICETABLE_DIR;

	_zaConfigData.Ao.DisableOriginalVoice = A_DFT_DISABLE_ORIVOICE;

	_zaConfigData.Ao.VoiceIdLength = A_LENGTH_VOICE_ID;
}
