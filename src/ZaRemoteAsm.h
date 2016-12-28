#ifndef __ZAREMOTEASM_H__
#define __ZAREMOTEASM_H__

#include "ZaConst.h"
#include "ZaRemote.h"

#define FAKE_OP1 nop
#define FAKE_CODE1 0x90
#define FAKE_OP2 ret
#define FAKE_CODE2 0xC3

#define CODE_NOP 0x90

#define INIT_CODE 0xCC
#define JMP_CODE 0xE9
#define MOVEBX_CODE 0xBB

#define FAKE_OP_CALLPTR nop
#define FAKE_CALLPTR_CODE1 0xFF
#define FAKE_CALLPTR_CODE2 0x15

#define OFF_FIRSTPARAM 0x10

#define FAKE_RAZADATA   0x23D6C51A
#define FAKE_MESSAGE_ID	(FAKE_RAZADATA + 1)
#define FAKE_TYPE		(FAKE_MESSAGE_ID + 1)
#define FAKE_HWND		(FAKE_TYPE + 1)
#define FAKE_PTR_API	(FAKE_HWND + 1)

#define EMPTYVOICE_HEX	0x39393939 //"9999"
#define EMPTYVOICE_LOOPTIMES 20

namespace Za {
	namespace Remote {

		struct RemoteData
		{
			int flag;
			int disableOriVoice;
		};
#define OFF_flag			0
#define OFF_disableOriVoice	4

		__declspec(naked) void rNewLoadScenaMsg()
		{
			__asm
			{
				push eax
				push ebx
				push ecx

				//PostMessage(hwnd, msgId, param, 0)
				mov eax, [esp + OFF_FIRSTPARAM]
				push eax
				push FAKE_TYPE
				push FAKE_MESSAGE_ID
				push FAKE_HWND
				mov eax, FAKE_PTR_API
				call dword ptr[eax]

				pop ecx
				pop ebx
				pop eax

				//这里应为jmp OLD_JCTO_LOADSCENA，先占位
				FAKE_OP2
				FAKE_OP2
				FAKE_OP2
				FAKE_OP2
				FAKE_OP2
			}
		}

		void *const rNewLoadScena1Msg = rNewLoadScenaMsg;
		void *const rNewLoadBlockMsg = rNewLoadScenaMsg;

		__declspec(naked) void rNewShowTextMsg()
		{
			__asm
			{
				push eax
				push ebx
				push ecx

				//ebx = raZaData
				//这里应为mov ebx, raZaData，先占位
				mov ebx, FAKE_RAZADATA

				//if(raZada.disableOriVoice == 0) goto rNewShowTextNoChange
				mov cl, [ebx + OFF_disableOriVoice]
				cmp cl, 0
				je rNewShowTextNoChange

				//eax = param
				mov eax, [esp + OFF_FIRSTPARAM]

				//for (ch = A_EMPTYVOICE_LOOPTIMES; ch > 0; ch--, eax++) {
				//	if ([eax] != '#' || [eax + 5] != 'V') continue;
				//	param[1..4] = "9999"; break; }
				mov ch, EMPTYVOICE_LOOPTIMES + 1
				dec eax
			rNewShowTextLoop :
				inc eax
				dec ch
				jz rNewShowTextNoChange

				mov cl, byte ptr[eax]
				cmp cl, '#'
				jne rNewShowTextLoop
				mov cl, byte ptr[eax + 5]
				cmp cl, 'V'
				jne rNewShowTextLoop
				mov ecx, EMPTYVOICE_HEX
				mov[eax + 1], ecx

			rNewShowTextNoChange :
				//if(raZada.flag != 0) goto rNewShowTextOK
				mov ecx, [ebx + OFF_flag]
				mov dword ptr[ebx + OFF_flag], 0
				cmp ecx, 0
				jne rNewShowTextOK

				//eax = param
				mov eax, [esp + OFF_FIRSTPARAM]
				//if(*param != '#') goto rNewShowTextFlagFailed
				mov cl, byte ptr[eax]
				cmp cl, '#'
				jne rNewShowTextFlagFailed

				//if(param[3] != 'W' && param[4] != 'W') goto rNewShowTextFlagFailed
				mov cl, byte ptr[eax + 4]
				cmp cl, 'W'
				je rNewShowTextFlagOK
				mov cl, byte ptr[eax + 5]
				cmp cl, 'W'
				jne rNewShowTextFlagFailed

			rNewShowTextFlagOK :
				mov dword ptr[ebx + OFF_flag], 1
				jmp rNewShowTextEnd

			rNewShowTextFlagFailed :
				//if(*param < 0x03 || *param == 0xFF) goto rNewShowTextEnd
				mov cl, byte ptr[eax]
				cmp cl, 0x03
				jb rNewShowTextEnd
				cmp cl, 0xFF
				je rNewShowTextEnd

				//if(*(param-1) >= 0x20 && *(param-1) < 0xFF) goto rNewShowTextEnd
				mov cl, byte ptr[eax - 1]
				cmp cl, 0x20
				jb rNewShowTextOK
				cmp cl, 0xFF
				jne rNewShowTextEnd

			rNewShowTextOK :
				//PostMessage(hwnd, msgId, param, 0)
				mov eax, [esp + OFF_FIRSTPARAM]
				push 0
				push eax
				push FAKE_MESSAGE_ID
				push FAKE_HWND
				mov eax, FAKE_PTR_API
				call dword ptr[eax]

			rNewShowTextEnd :
				pop ecx
				pop ebx
				pop eax

				//这里应为jmp OLD_JCTO_LOADSCENA，先占位
				FAKE_OP2
				FAKE_OP2
				FAKE_OP2
				FAKE_OP2
				FAKE_OP2
			}
		}
	}
}

#endif // !__ZAREMOTEASM_H__
