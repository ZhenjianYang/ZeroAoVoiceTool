#ifndef __ZACONST_H__
#define __ZACONST_H__

#define ZA_VERSION "0.3.0"

#define Z_LENGTH_VOICE_ID		7
#define A_LENGTH_VOICE_ID		9
#define MAX_LENGTH_VOICE_ID		10
#define INVAILD_VOICE_ID		0

#define INVALID_OFFSET	0xFFFFFF
#define FAKE_OFFSET		0x10000000

#define OFF_OFF_SCENANAME	0x34
#define MAX_SCENA_SIZE		(180 * 1024)

#define MAX_SCENANAME_LENGTH 9
#define MIN_SCENANAME_LENGTH 4

#define RM_FWD_CTRL_CH	1

#define MAX_VOLUME 100

#define VOICE_TABLE_ATTR	"tbl"
#define VOICE_FILE_ATTRS	{"ogg", "wav"}

#define DATA_FILENAME		"Data.ini"
#define DATACSTM_FILENAME	"DataCustomized.ini"

#define MSGTYPE_LOADSCENA	0
#define MSGTYPE_LOADSCENA1	1
#define MSGTYPE_LOADBLOCK	2
#define MSGTYPE_SHOWTEXT	3
#define MSGTYPE_PLAYWAIT	4


#endif // !__ZACONST_H__

