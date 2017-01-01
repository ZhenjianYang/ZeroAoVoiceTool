#include "stdafx.h"
#include "Config.h"

#include <ini.hpp>
#include <fstream>
#include <stdlib.h>
#include <iomanip>

#include "Const.h"

static bool _setValue(int& var, INI::Level& level, const char* name) {
	auto it = level.values.find(name);
	if (it == level.values.end()) return false;

	int rad = 10;
	if (it->second[0] == '0' && (it->second[1] == 'x' || it->second[1] == 'X')) {
		rad = 16;
	}
	char *p;
	var = (int)std::strtoll(it->second.c_str(), &p, rad);
	return true;
}

static bool _setValue(char var[], INI::Level& level, const char* name) {
	auto it = level.values.find(name);
	if (it == level.values.end()) return false;
	int i;
	for (i = 0; i < MAX_FONTNAME_LENGTH && it->second[i]; ++i) {
		var[i] = it->second[i];
	}
	var[i] = 0;
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
	_setValue(MaxLogNum, ini->top(), STR_MaxLogNum);
	_setValue(RemembeWinInfo, ini->top(), STR_RemembeWinInfo);

	_setValue(PosX, ini->top(), STR_PosX);
	_setValue(PosY, ini->top(), STR_PosY);
	_setValue(Width, ini->top(), STR_Width);
	_setValue(Height, ini->top(), STR_Height);

	_setValue(FontName, ini->top(), STR_FontName);
	_setValue(FontSize, ini->top(), STR_FontSize);
	_setValue(FontColor, ini->top(), STR_FontColor);
	_setValue(FontStyle, ini->top(), STR_FontStyle);

	delete ini;
	return true;
}

bool Config::SaveConfig(const char * configFile)
{
	std::ofstream ofs(configFile);
	if (!ofs) return false;

	ofs << STR_Volume << " = " << Volume << '\n'
		<< STR_DisableOriVoice << " = " << DisableOriVoice << '\n'
		<< STR_MaxLogNum << " = " << MaxLogNum << '\n'
		<< STR_RemembeWinInfo << " = " << RemembeWinInfo << '\n';

	if (RemembeWinInfo) {
		ofs << STR_Width << " = " << Width << '\n'
			<< STR_Height << " = " << Height << '\n'
			<< STR_PosX << " = " << PosX << '\n'
			<< STR_PosY << " = " << PosY << '\n';
	}

	if (FontName[0]) ofs << STR_FontName << " = " << FontName << '\n';
	if (FontSize) ofs << STR_FontSize << " = " << FontSize << '\n';
		
	if (FontColor) ofs << STR_FontColor << " = 0x" << std::hex << std::uppercase 
		<< std::setfill('0') << std::setw(6) << std::setiosflags(std::ios::right) << FontColor << '\n';
	if (FontStyle) ofs	<< STR_FontStyle << " = 0x" << std::hex << std::uppercase 
		<< std::setfill('0') << std::setw(4) << std::setiosflags(std::ios::right) << FontStyle << '\n';

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

	RemembeWinInfo = 0;

	Width = Height = 0;
	PosX = PosY = 0;

	FontName[0] = 0;
	FontSize = 0;
	FontColor = 0;
	FontStyle = 0;
}
