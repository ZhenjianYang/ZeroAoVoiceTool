#include "ZaVoiceTable.h"
#include "ZaConst.h"

#include <fstream>

#define MAXCH_ONE_LINE 1024

static bool GetAndFormat(char* buff, int &offset, int &voiceid, const char* &jpText) {
	offset = voiceid = 0;
	jpText = NULL;

	while (*buff == 0x20 || *buff == '\t') ++buff;
	while (*buff != 0x20 && *buff != '\t') {
		offset *= 16;
		if (*buff >= '0' && *buff <= '9') offset += *buff - '0';
		else if (*buff >= 'A' && *buff <= 'F') offset += *buff - 'A' + 10;
		else if (*buff >= 'a' && *buff <= 'f') offset += *buff - 'a' + 10;
		else return false;
		++buff;
	}

	while (*buff == 0x20 || *buff == '\t') ++buff;
	if (*buff == 0) return false;
 	while (*buff != 0x20 && *buff != '\t' && *buff != '\0') {
		voiceid *= 10;
		if (*buff >= '0' && *buff <= '9') voiceid += *buff - '0';
		else return false;
		++buff;
	}

	while (*buff == 0x20 || *buff == '\t') ++buff;
	jpText = buff;
	while (*buff != 0) {
		if (*buff == '\\' && *(buff + 1) == 'n') { *buff = ' '; *(buff + 1) = '\n'; }
		++buff;
	}
	return true;
}

bool ZaVoiceTable::LoadTblFile(const char * vtblFile)
{
	destory();

	std::ifstream ifs(vtblFile, std::ifstream::in);
	if (!ifs) return false;

	char buff[MAXCH_ONE_LINE + 1];

	while (ifs.getline(buff, MAXCH_ONE_LINE))
	{
		if (buff[0] == '\0' || buff[0] == '#' || buff[0] == ';')
			continue;

		int offset, voiceid;
		const char* jpText;
		
		if (!GetAndFormat(buff, offset, voiceid, jpText)) {
			continue;
		}

		if (map_off_vinf.find(offset) != map_off_vinf.end()) continue;
		
		VoiceInfo* vinf = new VoiceInfo;
		vinf->voiceID = voiceid;
		vinf->jpText = jpText;

		map_off_vinf[offset] = vinf;
	}

	ifs.close();
	return true;
}

const VoiceInfo * ZaVoiceTable::GetVoiceInfo(int offset) const
{
	auto tvif = map_off_vinf.find(offset);
	if (tvif == map_off_vinf.end()) return nullptr;

	return tvif->second;
}

void ZaVoiceTable::destory()
{
	for (auto ovi : map_off_vinf) {
		delete ovi.second;
	}
	map_off_vinf.clear();
}
