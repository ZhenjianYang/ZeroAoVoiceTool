#include "ZaVoiceTable.h"
#include "ZaConst.h"

#include <fstream>
#include <map>
#include <string>

using namespace std;

#define MAXCH_ONE_LINE 1024

typedef Za::VoiceTable::VoiceInfo VoiceInfo;
typedef map<int, VoiceInfo> _TableType;
typedef map<string, const _TableType*> _GroupType;

static _GroupType _group;

static const _TableType _emptyTable;
static const _GroupType _emptyGroup = { {"",  &_emptyTable} };

static _GroupType::const_iterator _cur = _emptyGroup.begin();
#define _curTable _cur->second 
#define _curName _cur->first

static bool _getAndFormat(char* buff, int &offset, int &voiceid, const char* &jpText, int &jpLen);
static const _TableType* _creatTable(const char* vtblFile);
static void _destoryTable(const _TableType* table);

int Za::VoiceTable::Num()
{
	return _cur->second->size();
}

const VoiceInfo* Za::VoiceTable::GetVoiceInfo(int offset)
{
	auto it = _curTable->find(offset);
	if (it != _curTable->end()) {
		return &it->second;
	}
	else {
		return nullptr;
	}
}

const char * Za::VoiceTable::Name()
{
	return _curName.c_str();
}

int Za::VoiceTable::AllGroups::GroupsNum()
{
	return _group.size();
}

void Za::VoiceTable::AllGroups::Clear()
{
	for (auto &it : _group) {
		_destoryTable(it.second);
	}
	_group.clear();
	_cur = _emptyGroup.begin();
}

bool Za::VoiceTable::AllGroups::AddGroup(const char * scenaName, const char * vtblFile)
{
	if (scenaName == nullptr || scenaName[0] == 0) return false;

	auto it = _group.find(scenaName);
	if (it != _group.end()) return false;

	auto newTable = _creatTable(vtblFile);
	if (!newTable) return false;

	_group[scenaName] = newTable;
	return true;
}

bool Za::VoiceTable::AllGroups::SetCurGroup(const char * scenaName)
{
	auto it = _group.find(scenaName);
	if (it == _group.end()) _cur = _emptyGroup.begin();
	else _cur = it;

	return true;
}

const _TableType* _creatTable(const char* vtblFile) {
	std::ifstream ifs(vtblFile, std::ifstream::in);
	if (!ifs) return nullptr;

	char buff[MAXCH_ONE_LINE + 1];
	_TableType* newTable = new _TableType;

	while (ifs.getline(buff, MAXCH_ONE_LINE))
	{
		if (buff[0] == '\0' || buff[0] == '#' || buff[0] == ';')
			continue;

		int offset, voiceid, jpLen;
		const char* jpText;

		if (!_getAndFormat(buff, offset, voiceid, jpText, jpLen)) {
			continue;
		}

		if (offset == INVALID_OFFSET) continue;
		if (newTable->find(offset) != newTable->end()) continue;

		VoiceInfo vinf;
		vinf.voiceId = voiceid;

		if (jpLen) {
			char* tjpText = new char[jpLen + 1];
			for (int i = 0; jpText[i]; ++i) tjpText[i] = jpText[i];
			tjpText[jpLen] = 0;
			vinf.jpText = tjpText;
		}
		else {
			vinf.jpText = nullptr;
		}

		(*newTable)[offset] = vinf;
	}

	ifs.close();
	return newTable;
}
void _destoryTable(const _TableType* table) {
	if (!table) return;
	for (auto &it : *table) {
		delete [] it.second.jpText;
	}
	delete table;
}
bool _getAndFormat(char* buff, int &offset, int &voiceid, const char* &jpText, int &jpLen) {
	offset = voiceid = 0;
	jpText = nullptr;
	jpLen = 0;

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
	for (char* tjpText = buff;  ; ++tjpText, ++jpLen) {
		if (*buff == '\\' && *(buff + 1) == 'n') { *tjpText = '\n'; buff += 2; }
		else { *tjpText = *buff; ++buff; }

		if (*tjpText == '\0') break;
	}
	return true;
}

