#ifndef __ZAIO_H__
#define __ZAIO_H__

#include <windows.h>
#include <string>
#include <vector>

#include "ZaConst.h"

static void ZaGetSubFiles(const std::string& dir, const std::string& searchName, std::vector<std::string> &subs)
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

static char* GetStrVoiceID(int voiceId, int voiceIdLength, char* buff_strVoiceId) {
	if (voiceId == INVAILD_VOICE_ID) {
		for (int i = 0; i < voiceIdLength; ++i)
			buff_strVoiceId[i] = '-';
	}
	else {
		for (int i = voiceIdLength - 1; i >= 0; --i) {
			buff_strVoiceId[i] = voiceId % 10 + '0';
			voiceId /= 10;
		}
	}
	buff_strVoiceId[voiceIdLength] = 0;

	return buff_strVoiceId;
}


#endif
