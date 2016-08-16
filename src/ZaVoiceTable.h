#ifndef __ZAVOICETABLE_H__
#define __ZAVOICETABLE_H__

#include "ZaConst.h"

#include <string>
#include <map>

const int InValidVoiceId = INVAILD_VOICE_ID;

struct VoiceInfo
{
	int voiceID;
	std::string jpText;
};
const VoiceInfo InvaildVoiceInfo = { INVAILD_VOICE_ID, std::string("") };

class ZaVoiceTable
{
	typedef std::map<int, VoiceInfo*> MapOffVInfo;

private:
	MapOffVInfo map_off_vinf;

public:
	const int NumInfo() const { return map_off_vinf.size(); }

	const VoiceInfo* GetVoiceInfo(int offset) const;

	const VoiceInfo* operator[](int offset) const {
		return GetVoiceInfo(offset);
	}

	ZaVoiceTable(const char * vtblFile) {
		LoadTblFile(vtblFile);
	}
	bool LoadTblFile(const char * vtblFile);
	ZaVoiceTable() {}
	~ZaVoiceTable() { destory(); }

private:
	void destory();

private:
	ZaVoiceTable(const ZaVoiceTable&);
	ZaVoiceTable& operator=(const ZaVoiceTable&);
};

#endif // !__ZAVTBL_H__
