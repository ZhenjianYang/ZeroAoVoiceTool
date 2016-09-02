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

#define OFF_FIRSTPARAM 0x10

#define FAKE_RAZADATA   0x23D6C51A
#define FAKE_MESSAGE_ID	(FAKE_RAZADATA + 1)
#define FAKE_HWND		(FAKE_MESSAGE_ID + 1)
#define FAKE_PTR_API	(FAKE_HWND + 1)

#define FAKE_OP_CALLPTR nop
#define FAKE_CALLPTR_CODE1 0xFF
#define FAKE_CALLPTR_CODE2 0x15


__declspec(naked) void rNewLoadScena()
{
	__asm
	{
		push eax
		push ebx
		push ecx

		//ebx = raZaData
		//这里应为mov ebx, raZaData，先占位
		mov ebx, FAKE_RAZADATA

		//raZaData->aScena = param
		mov eax, [esp + OFF_FIRSTPARAM]
		mov[ebx + OFFZAD_aScena], eax

		//raZaData->cScena++
		mov eax, [ebx + OFFZAD_cScena]
		inc eax
		mov[ebx + OFFZAD_cScena], eax

		//raZaData->aScena1 = raZaData->aScena2 = 0
		//raZaData->cBlock = raZaData->cText = 0
		mov eax, 0
		mov [ebx + OFFZAD_aScena1], eax
		mov [ebx + OFFZAD_aScena2], eax
		mov [ebx + OFFZAD_cBlock], eax
		mov [ebx + OFFZAD_cText], eax

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

__declspec(naked) void rNewLoadScena1()
{
	__asm
	{
		push eax
		push ebx
		push ecx

		//ebx = raZaData
		//这里应为mov ebx, raZaData，先占位
		mov ebx, FAKE_RAZADATA

		//eax = param
		mov eax, [esp + OFF_FIRSTPARAM]

		//if(raZaData->aScena1 == 0) raZaData->aScena1 = eax (param)
		//else raZaData->aScena2 = eax (param)
		mov ecx, [ebx + OFFZAD_aScena1]
		cmp ecx, 0
		je rNewLoadScena1_1
		mov [ebx + OFFZAD_aScena2], eax
		jmp rNewLoadScena1_End
	rNewLoadScena1_1:
		mov [ebx + OFFZAD_aScena1], eax

	rNewLoadScena1_End :

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

__declspec(naked) void rNewLoadBlock()
{
	__asm
	{
		push eax
		push ebx
		push ecx

		//ebx = raZaData
		//这里应为mov ebx, raZaData，先占位
		mov ebx, FAKE_RAZADATA

		//raZaData->aCurBlock = param
		mov eax, [esp + OFF_FIRSTPARAM]
		mov[ebx + OFFZAD_aCurBlock], eax

		//raZaData->cBlock++
		mov eax, [ebx + OFFZAD_cBlock]
		inc eax
		mov[ebx + OFFZAD_cBlock], eax

		//raZaData->cText = 0
		mov eax, 0
		mov[ebx + OFFZAD_cText], eax

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

__declspec(naked) void rNewShowText()
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
		mov cl, [ebx + OFFZAD_disableOriVoice]
		cmp cl, 1
		jne rNewShowTextNoChange

		//eax = param
		mov eax, [esp + OFF_FIRSTPARAM]
		
		//for (ch = A_EMPTYVOICE_LOOPTIMES; ch > 0; ch--, eax++) {
		//	if ([eax] != '#' || [eax + 5] != 'V') continue;
		//	param[1..4] = "9999"; break; }
		mov ch, A_EMPTYVOICE_LOOPTIMES + 1
		dec eax
	rNewShowTextLoop:
		inc eax
		dec ch
		jz rNewShowTextNoChange

		mov cl, byte ptr[eax]
		cmp cl, '#'
		jne rNewShowTextLoop
		mov cl, byte ptr[eax + 5]
		cmp cl, 'V'
		jne rNewShowTextLoop
		mov ecx, A_EMPTYVOICE_HEX
		mov [eax + 1], ecx

	rNewShowTextNoChange:
		//if(raZada.flag != 0) goto rNewShowTextOK
		mov ecx, [ebx + OFFZAD_flag]
		mov dword ptr [ebx + OFFZAD_flag], 0
		cmp ecx, 0
		jne rNewShowTextOK

		//eax = param
		mov eax, [esp + OFF_FIRSTPARAM]
		//if(*param != '#') goto rNewShowTextFlagFailed
		mov cl, byte ptr [eax]
		cmp cl, '#'
		jne rNewShowTextFlagFailed

		//if(param[3] != 'W' && param[4] != 'W') goto rNewShowTextFlagFailed
		mov cl, byte ptr[eax + 4]
		cmp cl, 'W'
		je rNewShowTextFlagOK
		mov cl, byte ptr[eax + 5]
		cmp cl, 'W'
		jne rNewShowTextFlagFailed

	rNewShowTextFlagOK:
		mov dword ptr [ebx + OFFZAD_flag], 1
		jmp rNewShowTextEnd
	
	rNewShowTextFlagFailed:
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
		//eax = param
		mov eax, [esp + OFF_FIRSTPARAM]

		//raZaData->cText++
		//ecx = raZaData->cText
		mov ecx, [ebx + OFFZAD_cText]
		inc ecx
		mov[ebx + OFFZAD_cText], ecx

		cmp ecx, 1
		jne rNewShowTextNotFirst

		//raZaData->aFirstText = eax (param)
		mov[ebx + OFFZAD_aFirstText], eax

	rNewShowTextNotFirst :
		//raZaData->aCurText = eax (param)
		mov[ebx + OFFZAD_aCurText], eax

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

__declspec(naked) void rNewLoadScenaMsg()
{
	__asm
	{
		push eax
		push ebx
		push ecx

		//PostMessage(hwnd, msgId, param, 0)
		mov eax, [esp + OFF_FIRSTPARAM]
		push 0
		push eax
		push FAKE_MESSAGE_ID
		push FAKE_HWND
		mov eax, [FAKE_PTR_API]
		call eax

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
		mov cl, [ebx + OFFZAD_disableOriVoice]
		cmp cl, 1
		jne rNewShowTextNoChange

		//eax = param
		mov eax, [esp + OFF_FIRSTPARAM]

		//for (ch = A_EMPTYVOICE_LOOPTIMES; ch > 0; ch--, eax++) {
		//	if ([eax] != '#' || [eax + 5] != 'V') continue;
		//	param[1..4] = "9999"; break; }
		mov ch, A_EMPTYVOICE_LOOPTIMES + 1
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
		mov ecx, A_EMPTYVOICE_HEX
		mov[eax + 1], ecx

	rNewShowTextNoChange :
		//if(raZada.flag != 0) goto rNewShowTextOK
		mov ecx, [ebx + OFFZAD_flag]
		mov dword ptr[ebx + OFFZAD_flag], 0
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
	mov dword ptr[ebx + OFFZAD_flag], 1
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
		mov eax, [FAKE_PTR_API]
		call eax

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

#endif // !__ZAREMOTEASM_H__
