#ifndef __ZASCENAANALYZER_H__
#define __ZASCENAANALYZER_H__

#include "ZaConst.h"
#include "ZaVoiceTable.h"

namespace Za {
	class ScenaAnalyzer {
	public:
		static int Init(void* data = 0);
		static int End();

		static int DLoadScena(unsigned raScena, const char* &out_scenaName);
		static int DLoadScena1(unsigned raScena1, const char* &out_scenaName);
		static int DLoadBlock(unsigned raBlock, const char* &out_scenaName);
		static int DShowText(unsigned raText, int & out_voiceID, bool &out_wait);

	private:
		struct ScenaData {
			unsigned aScenaX;
			char* pScneaNameX;

			bool operator<(const ScenaData& b) { return aScenaX < b.aScenaX; }
		};
		static ScenaData _scenas[3];
		static char _scenaNameX[3][MAX_SCENANAME_LENGTH + 1];

		static unsigned _raBlock;
		static unsigned _raFirstText;
		static unsigned _raCurText;

		static ZaVoiceTablesGroup _zaVoiceTablesGroup;
		static const ZaVoiceTable *_zaVoiceTable;

		static char* _pScenaName;
		static unsigned _offset1;

		static bool _checkScenaName(const char *scenaName);
		static int _textAnalysisCN(unsigned char *dst, const unsigned char* src);
		static const unsigned char* _textAnalysisJP(const unsigned char* buff);
		static void _loadNewVoiceTable(const char* scenaName);
		static void _clearAllVoiceTable();
		static void _reLoadAllVoiceTables(void * data);

		static int LoadScenaX(unsigned raScena, int X);

	private:
		virtual ~ScenaAnalyzer() = 0;
	};
}

#endif
