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
			if(mode & MODE_AO) failed = !SetStrVaule(_zaConfigData.ao_dir_voice, pb + p);
			if(mode & MODE_ZERO) failed = !SetStrVaule(_zaConfigData.zero_dir_voice, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceFileExtension)) {
			if (mode & MODE_AO) failed = !SetStrVaule(_zaConfigData.ao_ext_voice, pb + p);
			if (mode & MODE_ZERO) failed = !SetStrVaule(_zaConfigData.zero_ext_voiceFile, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceTableFileDirectory)) {
			if (mode & MODE_AO) failed = !SetStrVaule(_zaConfigData.ao_dir_voiceTable, pb + p);
			if (mode & MODE_ZERO) failed = !SetStrVaule(_zaConfigData.zero_dir_voiceTable, pb + p);
		}
		else if (p = Compare(pb, STR_VoiceTableFileExtension)) {
			if (mode & MODE_AO) failed = !SetStrVaule(_zaConfigData.ao_ext_voiceTable, pb + p);
			if (mode & MODE_ZERO) failed = !SetStrVaule(_zaConfigData.zero_ext_voiceTable, pb + p);
		}
		else if (p = Compare(pb, STR_DisableOriginalVoice)) {
			if (mode & MODE_AO) failed = !SetIntVaule(_zaConfigData.ao_disableOriginalVoice, pb + p);
		}
		else if (p = Compare(pb, STR_OpenDebugLog)) {
			failed = !SetIntVaule(_zaConfigData.debuglog, pb + p);
		}
		else if (p = Compare(pb, STR_UseLogFile)) {
			failed = !SetIntVaule(_zaConfigData.uselogfile, pb + p);
		}
		else failed = true;

		if (failed) {
			cout << "Config File Bad Line " << cnt << endl;
		}
	}
}

void ZaConfigSetDefault() {
	_zaConfigData.debuglog = DFT_DEBUGLOG;
	_zaConfigData.uselogfile = DFT_USELOGFILE;

	_zaConfigData.sleepTime = DFT_SLEEP_TIME;
	_zaConfigData.removeFwdCtrlCh = DFT_RMFWDCTRLCH;
	_zaConfigData.mode = MODE_AUTO;


	_zaConfigData.zero_ext_voiceFile = Z_DFT_VOICEFILE_EXT;
	_zaConfigData.zero_ext_voiceTable = Z_DFT_VOICETABLE_EXT;
	_zaConfigData.zero_name_voice = Z_DFT_VOICE_NAME;

	_zaConfigData.zero_dir_voice = Z_DFT_VOICE_DIR;
	_zaConfigData.zero_dir_voiceTable = Z_DFT_VOICETABLE_DIR;
	

	_zaConfigData.ao_ext_voice = A_DFT_VOICEFILE_EXT;
	_zaConfigData.ao_ext_voiceTable = A_DFT_VOICETABLE_EXT;
	_zaConfigData.ao_name_voice = A_DFT_VOICE_NAME;

	_zaConfigData.ao_dir_voice = A_DFT_VOICE_DIR;
	_zaConfigData.ao_dir_voiceTable = A_DFT_VOICETABLE_DIR;

	_zaConfigData.ao_disableOriginalVoice = A_DFT_DISABLE_ORIVOICE;
}
