#include <Windows.h>

#include <direct.h>

#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

#include "ZaConst.h"

#define A_DFT_VOICETABLE_EXT	"tbl"
#define Z_DFT_VOICETABLE_EXT	"tbl"	

using namespace std;
int length_voiceid = Z_LENGTH_VOICE_ID;

static string GetScenaName(const string& fileName);
static void GetLines(const string& fileName, vector<string> &strs, vector<int> &offsets);
static int GetVoiceId(const string& str);
static void ZaGetSubFiles(const std::string& dir, const std::string& searchName, std::vector<std::string> &subs);

static bool addjp = false;
static bool addcn = false;
static bool addnov = false;

static bool isao = false;

#define LINE_BUFF_SIZE 2048

int main(int argc, char* argv[]) {
	if (argc <= 3) {
		cout << "Usage :\n"
			"\tZaMakeVoiceTable [-a] [[-f] | [-j] [-c] [-v]] dir_tbl dir_evo dir_cn\n"
			"\t\t-j : 添加日文文本\n"
			"\t\t-c : 添加中文文本（注释）\n"
			"\t\t-v : 添加无语音条目\n"
			"\t\t-f : 以上全部\n"
			"\t\t-a : 碧之轨迹模式\n" << endl;
		return -1;
	}

	string dir_tbl, dir_evo, dir_cn;
	int count = 0;
	for(int i = 1; i < argc; ++i)
	{
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'j') addjp = true;
			else if (argv[i][1] == 'c') addcn = true;
			else if (argv[i][1] == 'v') addnov = true;
			else if (argv[i][1] == 'f') {
				addjp = addcn = addnov = true;
			}
			else if (argv[i][1] == 'a') isao = true;
			else {
				cout << "Ignored : " << argv[i] << endl;
			}
		}
		else {
			if (count == 0) dir_tbl = argv[i];
			else if (count == 1) dir_evo = argv[i];
			else if (count == 2) dir_cn = argv[i];
			else {
				cout << "Ignored : " << argv[i] << endl;
			}
			++count;
		}
	}

	length_voiceid = isao ? A_LENGTH_VOICE_ID : Z_LENGTH_VOICE_ID;
	const string voicefile_ext = isao ? A_DFT_VOICETABLE_EXT : Z_DFT_VOICETABLE_EXT;
	_mkdir(dir_tbl.c_str());

	vector<string> subs_evo;
	ZaGetSubFiles(dir_evo, "*.txt", subs_evo);

	for (auto &sub : subs_evo) {
		string sname = GetScenaName(sub);
		cout << "处理" << sname << "..." << endl;

		vector<string> levo, lcn;
		vector<int> ofevo, ofcn;
		GetLines(dir_evo + '\\' + sub, levo, ofevo);
		GetLines(dir_cn + '\\' + sub, lcn, ofcn);

		ofstream ofs_tbl;

		bool first = true;
		for (int i = 0; i < (int)levo.size(); ++i) {
			if (ofevo[i] == INVALID_OFFSET)
				continue;

			int voiceId = GetVoiceId(levo[i]);
			if (voiceId == INVAILD_VOICE_ID && !addnov)
				continue;

			int off_cn = i < (int)ofcn.size() ? ofcn[i] : INVALID_OFFSET;
			if (off_cn == INVALID_OFFSET) continue;

			if (first) {
				first = false;
				ofs_tbl.open(dir_tbl + '\\' + sname + '.' + voicefile_ext);
			}

			ofs_tbl << std::hex << setfill('0') << setw(6) << setiosflags(ios::uppercase) << setiosflags(ios::right) << off_cn << ' '
				<< std::dec << setfill('0') << setw(length_voiceid) << setiosflags(ios::right) << voiceId;

			if (addjp) {
				wchar_t wbuf[LINE_BUFF_SIZE / 2 + 1];
				char mbuf[LINE_BUFF_SIZE + 1];

				MultiByteToWideChar(932, 0, levo[i].c_str(), levo[i].size() + 1, wbuf, LINE_BUFF_SIZE / 2);
				WideCharToMultiByte(936, 0, wbuf, LINE_BUFF_SIZE / 2, mbuf, LINE_BUFF_SIZE, NULL, NULL);
				ofs_tbl << " " << mbuf;
			}

			if (addcn) {
				ofs_tbl << endl;
				ofs_tbl << ";";
				ofs_tbl << lcn[i];
			}

			ofs_tbl << endl << endl;
		}

		if (!first) ofs_tbl.close();
	}
}

string GetScenaName(const string& fileName)
{
	int left = fileName.rfind('\\') + 1;
	int right = fileName.find('.', left);
	if (right == -1) right = fileName.size();

	return fileName.substr(left, right - left);
}

void GetLines(const string& fileName, vector<string> &strs, vector<int> &offsets)
{
	strs.clear();
	ifstream ifs(fileName.c_str());

	char buff[LINE_BUFF_SIZE + 1];
	while (ifs.getline(buff, LINE_BUFF_SIZE))
	{
		if (buff[0] == '\0') continue;
		
		int off;
		if (!sscanf_s(buff, "offset=0x%X", &off))
			continue;

		if (!ifs.getline(buff, LINE_BUFF_SIZE))
			break;

		strs.push_back(buff);
		offsets.push_back(off);
	}

	ifs.close();
}

int GetVoiceId(const string& str) {
	int vid = INVAILD_VOICE_ID;

	int left = -1;
	while ((left = str.find('#', left + 1)) != string::npos
		&& left + length_voiceid + 1 < (int)str.size())
	{
		if (str[left + length_voiceid + 1] != 'V') continue;
		
		vid = 0;
		for (int i = 1; i <= length_voiceid; ++i) {
			if (str[left + i] >= '0' && str[left + i] <= '9') {
				vid *= 10;
				vid += str[left + i] - '0';
			}
			else {
				vid = INVAILD_VOICE_ID;
				break;
			}
		}

		if (vid != INVAILD_VOICE_ID) break;
	}
	return vid;
}

void ZaGetSubFiles(const std::string& dir, const std::string& searchName, std::vector<std::string> &subs)
{
	WIN32_FIND_DATA wfdp;
	HANDLE hFindp = FindFirstFile((dir + '\\' + searchName).c_str(), &wfdp);
	if (hFindp != NULL) {
		do
		{
			if (!(wfdp.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				subs.push_back(wfdp.cFileName);
			}
		} while (FindNextFile(hFindp, &wfdp));
	}
}
