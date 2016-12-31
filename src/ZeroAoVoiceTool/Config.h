#ifndef __CONFIG_H__
#define __CONFIG_H__

#define STR_Volume			"Volume"
#define STR_DisableOriVoice	"DisableOriVoice"
#define STR_MaxLogNum		"MaxLogNum"
#define STR_RemembeWinInfo	"RemembeWinInfo"
#define STR_PosX			"PosX"
#define STR_PosY			"PosY"
#define STR_Width			"Width"
#define STR_Height			"Height"
#define STR_FontName		"FontName"
#define STR_FontSize		"FontSize"
#define STR_FontColor		"FontColor"
#define STR_FontStyle		"FontStyle"


#define FONTSTYLE_WEIGHT	0x3FF
#define FONTSTYLE_ITALIC	0x400
#define FONTSTYLE_UNDERLINE	0x800
#define FONTSTYLE_DELLINE	0x1000


#define MAX_FONTNAME_LENGTH 31

struct Config {
	int Volume;
	int DisableOriVoice;
	int MaxLogNum;
	int RemembeWinInfo;

	int PosX;
	int PosY;
	int Width;
	int Height;

	char FontName[MAX_FONTNAME_LENGTH + 1];
	int FontSize;
	int FontColor;
	int FontStyle;
	
	bool LoadConfig(const char* configFile);
	bool SaveConfig(const char* configFile);

	Config();

	void Reset();
};

#endif
