#ifndef __ZASCENAANALYZER_H__
#define __ZASCENAANALYZER_H__

namespace Za {
	class ScenaAnalyzer {
	public:
		static int Init(void* data = nullptr);
		static int End();

		static int DLoadScena(unsigned raScena, const char* &out_scenaName);
		static int DLoadScena1(unsigned raScena1, const char* &out_scenaName);
		static int DLoadBlock(unsigned raBlock, const char* &out_scenaName);
		static int DShowText(unsigned raText, int & out_voiceID, bool &out_wait);

	private:
		virtual ~ScenaAnalyzer() = 0;
	};
}

#endif
