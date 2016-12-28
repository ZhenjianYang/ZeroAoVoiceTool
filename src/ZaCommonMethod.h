#ifndef __ZACOMMONMETHOD_H__
#define __ZACOMMONMETHOD_H__

#ifndef CpyStrToArray
#define CpyStrToArray(dst, src) StrCpyN(dst, src, sizeof(dst) / sizeof(*dst) - 1);
#endif

static char* StrCpyN(char* dst, const char* src, int maxlen) {
	for (int i = 0; i < maxlen && src[i]; ++i) {
		dst[i] = src[i];
	}
	dst[maxlen] = 0;
	return dst;
}

#endif
