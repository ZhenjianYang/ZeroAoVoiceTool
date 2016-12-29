#ifndef __CONFIG_H__
#define __CONFIG_H__

#define STR_Volume			"Volume"
#define STR_DisableOriVoice	"DisableOriVoice"
#define STR_Width			"Width"
#define STR_Height			"Height"
#define STR_MaxLogNum		"MaxLogNum"

struct Config {
	int Volume;
	int DisableOriVoice;
	int Width;
	int Height;
	int MaxLogNum;
	
	bool LoadConfig(const char* configFile);
	bool SaveConfig(const char* configFile);

	Config();

	void Reset();
};

#endif
