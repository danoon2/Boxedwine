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

#ifndef __ARMV8BTOPS_FPU_H__
#define __ARMV8BTOPS_FPU_H__

void opFADD_ST0_STj(Armv8btAsm* data);
void opFMUL_ST0_STj(Armv8btAsm* data);
void opFCOM_STi(Armv8btAsm* data);
void opFCOM_STi_Pop(Armv8btAsm* data);
void opFSUB_ST0_STj(Armv8btAsm* data);
void opFSUBR_ST0_STj(Armv8btAsm* data);
void opFDIV_ST0_STj(Armv8btAsm* data);
void opFDIVR_ST0_STj(Armv8btAsm* data);
void opFADD_SINGLE_REAL(Armv8btAsm* data);
void opFMUL_SINGLE_REAL(Armv8btAsm* data);
void opFCOM_SINGLE_REAL(Armv8btAsm* data);
void opFCOM_SINGLE_REAL_Pop(Armv8btAsm* data);
void opFSUB_SINGLE_REAL(Armv8btAsm* data);
void opFSUBR_SINGLE_REAL(Armv8btAsm* data);
void opFDIV_SINGLE_REAL(Armv8btAsm* data);
void opFDIVR_SINGLE_REAL(Armv8btAsm* data);

void opFLD_STi(Armv8btAsm* data);
void opFXCH_STi(Armv8btAsm* data);
void opFNOP(Armv8btAsm* data);
void opFST_STi_Pop(Armv8btAsm* data);
void opFCHS(Armv8btAsm* data);
void opFABS(Armv8btAsm* data);
void opFTST(Armv8btAsm* data);
void opFXAM(Armv8btAsm* data);
void opFLD1(Armv8btAsm* data);
void opFLDL2T(Armv8btAsm* data);
void opFLDL2E(Armv8btAsm* data);
void opFLDPI(Armv8btAsm* data);
void opFLDLG2(Armv8btAsm* data);
void opFLDLN2(Armv8btAsm* data);
void opFLDZ(Armv8btAsm* data);
void opF2XM1(Armv8btAsm* data);
void opFYL2X(Armv8btAsm* data);
void opFPTAN(Armv8btAsm* data);
void opFPATAN(Armv8btAsm* data);
void opFXTRACT(Armv8btAsm* data);
void opFPREM_nearest(Armv8btAsm* data);
void opFDECSTP(Armv8btAsm* data);
void opFINCSTP(Armv8btAsm* data);
void opFPREM(Armv8btAsm* data);
void opFYL2XP1(Armv8btAsm* data);
void opFSQRT(Armv8btAsm* data);
void opFSINCOS(Armv8btAsm* data);
void opFRNDINT(Armv8btAsm* data);
void opFSCALE(Armv8btAsm* data);
void opFSIN(Armv8btAsm* data);
void opFCOS(Armv8btAsm* data);
void opFLD_SINGLE_REAL(Armv8btAsm* data);
void opFST_SINGLE_REAL(Armv8btAsm* data);
void opFST_SINGLE_REAL_Pop(Armv8btAsm* data);
void opFLDENV(Armv8btAsm* data);
void opFLDCW(Armv8btAsm* data);
void opFNSTENV(Armv8btAsm* data);
void opFNSTCW(Armv8btAsm* data);

void opFCMOV_ST0_STj_CF(Armv8btAsm* data);
void opFCMOV_ST0_STj_ZF(Armv8btAsm* data);
void opFCMOV_ST0_STj_CF_OR_ZF(Armv8btAsm* data);
void opFCMOV_ST0_STj_PF(Armv8btAsm* data);
void opFUCOMPP(Armv8btAsm* data);
void opFIADD_DWORD_INTEGER(Armv8btAsm* data);
void opFIMUL_DWORD_INTEGER(Armv8btAsm* data);
void opFICOM_DWORD_INTEGER(Armv8btAsm* data);
void opFICOM_DWORD_INTEGER_Pop(Armv8btAsm* data);
void opFISUB_DWORD_INTEGER(Armv8btAsm* data);
void opFISUBR_DWORD_INTEGER(Armv8btAsm* data);
void opFIDIV_DWORD_INTEGER(Armv8btAsm* data);
void opFIDIVR_DWORD_INTEGER(Armv8btAsm* data);

void opFCMOV_ST0_STj_NCF(Armv8btAsm* data);
void opFCMOV_ST0_STj_NZF(Armv8btAsm* data);
void opFCMOV_ST0_STj_NCF_AND_NZF(Armv8btAsm* data);
void opFCMOV_ST0_STj_NPF(Armv8btAsm* data);
void opFNCLEX(Armv8btAsm* data);
void opFNINIT(Armv8btAsm* data);
void opFUCOMI_ST0_STj(Armv8btAsm* data);
void opFCOMI_ST0_STj(Armv8btAsm* data);
void opFILD_DWORD_INTEGER(Armv8btAsm* data);
void opFISTTP32(Armv8btAsm* data);
void opFIST_DWORD_INTEGER(Armv8btAsm* data);
void opFIST_DWORD_INTEGER_Pop(Armv8btAsm* data);
void opFLD_EXTENDED_REAL(Armv8btAsm* data);
void opFSTP_EXTENDED_REAL(Armv8btAsm* data);

void opFADD_STi_ST0(Armv8btAsm* data);
void opFMUL_STi_ST0(Armv8btAsm* data);
void opFSUBR_STi_ST0(Armv8btAsm* data);
void opFSUB_STi_ST0(Armv8btAsm* data);
void opFDIVR_STi_ST0(Armv8btAsm* data);
void opFDIV_STi_ST0(Armv8btAsm* data);
void opFADD_DOUBLE_REAL(Armv8btAsm* data);
void opFMUL_DOUBLE_REAL(Armv8btAsm* data);
void opFCOM_DOUBLE_REAL(Armv8btAsm* data);
void opFCOM_DOUBLE_REAL_Pop(Armv8btAsm* data);
void opFSUB_DOUBLE_REAL(Armv8btAsm* data);
void opFSUBR_DOUBLE_REAL(Armv8btAsm* data);
void opFDIV_DOUBLE_REAL(Armv8btAsm* data);
void opFDIVR_DOUBLE_REAL(Armv8btAsm* data);

void opFFREE_STi(Armv8btAsm* data);
void opFST_STi(Armv8btAsm* data);
void opFUCOM_STi(Armv8btAsm* data);
void opFUCOM_STi_Pop(Armv8btAsm* data);
void opFLD_DOUBLE_REAL(Armv8btAsm* data);
void opFISTTP64(Armv8btAsm* data);
void opFST_DOUBLE_REAL(Armv8btAsm* data);
void opFST_DOUBLE_REAL_Pop(Armv8btAsm* data);
void opFRSTOR(Armv8btAsm* data);
void opFNSAVE(Armv8btAsm* data);
void opFNSTSW(Armv8btAsm* data);

void opFADD_STi_ST0_Pop(Armv8btAsm* data);
void opFMUL_STi_ST0_Pop(Armv8btAsm* data);
void opFCOMPP(Armv8btAsm* data);
void opFSUBR_STi_ST0_Pop(Armv8btAsm* data);
void opFSUB_STi_ST0_Pop(Armv8btAsm* data);
void opFDIVR_STi_ST0_Pop(Armv8btAsm* data);
void opFDIV_STi_ST0_Pop(Armv8btAsm* data);
void opFIADD_WORD_INTEGER(Armv8btAsm* data);
void opFIMUL_WORD_INTEGER(Armv8btAsm* data);
void opFICOM_WORD_INTEGER(Armv8btAsm* data);
void opFICOM_WORD_INTEGER_Pop(Armv8btAsm* data);
void opFISUB_WORD_INTEGER(Armv8btAsm* data);
void opFISUBR_WORD_INTEGER(Armv8btAsm* data);
void opFIDIV_WORD_INTEGER(Armv8btAsm* data);
void opFIDIVR_WORD_INTEGER(Armv8btAsm* data);

void opFFREEP_STi(Armv8btAsm* data);
void opFNSTSW_AX(Armv8btAsm* data);
void opFUCOMI_ST0_STj_Pop(Armv8btAsm* data);
void opFCOMI_ST0_STj_Pop(Armv8btAsm* data);
void opFILD_WORD_INTEGER(Armv8btAsm* data);
void opFISTTP16(Armv8btAsm* data);
void opFIST_WORD_INTEGER(Armv8btAsm* data);
void opFIST_WORD_INTEGER_Pop(Armv8btAsm* data);
void opFBLD_PACKED_BCD(Armv8btAsm* data);
void opFILD_QWORD_INTEGER(Armv8btAsm* data);
void opFBSTP_PACKED_BCD(Armv8btAsm* data);
void opFISTP_QWORD_INTEGER(Armv8btAsm* data);

#endif