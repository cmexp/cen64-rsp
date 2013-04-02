/* ============================================================================
 *  Decoder.c: Instruction decoder.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Common.h"
#include "Decoder.h"
#include "Opcodes.h"

/* These will only touch processor cachelines */
/* if an invalid/undefined instruction is used. */
static const struct RSPOpcode InvalidOpcodeTable[64] = {
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

/* ============================================================================
 *  Escaped opcode table: Special.
 *
 *      31---------26-----------------------------------------5---------0
 *      | SPECIAL/6 |                                         | OPCODE/6|
 *      ------6----------------------------------------------------6-----
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *  000 |  SLL  |  ---  |  SRL  |  SRA  | SLLV  |  ---  | SRLV  | SRAV  |
 *  001 |  JR   |  JALR |  ---  |  ---  |  ---  | BREAK |  ---  |  ---  |
 *  010 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *  011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *  100 |  ADD  | ADDU  |  SUB  | SUBU  |  AND  |  OR   |  XOR  |  NOR  |
 *  101 |  ---  |  ---  |  SLT  | SLTU  |  ---  |  ---  |  ---  |  ---  |
 *  110 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *  111 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 *
 * ========================================================================= */
static const struct RSPOpcode SpecialOpcodeTable[64] = {
  {SLL},     {INVALID}, {SRL},     {SRA},
  {SLLV},    {INVALID}, {SRLV},    {SRAV},
  {JR},      {JALR},    {INVALID}, {INVALID},
  {INVALID}, {BREAK},   {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {ADD},     {ADD},     {SUB},     {SUB},
  {AND},     {OR},      {XOR},     {NOR},
  {INVALID}, {INVALID}, {SLT},     {SLTU},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

/* ============================================================================
 *  Escaped opcode table: RegImm.
 *
 *      31---------26----------20-------16------------------------------0
 *      | REGIMM/6  |          |  FMT/5  |                              |
 *      ------6---------------------5------------------------------------
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *   00 | BLTZ  | BGEZ  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *   01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *   10 |BLTZAL |BGEZAL |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *   11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 *
 * ========================================================================= */
static const struct RSPOpcode RegImmOpcodeTable[32] = {
  {BLTZ},    {BGEZ},    {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {BLTZAL},  {BGEZAL},  {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

/* ============================================================================
 *  Escaped opcode table: CP0.
 *
 *      31--------26-25------21 ----------------------------------------0
 *      |  COP0/6   |  FMT/5  |                                         |
 *      ------6----------5-----------------------------------------------
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *   00 | MFC0  |  ---  |  ---  |  ---  | MTC0  |  ---  |  ---  |  ---  |
 *   01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *   10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *   11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 * ========================================================================= */
static const struct RSPOpcode COP0OpcodeTable[32] = {
  {MFC0},    {INVALID}, {INVALID}, {INVALID},
  {MTC0},    {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

/* ============================================================================
 *  Escaped opcode table: COP2.
 *
 *  A FMT of 0b11xxx => Lookup in VectorOpcodeTable.
 *  However, we let the the vector execution unit handle this.
 *
 *      31--------26-25------21 ----------------------------------------0
 *      |  COP2/6   |  FMT/5  |                                         |
 *      ------6----------5-----------------------------------------------
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *   00 | MFC2  |  ---  | CFC2  |  ---  | MTC2  |  ---  | CTC2  |  ---  |
 *   01 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *   10 | *VECT | *VECT | *VECT | *VECT | *VECT | *VECT | *VECT | *VECT |
 *   11 | *VECT | *VECT | *VECT | *VECT | *VECT | *VECT | *VECT | *VECT |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 *
 * ========================================================================= */
static const struct RSPOpcode COP2OpcodeTable[32] = {
  {MFC2},    {INVALID}, {CFC2},    {INVALID},
  {MTC2},    {INVALID}, {CTC2},    {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {VECT},    {VECT},    {VECT},    {VECT},
  {VECT},    {VECT},    {VECT},    {VECT},
  {VECT},    {VECT},    {VECT},    {VECT},
  {VECT},    {VECT},    {VECT},    {VECT}
};

/* ============================================================================
 *  Escaped opcode table: COP2/VECTOR.
 *
 *  Vector opcodes: Instr. encoded by the function field when opcode = COP2.
 *
 *      31---------26---25------------------------------------5---------0
 *      |  = COP2   | 1 |                                     |  FMT/6  |
 *      ------6-------1--------------------------------------------6-----
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *  000 | VMULF | VMULU | VRNDP | VMULQ | VMUDL | VMUDM | VMUDN | VMUDH |
 *  001 | VMACF | VMACU | VRNDN | VMACQ | VMADL | VMADM | VMADN | VMADH |
 *  010 | VADD  | VSUB  |  ---  | VABS  | VADDC | VSUBC |  ---  |  ---  |
 *  011 |  ---  |  ---  |  ---  |  ---  |  ---  | VSAR  |  ---  |  ---  |
 *  100 |  VLT  |  VEQ  |  VNE  |  VGE  |  VCL  |  VCH  |  VCR  | VMRG  |
 *  101 | VAND  | VNAND |  VOR  | VNOR  | VXOR  | VNXOR |  ---  |  ---  |
 *  110 | VRCP  | VRCPL | VRCPH | VMOV  | VRSQ  | VRSQL | VRSQH | VNOP  |
 *  111 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 *
 * ========================================================================= */
static const struct RSPVOpcode COP2VectorOpcodeTable[64] = {
  {VMULF},    {VMULU},    {VRNDP},    {VMULQ},
  {VMUDL},    {VMUDM},    {VMUDN},    {VMUDH},
  {VMACF},    {VMACU},    {VRNDN},    {VMACQ},
  {VMADL},    {VMADM},    {VMADN},    {VMADH},
  {VADD},     {VSUB},     {VINVALID}, {VABS},
  {VADDC},    {VSUBC},    {VINVALID}, {VINVALID},
  {VINVALID}, {VINVALID}, {VINVALID}, {VINVALID},
  {VINVALID}, {VSAR},     {VINVALID}, {VINVALID},
  {VLT},      {VEQ},      {VNE},      {VGE},
  {VCL},      {VCH},      {VCR},      {VMRG},
  {VAND},     {VNAND},    {VOR},      {VNOR},
  {VXOR},     {VNXOR},    {VINVALID}, {VINVALID},
  {VRCP},     {VRCPL},    {VRCPH},    {VMOV},
  {VRSQ},     {VRSQL},    {VRSQH},    {VNOP},
  {VINVALID}, {VINVALID}, {VINVALID}, {VINVALID},
  {VINVALID}, {VINVALID}, {VINVALID}, {VINVALID},
};

/* ============================================================================
 *  Escaped opcode table: LWC2.
 *
 *      31---------26-------------------15-------11---------------------0
 *      |   LWC2/6  |                   | FUNC/5 |                      |
 *      ------6-----------------------------5----------------------------
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *   00 |  LBV  |  LSV  |  LLV  |  LDV  |  LQV  |  LRV  |  LPV  |  LUV  |
 *   01 |  LHV  |  LFV  |  ---  |  LTV  |  ---  |  ---  |  ---  |  ---  |
 *   10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *   11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 *
 * ========================================================================= */
static const struct RSPOpcode LWC2OpcodeTable[32] = {
  {LBV},     {LSV},     {LLV},     {LDV},
  {LQV},     {LRV},     {LPV},     {LUV},
  {LHV},     {LFV},     {INVALID}, {LTV},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

/* ============================================================================
 *  Escaped opcode table: SWC2.
 *
 *      31---------26-------------------15-------11---------------------0
 *      |   SWC2/6  |                   | FMT/5  |                      |
 *      ------6-----------------------------5----------------------------
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *   00 |  SBV  |  SSV  |  SLV  |  SDV  |  SQV  |  SRV  |  SPV  |  SUV  |
 *   01 |  SHV  |  SFV  |  SWV  |  STV  |  ---  |  ---  |  ---  |  ---  |
 *   10 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *   11 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 *
 * ========================================================================= */
static const struct RSPOpcode SWC2OpcodeTable[32] = {
  {SBV},     {SSV},     {SLV},     {SDV},
  {SQV},     {SRV},     {SPV},     {SUV},
  {SHV},     {SFV},     {SWV},     {STV},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

/* ============================================================================
 *  First-order opcode table.
 *
 *  An OPCODE of 0b000000   => Lookup in SpecialOpcodeTable.
 *  An OPCODE of 0b000001   => Lookup in RegImmOpcodeTable.
 *  An OPCODE of 0b010000   => Lookup in COP0OpcodeTable.
 *  An OPCODE of 0b010010|0 => Lookup in COP2OpcodeTable.
 *  An OPCODE of 0b010010|1 => Lookup in COP2VectorTable.
 *  An OPCODE of 0b110010   => Lookup in LWC2OpcodeTable.
 *  An OPCODE of 0b111010   => Lookup in SWC2OpcodeTable.
 *
 *      31---------26---------------------------------------------------0
 *      |  OPCODE/6 |                                                   |
 *      ------6----------------------------------------------------------
 *      |--000--|--001--|--010--|--011--|--100--|--101--|--110--|--111--|
 *  000 | *SPEC | *RGIM |   J   |  JAL  |  BEQ  |  BNE  | BLEZ  | BGTZ  |
 *  001 | ADDI  | ADDIU | SLTI  | SLTIU | ANDI  |  ORI  | XORI  |  LUI  |
 *  010 | *COP0 |  ---  | *COP2 |  ---  |  ---  |  ---  |  ---  |  ---  |
 *  011 |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |  ---  |
 *  100 |  LB   |  LH   |  ---  |  LW   |  LBU  |  LHU  |  ---  |  ---  |
 *  101 |  SB   |  SH   |  ---  |  SW   |  ---  |  ---  |  ---  |  ---  |
 *  110 |  ---  |  ---  | *LWC2 |  ---  |  ---  |  ---  |  ---  |  ---  |
 *  111 |  ---  |  ---  | *SWC2 |  ---  |  ---  |  ---  |  ---  |  ---  |
 *      |-------|-------|-------|-------|-------|-------|-------|-------|
 *
 * ========================================================================= */
static const struct RSPOpcode OpcodeTable[64] = {
  {INVALID}, {INVALID}, {J},       {JAL},
  {BEQ},     {BNE},     {BLEZ},    {BGTZ},
  {ADDI},    {ADDI},    {SLTI},    {SLTIU},
  {ANDI},    {ORI},     {XORI},    {LUI},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {LB},      {LH},      {INVALID}, {LW},
  {LBU},     {LHU},     {INVALID}, {INVALID},
  {SB},      {SH},      {INVALID}, {SW},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID},
  {INVALID}, {INVALID}, {INVALID}, {INVALID}
};

/* Escaped table listings. Most of these will never */
/* see a processor cacheline, so not much waste here. */
static const struct RSPOpcodeEscape EscapeTable[64] = {
  {SpecialOpcodeTable,  0, 0x3F}, {RegImmOpcodeTable,  16, 0x1F},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},

  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},

  {COP0OpcodeTable,    21, 0x1F}, {InvalidOpcodeTable,  0,    0},
  {COP2OpcodeTable,    21, 0x1F}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},

  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},

  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},

  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {LWC2OpcodeTable,    11, 0x1F}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},

  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {SWC2OpcodeTable,    11, 0x1F}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
  {InvalidOpcodeTable,  0,    0}, {InvalidOpcodeTable,  0,    0},
};

/* ============================================================================
 *  RSPDecodeInstruction: Looks up an instruction in the opcode table.
 *  Instruction words are assumed to be in big-endian byte order.
 * ========================================================================= */
const struct RSPOpcode*
RSPDecodeInstruction(uint32_t iw) {
  const struct RSPOpcode *opcode = &OpcodeTable[iw >> 26];

  if (likely(opcode->id == RSP_OPCODE_INV)) {
    const struct RSPOpcodeEscape *escape = &EscapeTable[iw >> 26];
    uint8_t index = iw >> escape->shift & escape->mask;
    return &escape->table[index];
  }

  return opcode;
}

/* ============================================================================
 *  RSPDecodeVectorInstruction: Looks up a v. instruction in the opcode table. 
 *  Instruction words are assumed to be in big-endian byte order.
 * ========================================================================= */
const struct RSPVOpcode*
RSPDecodeVectorInstruction(uint32_t iw) {
  return &COP2VectorOpcodeTable[iw & 0x3F];
}

/* ============================================================================
 *  RSPInvalidateOpcode: Invalidates an opcode.
 * ========================================================================= */
void
RSPInvalidateOpcode(struct RSPOpcode *opcode) {
  *opcode = OpcodeTable[0];
}

/* ============================================================================
 *  RSPInvalidateVectorOpcode: Invalidates a vector opcode.
 * ========================================================================= */
void
RSPInvalidateVectorOpcode(struct RSPVOpcode *opcode) {
  *opcode = COP2VectorOpcodeTable[63];
}

