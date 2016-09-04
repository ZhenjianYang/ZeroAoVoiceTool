#ifndef __ZASCENAANALYZER_H__
#define __ZASCENAANALYZER_H__

int ZaScenaAnalyzerInit();
int ZaScenaAnalyzerEnd();

int ZaDetected_LoadScena(unsigned raScena, const char* &out_scenaName);
int ZaDetected_LoadScena1(unsigned raScena1, const char* &out_scenaName);
int ZaDetected_LoadBlock(unsigned raBlock, const char* &out_scenaName);
int ZaDetected_ShowText(unsigned raText, int & out_voiceID, bool &out_wait);

int GetStrVoiceID(int voiceID, char* buff_strVoiceId);

#endif
