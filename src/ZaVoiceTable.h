#ifndef __ZAVOICETABLE_H__
#define __ZAVOICETABLE_H__

namespace Za {
	namespace VoiceTable {
		struct VoiceInfo
		{
			int voiceId;
			const char* jpText;
		};

		namespace AllGroups {
			int GroupsNum();
			void Clear();
			bool AddGroup(const char* scenaName, const char* vtblFile);

			bool SetCurGroup(const char* scenaName);
		};

		int Num();
		const VoiceInfo* GetVoiceInfo(int offset);
		const char* Name();
	}
}

#endif // !__ZAVTBL_H__
