#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>
#include <fstream>

#include <Windows.h>

#include "ZaConst.h"

using namespace std;

int length_voiceid = Z_LENGTH_VOICE_ID;

static void ZaGetSubFiles(const std::string& dir, const std::string& searchName, std::vector<std::string> &subs);

bool Check(unsigned char* buf) {

	if (*(buf + length_voiceid + 1) != 'V' && *(buf + length_voiceid + 1) != 'v') return false;
	if (*buf != '#') return false;

	for (int i = 1; i <= length_voiceid; ++i) {
		if (buf[i] < '0' || buf[i] > '9') return false;
	}

	return true;
}

void static outputmap(ofstream& ofs, const map<int, int>& m, const char* name)
{
	ofs << "-------------------------------------" << endl;
	ofs << name << " size: " << m.size() << endl;
	int width = name[2] == '2' ? 4 : 2;

	for (auto &it : m)
	{
		ofs << std::hex << setfill('0') << setw(width) << setiosflags(ios::uppercase) << setiosflags(ios::right) << it.first << ','
			<< std::dec << setw(0) << it.second << endl;
	}
	ofs << "#####################################" << endl;

}

static bool isao = false;
int main(int argc, char* argv[]) {
	if (argc <= 1) {
		cout << "Usage :\n"
			"\tZaMakeVoiceTableFound [-a] dir_evo\n"
			"\t\t-a : 碧之轨迹模式\n" << endl;
		return -1;
	}

	string dir_evo;
	int count = 0;
	for(int i = 1; i < argc; ++i)
	{
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'a') isao = true;
			else {
				cout << "Ignored : " << argv[i] << endl;
			}
		}
		else {
			if (count == 0) dir_evo = argv[i];
			else {
				cout << "Ignored : " << argv[i] << endl;
			}
			++count;
		}
	}

	if (isao) length_voiceid = A_LENGTH_VOICE_ID;

	string path_rep = dir_evo + ".txt";
	vector<string> subs_evo;
	ZaGetSubFiles(dir_evo, "*.bin", subs_evo);

	map<int, int> mpb, mpa, mna, mnb;
	map<int, int> mp2, mn2;

	for (auto &sub : subs_evo) {
		ifstream ifs(dir_evo + "\\" + sub, ifstream::binary);
		if (!ifs) continue;
		ifs.seekg(0, ios::end);
		int len = (int)ifs.tellg();

		ifs.seekg(0, ios::beg);
		unsigned char *buf = new unsigned char[len];
		ifs.read((char*)buf, len);
		ifs.close();

		ofstream ofs;

		int index = 0;
		bool first = true;

		cout << "处理" << sub << "..." << endl;

		while (index + length_voiceid + 2 < len)
		{
			if (Check(buf + index)) {
				if (first) {
					ofs.open(dir_evo + '\\' + sub + ".txt");
					first = false;
				}

				while (buf[index-1] >= 0x20 && buf[index-1] <= 0x7F)
					--index;

				unsigned int pa, pb, na, nb;
				string ts;
				int lent = 0;

				pa = buf[index - 1];
				pb = buf[index - 2];

				++mpa[pa]; ++mpb[pb]; ++mp2[pb * 0x100 + pa];
				for (;;) {
					if (buf[index + lent] < 0x20 || buf[index + lent] == 0xFF) {
						if (buf[index + lent] == 0x01) {
							ts.push_back('\\');
							ts.push_back('n');
						}
						else
						{
							na = buf[index + lent];
							nb = buf[index + lent + 1];
							++mna[na]; ++mnb[nb]; ++mn2[na * 0x100 + nb];
							buf[index + lent] = 0;
							break;
						}
					}
					else {
						ts.push_back(buf[index + lent]);
					}
					++lent;
				}

				ofs << "offset=0x" << std::hex << setfill('0') << setw(6) << setiosflags(ios::uppercase) << setiosflags(ios::right) << index << ','
					<< "pb=0x" << std::hex << setfill('0') << setw(2) << setiosflags(ios::uppercase) << setiosflags(ios::right) << pb << ','
					<< "pa=0x" << std::hex << setfill('0') << setw(2) << setiosflags(ios::uppercase) << setiosflags(ios::right) << pa << ','
					<< "p2=0x" << std::hex << setfill('0') << setw(4) << setiosflags(ios::uppercase) << setiosflags(ios::right) << pb * 0x100 + pa << ','
					<< "na=0x" << std::hex << setfill('0') << setw(2) << setiosflags(ios::uppercase) << setiosflags(ios::right) << na << ','
					<< "nb=0x" << std::hex << setfill('0') << setw(2) << setiosflags(ios::uppercase) << setiosflags(ios::right) << nb << ','
					<< "n2=0x" << std::hex << setfill('0') << setw(4) << setiosflags(ios::uppercase) << setiosflags(ios::right) << na * 0x100 + nb << ',' << endl;
				ofs << ts << endl << endl;
				index += lent;
			}
			else {
				++index;
			}
		}

		if (!first)
			ofs.close();

		delete[] buf;
	}

	ofstream ofs(path_rep.c_str());
	outputmap(ofs, mpa, "mpa");
	outputmap(ofs, mpb, "mpb");
	outputmap(ofs, mp2, "mp2");
	outputmap(ofs, mna, "mna");
	outputmap(ofs, mnb, "mnb");
	outputmap(ofs, mn2, "mn2");

	return 0;
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
