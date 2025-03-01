/*
 *  Copyright (C) 2012-2025  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __X64OPS_FPU_H__
#define __X64OPS_FPU_H__

/*
void opF2XM1(X64Asm* data);
void opFYL2X(X64Asm* data);
void opFPTAN(X64Asm* data);
void opFPATAN(X64Asm* data);
void opFXTRACT(X64Asm* data);
void opFPREM_nearest(X64Asm* data);
void opFPREM(X64Asm* data);
void opFYL2XP1(X64Asm* data);
void opFSINCOS(X64Asm* data);
void opFRNDINT(X64Asm* data);
void opFSCALE(X64Asm* data);
void opFSIN(X64Asm* data);
void opFCOS(X64Asm* data);
void opFLDENV(X64Asm* data);
void opFLDCW(X64Asm* data);
void opFNSTENV(X64Asm* data);
void opFLD_EXTENDED_REAL(X64Asm* data);
void opFSTP_EXTENDED_REAL(X64Asm* data);
void opFRSTOR(X64Asm* data);
void opFNSAVE(X64Asm* data);
void opFBLD_PACKED_BCD(X64Asm* data);
void opFBSTP_PACKED_BCD(X64Asm* data);
*/

void opFADD_ST0_STj(X64Asm* data, U8 reg);
void opFMUL_ST0_STj(X64Asm* data, U8 reg);
void opFCOM_STi(X64Asm* data, U8 reg);
void opFCOM_STi_Pop(X64Asm* data, U8 reg);
void opFSUB_ST0_STj(X64Asm* data, U8 reg);
void opFSUBR_ST0_STj(X64Asm* data, U8 reg);
void opFDIV_ST0_STj(X64Asm* data, U8 reg);
void opFDIVR_ST0_STj(X64Asm* data, U8 reg);
void opFADD_SINGLE_REAL(X64Asm* data, U8 rm);
void opFMUL_SINGLE_REAL(X64Asm* data, U8 rm);
void opFCOM_SINGLE_REAL(X64Asm* data, U8 rm);
void opFCOM_SINGLE_REAL_Pop(X64Asm* data, U8 rm);
void opFSUB_SINGLE_REAL(X64Asm* data, U8 rm);
void opFSUBR_SINGLE_REAL(X64Asm* data, U8 rm);
void opFDIV_SINGLE_REAL(X64Asm* data, U8 rm);
void opFDIVR_SINGLE_REAL(X64Asm* data, U8 rm);

void opFLD_STi(X64Asm* data, U8 reg);
void opFXCH_STi(X64Asm* data, U8 reg);
void opFNOP(X64Asm* data);
void opFST_STi_Pop(X64Asm* data, U8 reg);
void opFCHS(X64Asm* data);
void opFABS(X64Asm* data);
void opFTST(X64Asm* data);
void opFXAM(X64Asm* data);
void opFLD1(X64Asm* data);
void opFLDL2T(X64Asm* data);
void opFLDL2E(X64Asm* data);
void opFLDPI(X64Asm* data);
void opFLDLG2(X64Asm* data);
void opFLDLN2(X64Asm* data);
void opFLDZ(X64Asm* data);
void opFDECSTP(X64Asm* data);
void opFINCSTP(X64Asm* data);
void opFSQRT(X64Asm* data);
void opFLD_SINGLE_REAL(X64Asm* data, U8 rm);
void opFST_SINGLE_REAL(X64Asm* data, U8 rm);
void opFST_SINGLE_REAL_Pop(X64Asm* data, U8 rm);
void opFNSTCW(X64Asm* data, U8 rm);

void opFCMOV_ST0_STj_CF(X64Asm* data, U8 reg);
void opFCMOV_ST0_STj_ZF(X64Asm* data, U8 reg);
void opFCMOV_ST0_STj_CF_OR_ZF(X64Asm* data, U8 reg);
void opFCMOV_ST0_STj_PF(X64Asm* data, U8 reg);
void opFUCOMPP(X64Asm* data);
void opFIADD_DWORD_INTEGER(X64Asm* data, U8 rm);
void opFIMUL_DWORD_INTEGER(X64Asm* data, U8 rm);
void opFICOM_DWORD_INTEGER(X64Asm* data, U8 rm);
void opFICOM_DWORD_INTEGER_Pop(X64Asm* data, U8 rm);
void opFISUB_DWORD_INTEGER(X64Asm* data, U8 rm);
void opFISUBR_DWORD_INTEGER(X64Asm* data, U8 rm);
void opFIDIV_DWORD_INTEGER(X64Asm* data, U8 rm);
void opFIDIVR_DWORD_INTEGER(X64Asm* data, U8 rm);

void opFCMOV_ST0_STj_NCF(X64Asm* data, U8 reg);
void opFCMOV_ST0_STj_NZF(X64Asm* data, U8 reg);
void opFCMOV_ST0_STj_NCF_AND_NZF(X64Asm* data, U8 reg);
void opFCMOV_ST0_STj_NPF(X64Asm* data, U8 reg);
void opFNCLEX(X64Asm* data);
void opFNINIT(X64Asm* data);
void opFUCOMI_ST0_STj(X64Asm* data, U8 reg);
void opFCOMI_ST0_STj(X64Asm* data, U8 reg);
void opFILD_DWORD_INTEGER(X64Asm* data, U8 rm);
void opFISTTP32(X64Asm* data, U8 rm);
void opFIST_DWORD_INTEGER(X64Asm* data, U8 rm);
void opFIST_DWORD_INTEGER_Pop(X64Asm* data, U8 rm);

void opFADD_STi_ST0(X64Asm* data, U8 reg);
void opFMUL_STi_ST0(X64Asm* data, U8 reg);
void opFSUBR_STi_ST0(X64Asm* data, U8 reg);
void opFSUB_STi_ST0(X64Asm* data, U8 reg);
void opFDIVR_STi_ST0(X64Asm* data, U8 reg);
void opFDIV_STi_ST0(X64Asm* data, U8 reg);
void opFADD_DOUBLE_REAL(X64Asm* data, U8 rm);
void opFMUL_DOUBLE_REAL(X64Asm* data, U8 rm);
void opFCOM_DOUBLE_REAL(X64Asm* data, U8 rm);
void opFCOM_DOUBLE_REAL_Pop(X64Asm* data, U8 rm);
void opFSUB_DOUBLE_REAL(X64Asm* data, U8 rm);
void opFSUBR_DOUBLE_REAL(X64Asm* data, U8 rm);
void opFDIV_DOUBLE_REAL(X64Asm* data, U8 rm);
void opFDIVR_DOUBLE_REAL(X64Asm* data, U8 rm);

void opFFREE_STi(X64Asm* data, U8 reg);
void opFST_STi(X64Asm* data, U8 reg);
void opFUCOM_STi(X64Asm* data, U8 reg);
void opFUCOM_STi_Pop(X64Asm* data, U8 reg);
void opFLD_DOUBLE_REAL(X64Asm* data, U8 rm);
void opFISTTP64(X64Asm* data, U8 rm);
void opFST_DOUBLE_REAL(X64Asm* data, U8 rm);
void opFST_DOUBLE_REAL_Pop(X64Asm* data, U8 rm);
void opFNSTSW(X64Asm* data, U8 rm);

void opFADD_STi_ST0_Pop(X64Asm* data, U8 reg);
void opFMUL_STi_ST0_Pop(X64Asm* data, U8 reg);
void opFCOMPP(X64Asm* data);
void opFSUBR_STi_ST0_Pop(X64Asm* data, U8 reg);
void opFSUB_STi_ST0_Pop(X64Asm* data, U8 reg);
void opFDIVR_STi_ST0_Pop(X64Asm* data, U8 reg);
void opFDIV_STi_ST0_Pop(X64Asm* data, U8 reg);
void opFIADD_WORD_INTEGER(X64Asm* data, U8 rm);
void opFIMUL_WORD_INTEGER(X64Asm* data, U8 rm);
void opFICOM_WORD_INTEGER(X64Asm* data, U8 rm);
void opFICOM_WORD_INTEGER_Pop(X64Asm* data, U8 rm);
void opFISUB_WORD_INTEGER(X64Asm* data, U8 rm);
void opFISUBR_WORD_INTEGER(X64Asm* data, U8 rm);
void opFIDIV_WORD_INTEGER(X64Asm* data, U8 rm);
void opFIDIVR_WORD_INTEGER(X64Asm* data, U8 rm);

void opFFREEP_STi(X64Asm* data, U8 reg);
void opFNSTSW_AX(X64Asm* data);
void opFUCOMI_ST0_STj_Pop(X64Asm* data, U8 reg);
void opFCOMI_ST0_STj_Pop(X64Asm* data, U8 reg);
void opFILD_WORD_INTEGER(X64Asm* data, U8 rm);
void opFISTTP16(X64Asm* data, U8 rm);
void opFIST_WORD_INTEGER(X64Asm* data, U8 rm);
void opFIST_WORD_INTEGER_Pop(X64Asm* data, U8 rm);
void opFILD_QWORD_INTEGER(X64Asm* data, U8 rm);
void opFISTP_QWORD_INTEGER(X64Asm* data, U8 rm);

#endif