#ifndef __ZAERRORMSG_H__
#define __ZAERRORMSG_H__

#define MAX_ERRMSG_LEGNTH 500

namespace Za { 
	namespace Error {
		const char* LastErrMsg();
		void SetErrMsg(const char* err = nullptr);
	}
}

#endif // !__ZAERRORMSG_H__
