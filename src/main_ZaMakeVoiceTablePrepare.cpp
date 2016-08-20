#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>

#include <Windows.h>
#include <map>
#include <set>

#include "ZaConst.h"
#include "ZaIo.h"

using namespace std;

const int length_voiceid = Z_LENGTH_VOICE_ID;

set<int> zpre1 = Z_SET_PRE_ONEBYTE;
set<int> znext2 = Z_SET_NEXT_TWOBYTE;

set<int> apre1 = A_SET_PRE_ONEBYTE;
set<int> anext2 = A_SET_NEXT_TWOBYTE;

set<int> pre1;
set<int> next2;

set<int> notallowed_ascii = NOT_ALLOWED_ASCII;

const int MinLength = 4;

int CheckJIS(unsigned char* buf)
{
	if (*buf < 0x20) {
		if (*buf == 0x01) {
			return 1;
		}
		else
			return 0;
	}
	else if (*buf == 0x80 || *buf == 0xFF || *buf == 0xFE 
		|| notallowed_ascii.find(*buf) != notallowed_ascii.end())
		return 0;
	else if (*buf >= 0xA0 && *buf <= 0xDF
		|| *buf >= 0x20 && *buf <= 0x7F) {
		return 1;
	}
	else if(*(buf + 1) >= 0x40 && *(buf + 1) <= 0x7E
		|| *(buf + 1) >= 0x80 && *(buf + 1) <= 0xFC){
		return 2;
	}
	return 0;
}
int CheckGBK(unsigned char* buf)
{
	if (*buf < 0x20) {
		if (*buf == 0x01) {
			return 1;
		}
		else
			return 0;
	}
	else if (*buf == 0x80 || *buf == 0xFF
		|| notallowed_ascii.find(*buf) != notallowed_ascii.end())
		return 0;
	else if (*buf >= 0x20 && *buf <= 0x7F)
	{
		return 1;
	}
	else if (*(buf + 1) >= 0x40 && *(buf + 1) <= 0x7E
		|| *(buf + 1) >= 0x80 && *(buf + 1) <= 0xFE) {
		return 2;
	}
	return 0;
}

typedef int (*CheckFunc)(unsigned char*);

static bool isao = false;
int mode = 0;

int main(int argc, char* argv[]) {
	if (argc <= 1) {
		cout << "Usage :\n"
			"\tZaMakeVoiceTablePrepare [-a] [-c|j] dir\n"
			"\t\t-c|j : 中|日文模式（不加则根据文件夹名称自动判断）\n" 
			"\t\t-a : 碧之轨迹模式\n" << endl;
		return -1;
	}

	string dir;
	int count = 0;
	for (int i = 1; i < argc; ++i)
	{
		if (argv[i][0] == '-') {
			if (argv[i][1] == 'a') isao = true;
			else if (argv[i][1] == 'c') mode = 2;
			else if (argv[i][1] == 'j') mode = 1;
			else {
				cout << "Ignored : " << argv[i] << endl;
			}
		}
		else {
			if (count == 0) dir = argv[i];
			else {
				cout << "Ignored : " << argv[i] << endl;
			}
			++count;
		}
	}

	if (isao) {
		pre1 = apre1;
		next2 = anext2;
	}
	else {
		pre1 = zpre1;
		next2 = znext2;
	}

	vector<string> subs;
	ZaGetSubFiles(dir, "*.bin", subs);

	
	CheckFunc checkFunc = CheckJIS;

	if(mode == 2 || mode == 0 && dir.find("cn", dir.rfind('\\') + 1) != string::npos)
		checkFunc = CheckGBK;

	cout << (checkFunc == CheckJIS ? "JIS" : "GBK") << endl;

	ofstream of_rep(dir + ".txt");

	for (auto sub : subs) {
		cout << "处理" << sub << "..." << endl;

		ifstream ifs(dir + '\\' + sub, ifstream::binary);
		ifs.seekg(0, ios::end);
		int len = (int)ifs.tellg();

		ifs.seekg(0, ios::beg);
		unsigned char *buf = new unsigned char[len];
		ifs.read((char*)buf, len);
		ifs.close();

		ofstream ofs;

		int index = 1;
		bool first = true;
		int num = 0;

		while (index + MinLength + 2 < len) {
			int lent = 0;

			//只允许双字节或#开头
			if ((buf[index] > 0x80 || buf[index] == '#') &&
				pre1.find(buf[index - 1]) != pre1.end()) {
				string ts;
				
				while (index + lent + 2 < len)
				{
					int isJis = checkFunc(buf + index + lent);
					if (isJis == 0) break;
					else if (isJis == 1) {
						if (buf[index + lent] == 0x09) {
							ts.push_back('\\');
							ts.push_back('t');
						}
						else if (buf[index + lent] == 0x01) {
							ts.push_back('\\');
							ts.push_back('n');
						}
						else
							ts.push_back(buf[index + lent]);

						++lent;
					}
					else {
						ts.push_back(buf[index + lent]);
						ts.push_back(buf[index + lent + 1]);
						lent += 2;
					}
				}

				int t = buf[index + lent] * 0x100 + buf[index + lent + 1];
				if (next2.find(t) == next2.end()) lent = 0;
				
				//长度为4的文本只允许2个双字节字符
				if (lent == MinLength) {
					for (int i = 0; i < lent; ++i) {
						if (buf[index + i] < 0x80) {
							lent = 0;
							break;
						}
					}
				}
				//不允许长度5
				if (lent == MinLength + 1)
					lent = 0;

				if (lent >= MinLength) {
					if (first) {
						ofs.open(dir + '\\' + sub + ".txt");
						first = false;
					}
					ofs << "offset=0x" << std::hex << setfill('0') << setw(6) << setiosflags(ios::uppercase) << setiosflags(ios::right) << index << ',' << endl;
					ofs << ts << endl << endl;

					++num;
				}
				else
					lent = 0;
			}

			if (lent == 0) index++;
			else index += lent + 2;
		}//while


		if (num > 0) {
			int left = sub.rfind('\\');
			of_rep << sub.c_str() + left + 1 << ": " << num << endl;
		}
	}//for
	of_rep.close();

	return 0;
}
