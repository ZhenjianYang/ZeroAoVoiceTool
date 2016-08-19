#ifndef __ZAIO_H__
#define __ZAIO_H__

#include <windows.h>
#include <string>
#include <vector>

static void GetSubs(const std::string& dir, const std::string& searchName, std::vector<std::string> &subs)
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


#endif
