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
extern const VoiceInfo InvaildVoiceInfo; 

class ZaVoiceTablesGroup;
class ZaVoiceTable
{
	friend ZaVoiceTablesGroup;
	typedef std::map<int, VoiceInfo*> MapOffVInfo;

private:
	MapOffVInfo map_off_vinf;

public:
	const int Num() const { return map_off_vinf.size(); }

	const VoiceInfo* GetVoiceInfo(int offset) const;

private:
	ZaVoiceTable(const std::string& vtblFile) {
		LoadTblFile(vtblFile);
	}
	bool LoadTblFile(const std::string& vtblFile);
	ZaVoiceTable() {}
	~ZaVoiceTable() { destory(); }
	void destory();

private:
	ZaVoiceTable(const ZaVoiceTable&);
	ZaVoiceTable& operator=(const ZaVoiceTable&);
};

class ZaVoiceTablesGroup {
	typedef std::map<std::string, ZaVoiceTable*> MapNameVoiceTable;
public:
	static const ZaVoiceTable _InvalidVoiceTable;

public:
	const int Num() const { return map_name_vtbl.size(); }
	const ZaVoiceTable* GetVoiceTable(const std::string& name) const;
	const ZaVoiceTable* AddVoiceTable(const std::string& name, const std::string& vtblFile);
	void Clear() { destory(); }
	ZaVoiceTablesGroup() {}
	~ZaVoiceTablesGroup() { destory(); }

private:
	MapNameVoiceTable map_name_vtbl;
	void destory();

private:
	ZaVoiceTablesGroup(const ZaVoiceTablesGroup&);
	ZaVoiceTablesGroup& operator=(const ZaVoiceTablesGroup&);
};

extern const ZaVoiceTable & InvalidVoiceTable;

#endif // !__ZAVTBL_H__
