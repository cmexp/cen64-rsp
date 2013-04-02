/* ============================================================================
 *  Decoder.h: Instruction decoder definitions.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __RSP__DECODER_H__
#define __RSP__DECODER_H__
#include "Common.h"
#include "Opcodes.h"

#define GET_RD(opcode) ((opcode) >> 11 & 0x1F)
#define GET_RS(opcode) ((opcode) >> 21 & 0x1F)
#define GET_RT(opcode) ((opcode) >> 16 & 0x1F)
#define GET_VS(opcode) ((opcode) >> 21 & 0x1F)
#define GET_VT(opcode) ((opcode) >> 16 & 0x1F)

/* opcode_t->infoFlags */
#define OPCODE_INFO_NONE (0)
#define OPCODE_INFO_BRANCH (1 << 1)         /* Branch insns.          */
#define OPCODE_INFO_BREAK (1 << 2)          /* Break instruction.     */
#define OPCODE_INFO_CP0 (1 << 3)            /* CP0 insns (MF,MT).    */
#define OPCODE_INFO_CP2 (1 << 4)            /* CP2 insns (CT,MF,MT). */
#define OPCODE_INFO_CTC2_WR_VCO (1 << 5)    /* CTC2, writes to VCO.   */
#define OPCODE_INFO_CTC2_WR_VCE (1 << 6)    /* CTC2, writes to VCE.   */
#define OPCODE_INFO_CTC2_WR_VCC (1 << 7)    /* CTC2, writes to VCC.   */
#define OPCODE_INFO_LOAD (1 << 8)           /* Scalar load insns.     */
#define OPCODE_INFO_LWC2 (1 << 9)           /* LWC2 instruction.      */
#define OPCODE_INFO_MFC2 (1 << 10)          /* MFC2 instruction.      */
#define OPCODE_INFO_MTC2 (1 << 11)          /* MTC2 instruction.      */
#define OPCODE_INFO_NEED_RS (1 << 12)       /* Requires rs/vs.        */
#define OPCODE_INFO_NEED_RT (1 << 13)       /* Requires rt/vt.        */
#define OPCODE_INFO_SCALAR (1 << 14)        /* Scalar unit insns.     */
#define OPCODE_INFO_STORE (1 << 15)         /* Scalar store insns.    */
#define OPCODE_INFO_SWC2 (1 << 16)          /* SWC2 instruction.      */
#define OPCODE_INFO_VCOMP (1 << 17)         /* Vector compute insns.  */
#define OPCODE_INFO_WRITE_LR (1 << 18)      /* Can write to link reg. */
#define OPCODE_INFO_WRITE_RD (1 << 19)      /* Writes to rd/vd.       */
#define OPCODE_INFO_WRITE_RT (1 << 20)      /* Writes to rt/vt.       */
#define OPCODE_INFO_XPOSE (1 << 21)         /* Transpose load/store.  */

/* Decoder results. */
struct RSPOpcode{
  enum RSPOpcodeID id;
  uint32_t infoFlags;
};

/* Decoder results. */
struct RSPVOpcode{
  enum RSPVOpcodeID id;
  uint32_t infoFlags;
};

/* Escape table data. */
struct RSPOpcodeEscape {
  const struct RSPOpcode *table;
  uint32_t shift, mask;
};

const struct RSPOpcode* RSPDecodeInstruction(uint32_t);
const struct RSPVOpcode* RSPDecodeVectorInstruction(uint32_t);
void RSPInvalidateOpcode(struct RSPOpcode *);
void RSPInvalidateVectorOpcode(struct RSPVOpcode *);

#endif

