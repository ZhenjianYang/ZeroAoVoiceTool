#ifndef __ZAVOICETABLE_H__
#define __ZAVOICETABLE_H__

namespace Za {
	class VoiceTable {
	public:
		struct VoiceInfo
		{
			int voiceId;
			const char* jpText;
		};

		class AllGroups {

		public:
			static int GroupsNum();
			static void Clear();
			static bool AddGroup(const char* scenaName, const char* vtblFile);

			static bool SetCurGroup(const char* scenaName);

		private:
			virtual ~AllGroups() = 0;
		};

		static int Num();
		static const VoiceInfo* GetVoiceInfo(int offset);
		static const char* Name();

	private:
		virtual ~VoiceTable() = 0;
	};
}

#endif // !__ZAVTBL_H__
