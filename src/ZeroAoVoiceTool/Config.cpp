#include "stdafx.h"
#include "Config.h"

#include <ini.hpp>
#include <fstream>

#include "Const.h"

static bool _setValue(int& var, INI::Level& level, const char* name) {
	auto it = level.values.find(name);
	if (it == level.values.end()) return false;
	var = atoi(it->second.c_str());
	return true;
}

bool Config::LoadConfig(const char * configFile)
{
	INI::Parser* ini = nullptr;
	try {
		ini = new INI::Parser(configFile);
	}
	catch (const std::exception&) {
		delete ini;
		return false;
	}
	if (ini->top().values.size() == 0) return false;
	Reset();

	_setValue(Volume, ini->top(), STR_Volume);
	_setValue(DisableOriVoice, ini->top(), STR_DisableOriVoice);
	_setValue(Width, ini->top(), STR_Width);
	_setValue(Height, ini->top(), STR_Height);
	_setValue(MaxLogNum, ini->top(), STR_MaxLogNum);

	delete ini;
	return false;
}

bool Config::SaveConfig(const char * configFile)
{
	std::ofstream ofs(configFile);
	if (!ofs) return false;

	ofs << STR_Volume << " = " << Volume << '\n'
		<< STR_DisableOriVoice << " = " << DisableOriVoice << '\n'
		<< STR_Width << " = " << Width << '\n'
		<< STR_Height << " = " << Height << '\n'
		<< STR_MaxLogNum << " = " << MaxLogNum << std::endl;

	ofs.close();
	return true;
}

Config::Config()
{
	Reset();
}

void Config::Reset()
{
	Volume = DFT_VOLUME;
	DisableOriVoice = DFT_DISABLE_ORIVOICE;
	MaxLogNum = DFT_MAX_LOGNUM;
	Width = Height = 0;
}
