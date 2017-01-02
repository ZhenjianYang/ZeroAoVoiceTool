#ifndef __ZACOMMONMETHOD_H__
#define __ZACOMMONMETHOD_H__

#ifndef CpyStrToArray
#define CpyStrToArray(dst, src) StrCpyN(dst, src, sizeof(dst) / sizeof(*dst) - 1);
#endif

static int StrCpyN(char* dst, const char* src, int maxlen) {
	int i;
	for (i = 0; i < maxlen && src[i]; ++i) {
		dst[i] = src[i];
	}
	dst[i] = 0;
	return i;
}

#endif
