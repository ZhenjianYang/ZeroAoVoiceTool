#include "ZaGameData.h"

#include <ini.hpp>

#include <map>

using namespace std;

template<typename Type>
static bool SetValue(Type &var, const INI::Level& level, const string& name) {
	auto it = level.values.find(name);
	if (it == level.values.end()) return false;

	const string& val = it->second;
	char* p;
	int radix = 10;
	if (val[0] == '0' && (val[1] == 'x' || val[1] == 'X')) {
		radix = 16;
	}

	long t = std::strtol(val.c_str(), &p, radix);
	var = (Type)t;

	return true;
}

template<>
static bool SetValue(string &var, const INI::Level& level, const string& name) {
	auto it = level.values.find(name);
	if (it == level.values.end()) return false;

	const string& val = it->second;
	var = val;

	return true;
}

static void GetFromFile(std::list<Za::Data::GameData>& gdl, 
						map<string, std::list<Za::Data::GameData>::iterator>& map_name_data,
						const char* dataFN)
{
	if (dataFN) {
		INI::Parser ini(dataFN);
		auto &sections = ini.top().ordered_sections;

		for (auto &it : sections) {
			const string& name = it->first;
			if (map_name_data.find(name) == map_name_data.end()) {
				Za::Data::GameData gd;

				if (SetValue(gd.Base, it->second, STR_Base)) {
					auto it_base = map_name_data.find(gd.Base);
					if (it_base == map_name_data.end()) {
						continue;
					}
					gd = *it_base->second;
				}
				gd.Name = name;

				SetValue(gd.Enable, it->second, STR_Enable);
				SetValue(gd.Title, it->second, STR_Title);
				SetValue(gd.Comment, it->second, STR_Comment);
				SetValue(gd.FeatureAddr, it->second, STR_FeatureAddr);
				SetValue(gd.FeatureValue, it->second, STR_FeatureValue);
				SetValue(gd.AddrOpJc[0], it->second, STR_OpAddrJcLoadScena);
				SetValue(gd.AddrFunc[0], it->second, STR_AddrLoadScena);
				SetValue(gd.AddrOpJc[1], it->second, STR_OpAddrJcLoadScena1);
				SetValue(gd.AddrFunc[1], it->second, STR_AddrLoadScena1);
				SetValue(gd.AddrOpJc[2], it->second, STR_OpAddrJcLoadBlock);
				SetValue(gd.AddrFunc[2], it->second, STR_AddrLoadBlock);
				SetValue(gd.AddrOpJc[3], it->second, STR_OpAddrJcShowText);
				SetValue(gd.AddrFunc[3], it->second, STR_AddrShowText);
				SetValue(gd.PtrPostMessageA, it->second, STR_PtrPostMessageA);
				SetValue(gd.VoiceIdLegnth, it->second, STR_VoiceIdLegnth);
				SetValue(gd.VoiceFileName, it->second, STR_VoiceFileName);
				SetValue(gd.VoiceFileDir, it->second, STR_VoiceFileDir);
				SetValue(gd.VoiceTablesDir, it->second, STR_VoiceTablesDir);

				gdl.emplace_back(gd);
				map_name_data[name] = gdl.end();
				--map_name_data[name];
			}
		}
	}
}

int Za::Data::GameData::GetFromFiles(std::vector<GameData>& dataList, const char * dataFN, const char * dataCustomizedFN)
{
	std::list<GameData> gdl;
	std::list<GameData> gdlc;

	map<string, std::list<GameData>::iterator> map_name_data;

	GetFromFile(gdl, map_name_data, dataFN);
	GetFromFile(gdlc, map_name_data, dataCustomizedFN);

	dataList.clear();
	for (auto & gd : gdlc) {
		dataList.emplace_back(gd);
	}
	for (auto & gd : gdl) {
		dataList.emplace_back(gd);
	}

	return dataList.size();
}
