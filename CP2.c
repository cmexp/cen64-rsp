/* ============================================================================
 *  CP2.c: RSP Coprocessor #2.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Common.h"
#include "CP2.h"
#include "Decoder.h"
#include "Opcodes.h"
#include "ReciprocalROM.h"

#ifdef __cplusplus
#include <cstring>
#else
#include <string.h>
#endif

#ifdef USE_SSE
#include <smmintrin.h>

/* ============================================================================
 *  RSPGetVectorOperands: Builds and returns the proper configuration of the
 *  `vt` vector for instructions that require the use of a element specifier.
 * ========================================================================= */
static __m128i
RSPGetVectorOperands(__m128i vt, unsigned element) {
  static const uint8_t VectorOperandsArray[16][16] = {
    /* pshufb (_mm_shuffle_epi8) keys. */
    /* -- */ {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF},
    /* -- */ {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF},
    /* 0q */ {0x0,0x1,0x0,0x1,0x4,0x5,0x4,0x5,0x8,0x9,0x8,0x9,0xC,0xD,0xC,0xD},
    /* 1q */ {0x2,0x3,0x2,0x3,0x6,0x7,0x6,0x7,0xA,0xB,0xA,0xB,0xE,0xF,0xE,0xF},
    /* 0h */ {0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x1,0x8,0x9,0x8,0x9,0x8,0x9,0x8,0x9},
    /* 1h */ {0x2,0x3,0x2,0x3,0x2,0x3,0x2,0x3,0xA,0xB,0xA,0xB,0xA,0xB,0xA,0xB},
    /* 2h */ {0x4,0x5,0x4,0x5,0x4,0x5,0x4,0x5,0xC,0xD,0xC,0xD,0xC,0xD,0xC,0xD},
    /* 3h */ {0x6,0x7,0x6,0x7,0x6,0x7,0x6,0x7,0xE,0xF,0xE,0xF,0xE,0xF,0xE,0xF},
    /* 0w */ {0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x1},
    /* 1w */ {0x2,0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2,0x3},
    /* 2w */ {0x4,0x5,0x4,0x5,0x4,0x5,0x4,0x5,0x4,0x5,0x4,0x5,0x4,0x5,0x4,0x5},
    /* 3w */ {0x6,0x7,0x6,0x7,0x6,0x7,0x6,0x7,0x6,0x7,0x6,0x7,0x6,0x7,0x6,0x7},
    /* 4w */ {0x8,0x9,0x8,0x9,0x8,0x9,0x8,0x9,0x8,0x9,0x8,0x9,0x8,0x9,0x8,0x9},
    /* 5w */ {0xA,0xB,0xA,0xB,0xA,0xB,0xA,0xB,0xA,0xB,0xA,0xB,0xA,0xB,0xA,0xB},
    /* 6w */ {0xC,0xD,0xC,0xD,0xC,0xD,0xC,0xD,0xC,0xD,0xC,0xD,0xC,0xD,0xC,0xD},
    /* 7w */ {0xE,0xF,0xE,0xF,0xE,0xF,0xE,0xF,0xE,0xF,0xE,0xF,0xE,0xF,0xE,0xF}
  };

  __m128i key = _mm_load_si128((__m128i*) VectorOperandsArray[element]);
  return _mm_shuffle_epi8(vt, key);
}

/* ============================================================================
 *  SSE lacks nand, nor, and nxor (really, xnor), so define them manually.
 * ========================================================================= */
static __m128i _mm_nand_si128(__m128i a, __m128i b) {
  __m128i mask = _mm_cmpeq_epi8(_mm_setzero_si128(), _mm_setzero_si128());
  return _mm_xor_si128(_mm_and_si128(a, b), mask);
}

static __m128i _mm_nor_si128(__m128i a, __m128i b) {
  __m128i mask = _mm_cmpeq_epi8(_mm_setzero_si128(), _mm_setzero_si128());
  return _mm_xor_si128(_mm_or_si128(a, b), mask);
}

static __m128i _mm_nxor_si128(__m128i a, __m128i b) {
  __m128i mask = _mm_cmpeq_epi8(_mm_setzero_si128(), _mm_setzero_si128());
  return _mm_xor_si128(_mm_xor_si128(a, b), mask);
}

#endif

/* ============================================================================
 *  Instruction: VABS (Vector Absolute Value of Short Elements)
 *  TODO: Comment and use more sensible variable names within.
 * ========================================================================= */
void
RSPVABS(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;

  uint16_t *vd = cp2->regs[vdRegister].slices;
  uint16_t *acc = cp2->accumulatorLow.slices;

#ifdef USE_SSE
  __m128i lessThanMask, equalsMask;
  __m128i vsReg, vtReg, vdReg;

  vtReg = _mm_load_si128((__m128i*) vt);
  vsReg = _mm_load_si128((__m128i*) vs);
  vtReg = RSPGetVectorOperands(vtReg, element);

  /* If the VS slice == 0, zero out the VT slice. */
  equalsMask = _mm_cmpeq_epi16(vsReg, _mm_setzero_si128());
  equalsMask = _mm_cmpeq_epi16(equalsMask, equalsMask);
  vtReg = _mm_and_si128(vtReg, equalsMask);
  equalsMask = _mm_abs_epi16(vtReg);
  equalsMask = _mm_srli_epi16(equalsMask, 15);
  vdReg = _mm_xor_si128(vtReg, equalsMask);
  vdReg = _mm_sub_epi16(_mm_setzero_si128(), vdReg);

  /* Now mix all three (> 0, == 0, < 0) cases together. */
  lessThanMask = _mm_cmplt_epi16(vsReg, _mm_setzero_si128());
  vdReg = _mm_and_si128(vdReg, lessThanMask);
  lessThanMask = _mm_cmpeq_epi16(lessThanMask, _mm_setzero_si128());
  vtReg = _mm_and_si128(vtReg, lessThanMask);
  vdReg = _mm_or_si128(vdReg, vtReg);

  _mm_store_si128((__m128i*) vd, vdReg);
  _mm_store_si128((__m128i*) acc, vdReg);
#else
#warning "Unimplemented function: RSPVABS (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VADD (Vector Add of Short Elements)
 * ========================================================================= */
void
RSPVADD(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;

  uint16_t *vd = cp2->regs[vdRegister].slices;
  uint16_t *acc = cp2->accumulatorLow.slices;
  uint16_t *vco = cp2->carryOut.slices;

#ifdef USE_SSE
  __m128i minSlices, maxSlices, satSum, unsatSum;
  __m128i vtReg, vsReg, carryIn, carryOut;

  vtReg = _mm_load_si128((__m128i*) vt);
  vsReg = _mm_load_si128((__m128i*) vs);
  vtReg = RSPGetVectorOperands(vtReg, element);

  /* Load the current carries, clear it out. */
  carryIn = _mm_load_si128((__m128i*) vco);
  carryOut = _mm_setzero_si128();

  /* Use unsaturated arithmetic for the accumulator. */
  unsatSum = _mm_add_epi16(vsReg, vtReg);
  unsatSum = _mm_add_epi16(unsatSum, carryIn);

  /* Saturate the sum, including the carry. */
  minSlices = _mm_min_epi16(vsReg, vtReg);
  maxSlices = _mm_max_epi16(vsReg, vtReg);
  minSlices = _mm_adds_epi16(minSlices, carryIn);
  satSum = _mm_adds_epi16(minSlices, maxSlices);

  _mm_store_si128((__m128i*) vco, carryOut);
  _mm_store_si128((__m128i*) acc, unsatSum);
  _mm_store_si128((__m128i*) vd, satSum);
#else
#warning "Unimplemented function: RSPVADD (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VADDC (Vector Add of Short Elements with Carry)
 * ========================================================================= */
void
RSPVADDC(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;

  uint16_t *vd = cp2->regs[vdRegister].slices;
  uint16_t *acc = cp2->accumulatorLow.slices;
  uint16_t *vco = cp2->carryOut.slices;

#ifdef USE_SSE
  __m128i satSum, unsatSum, carryOut, equalMask;
  __m128i vtReg = _mm_load_si128((__m128i*) vt);
  __m128i vsReg = _mm_load_si128((__m128i*) vs);
  vtReg = RSPGetVectorOperands(vtReg, element);

  satSum = _mm_adds_epu16(vsReg, vtReg);
  unsatSum = _mm_add_epi16(vsReg, vtReg);

  equalMask = _mm_cmpeq_epi16(satSum, unsatSum);
  equalMask = _mm_cmpeq_epi16(equalMask, _mm_setzero_si128());
  carryOut = _mm_packs_epi16(equalMask, equalMask);

  _mm_store_si128((__m128i*) vco, carryOut);
  _mm_store_si128((__m128i*) acc, unsatSum);
  _mm_store_si128((__m128i*) vd, unsatSum);
#else
#warning "Unimplemented function: RSPVADDC (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VAND (Vector AND of Short Elements)
 * ========================================================================= */
void
RSPVAND(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;
  uint16_t *acc = cp2->accumulatorLow.slices;

#ifdef USE_SSE
  __m128i vtReg = _mm_load_si128((__m128i*) vt);
  __m128i vsReg = _mm_load_si128((__m128i*) vs);
  __m128i vdReg;

  vtReg = RSPGetVectorOperands(vtReg, element);

  vdReg = _mm_and_si128(vtReg, vsReg);
  _mm_store_si128((__m128i*) acc, vdReg);
  _mm_store_si128((__m128i*) vd, vdReg);
#else
#warning "Unimplemented function: RSPVAND (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VCH (Vector Select Clip Test High)
 * ========================================================================= */
void
RSPVCH(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VCH.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VCL (Vector Select Clip Test Low)
 * ========================================================================= */
void
RSPVCL(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VCL.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VCR (Vector Select Crimp Test Low)
 * ========================================================================= */
void
RSPVCR(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VCR.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VEQ (Vector Select Equal)
 * ========================================================================= */
void
RSPVEQ(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VEQ.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VGE (Vector Select Greater Than or Equal)
 * ========================================================================= */
void
RSPVGE(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VGE.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VINV (Invalid Vector Operation)
 * ========================================================================= */
void
RSPVINV(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VLT (Vector Select Less Than)
 * ========================================================================= */
void
RSPVLT(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VLT.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMACF (Vector Multiply-Accumulate of Signed Fractions)
 * ========================================================================= */
void
RSPVMACF(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented function: VMACF.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMACQ (Vector Accumulator Oddification)
 * ========================================================================= */
void
RSPVMACQ(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented function: VMACQ.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMACU (Vector Multiply-Accumulate of Unsigned Fractions)
 * ========================================================================= */
void
RSPVMACU(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented function: VMACU.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMADH (Vector Multiply-Accumulate of High Partial Products)
 * ========================================================================= */
void
RSPVMADH(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented function: VMADH.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMADL (Vector Multiply-Accumulate of Lower Partial Products).
 * ========================================================================= */
void
RSPVMADL(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented function: VMADL.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMADM (Vector Multiply-Accumulate of Mid Partial Products)
 * ========================================================================= */
void
RSPVMADM(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented function: VMADM.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMADN (Vector Multiply-Accumulate of Mid Partial Products)
 * ========================================================================= */
void
RSPVMADN(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented function: VMADN.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMOV (Vector Element Scalar Move)
 * ========================================================================= */
void
RSPVMOV(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vtRegister = iw >> 16 & 0x1F;
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned de = iw >> 11 & 0x7;
  unsigned e = iw >> 21 & 0x7;

  const uint16_t *vt = cp2->regs[vtRegister].slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

  memcpy(cp2->accumulatorLow.slices, vt, sizeof(__m128i));
  vd[de] = vt[e];

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VMRG (Vector Select Merge)
 * ========================================================================= */
void
RSPVMRG(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VMRG.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMUDH (Vector Multiply of High Partial Products)
 * ========================================================================= */
void
RSPVMUDH(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VMUDH.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMUDL (Vector Multiply of Low Partial Products)
 * ========================================================================= */
void
RSPVMUDL(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VMUDL.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMUDM (Vector Multiply of Middle Partial Products)
 * ========================================================================= */
void
RSPVMUDM(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VMUDM.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMUDN (Vector Multiply of Middle Partial Products)
 * ========================================================================= */
void
RSPVMUDN(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VMUDN.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMULF (Vector Multiply of Signed Fractions)
 * ========================================================================= */
void
RSPVMULF(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VMULF.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMULQ (Vector Multiply MPEG Quantization)
 * ========================================================================= */
void
RSPVMULQ(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VMULQ.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VMULU (Vector Multiply of Unsigned Fractions)
 * ========================================================================= */
void
RSPVMULU(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VMULU.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VNAND (Vector NAND of Short Elements)
 * ========================================================================= */
void
RSPVNAND(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *acc = cp2->accumulatorLow.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i vtReg = _mm_load_si128((__m128i*) vt);
  __m128i vsReg = _mm_load_si128((__m128i*) vs);
  __m128i vdReg;

  vtReg = RSPGetVectorOperands(vtReg, element);

  vdReg = _mm_nand_si128(vtReg, vsReg);
  _mm_store_si128((__m128i*) acc, vdReg);
  _mm_store_si128((__m128i*) vd, vdReg);
#else
#warning "Unimplemented function: RSPVNAND (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VNE (Vector Select Not Equal)
 * ========================================================================= */
void
RSPVNE(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VNE.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VNOP (Vector No Operation)
 * ========================================================================= */
void
RSPVNOP(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VNOR (Vector NOR of Short Elements)
 * ========================================================================= */
void
RSPVNOR(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *acc = cp2->accumulatorLow.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i vtReg = _mm_load_si128((__m128i*) vt);
  __m128i vsReg = _mm_load_si128((__m128i*) vs);
  __m128i vdReg;

  vtReg = RSPGetVectorOperands(vtReg, element);

  vdReg = _mm_nor_si128(vtReg, vsReg);
  _mm_store_si128((__m128i*) acc, vdReg);
  _mm_store_si128((__m128i*) vd, vdReg);
#else
#warning "Unimplemented function: RSPVNOR (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VOR (Vector OR of Short Elements)
 * ========================================================================= */
void
RSPVOR(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *acc = cp2->accumulatorLow.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i vtReg = _mm_load_si128((__m128i*) vt);
  __m128i vsReg = _mm_load_si128((__m128i*) vs);
  __m128i vdReg;

  vtReg = RSPGetVectorOperands(vtReg, element);

  vdReg = _mm_or_si128(vtReg, vsReg);
  _mm_store_si128((__m128i*) acc, vdReg);
  _mm_store_si128((__m128i*) vd, vdReg);
#else
#warning "Unimplemented function: RSPVOR (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VNXOR (Vector NXOR of Short Elements)
 * ========================================================================= */
void
RSPVNXOR(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *acc = cp2->accumulatorLow.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i vtReg = _mm_load_si128((__m128i*) vt);
  __m128i vsReg = _mm_load_si128((__m128i*) vs);
  __m128i vdReg;

  vtReg = RSPGetVectorOperands(vtReg, element);

  vdReg = _mm_nxor_si128(vtReg, vsReg);
  _mm_store_si128((__m128i*) acc, vdReg);
  _mm_store_si128((__m128i*) vd, vdReg);
#else
#warning "Unimplemented function: RSPVNXOR (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VRCP (Vector Element Scalar Reciprocal (Single Precision))
 * ========================================================================= */
void
RSPVRCP(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VRCP.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VRCPH (Vector Element Scalar Reciprocal (Double Prec. High))
 * ========================================================================= */
void
RSPVRCPH(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned delement = iw >> 11 & 0x7;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;
  uint16_t *acc = cp2->accumulatorLow.slices;

#ifdef USE_SSE
  __m128i vtReg = _mm_load_si128((__m128i*) vt);
  vtReg = RSPGetVectorOperands(vtReg, element);
#endif

  cp2->reciprocalIn = (uint32_t) vt[element] << 16;
  cp2->shouldUseDoublePrecision = 1;

#ifdef USE_SSE
  _mm_store_si128((__m128i*) acc, vtReg);
#else
#warning "Unimplemented function: RSPVRCPL (No SSE)."
#endif

  vd[delement] = cp2->reciprocalResult >> 16;
  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VRCPL (Vector Element Scalar Reciprocal (Double Prec. Low))
 * ========================================================================= */
void
RSPVRCPL(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned delement = iw >> 11 & 0x7;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;
  uint16_t *acc = cp2->accumulatorLow.slices;

  uint32_t input, data, result;
  unsigned index, shift = 0;
  int32_t divIn;

#ifdef USE_SSE
  __m128i vtReg = _mm_load_si128((__m128i*) vt);
  vtReg = RSPGetVectorOperands(vtReg, element);
#endif

  /* Take the absolute value of component. */
  /* We'll restore the sign when done. */
  if (cp2->shouldUseDoublePrecision) {
    divIn = cp2->reciprocalIn | vt[element];
    input = divIn;

    if (divIn < 0) {
      input = (divIn >= -32768)
        ? -input
        : ~input;
    }
  }

  else {
    divIn = (int16_t) vt[element];
    input = divIn;

    if (divIn < 0)
      input = -input;
  }

  /* Get the amount to shift by. */
  if (input) {
    unsigned i;

    for (i = 0; i < 32; i++) {
      if (input & (1<<(31 - i))) {
        shift = i;
        break;
      }
    }
  }

  else
    shift = (cp2->shouldUseDoublePrecision) ? 0x00 : 0x10;

  /* Compute the result. */
  index = ((input << shift) & 0x7FC00000) >> 22;
  data = ReciprocalLUT[index];
  result = (0x40000000 | (data << 14)) >> ((~shift) & 0x1F);

  /* Restore the sign. */
  if (divIn < 0)
    result = ~result;

  /* Handle corner cases. */
  if (divIn == 0)
    result = 0x7FFFFFFF;
  else if ((uint32_t) divIn == 0xFFFF8000U)
    result = 0xFFFF0000;

  cp2->shouldUseDoublePrecision = 0;
  cp2->reciprocalResult = result;
  vd[delement] = result;

#ifdef USE_SSE
  _mm_store_si128((__m128i*) acc, vtReg);
#else
#warning "Unimplemented function: RSPVRCPL (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VRNDN (Vector Accumulator DCT Rounding (Negative))
 * ========================================================================= */
void
RSPVRNDN(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VRNDN.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VRNDP (Vector Accumulator DCT Rounding (Positive))
 * ========================================================================= */
void
RSPVRNDP(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VRNDP.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VRSQ (Vector Element Scalar SQRT Reciprocal)
 * ========================================================================= */
void
RSPVRSQ(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VRSQ.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VRSQH (Vector Element Scalar SQRT Reciprocal (Double Prec. H))
 * ========================================================================= */
void
RSPVRSQH(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VRSQH.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VRSQL (Vector Element Scalar SQRT Reciprocal (Double Prec. L))
 * ========================================================================= */
void
RSPVRSQL(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VRSQL.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VSAR (Vector Accumulator Read (and Write))
 * ========================================================================= */
void
RSPVSAR(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VSAR.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VSUB (Vector Subtraction of Short Elements)
 *  TODO: Test this with edge cases on optimized builds.
 * ========================================================================= */
void
RSPVSUB(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VSUB.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VSUBC (Vector Subtraction of Short Elements with Carry)
 * ========================================================================= */
void
RSPVSUBC(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VSUBC.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VXOR (Vector XOR of Short Elements)
 * ========================================================================= */
void
RSPVXOR(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *acc = cp2->accumulatorLow.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i vtReg = _mm_load_si128((__m128i*) vt);
  __m128i vsReg = _mm_load_si128((__m128i*) vs);
  __m128i vdReg;

  vtReg = RSPGetVectorOperands(vtReg, element);

  vdReg = _mm_xor_si128(vtReg, vsReg);
  _mm_store_si128((__m128i*) acc, vdReg);
  _mm_store_si128((__m128i*) vd, vdReg);
#else
#warning "Unimplemented function: RSPVXOR (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  RSPInitCP2: Initializes the co-processor.
 * ========================================================================= */
void
RSPInitCP2(struct RSPCP2 *cp2) {
  debug("Initializing CP2.");
  memset(cp2, 0, sizeof(*cp2));
}

/* ============================================================================
 *  RSPCycleCP2: Vector execute/multiply/accumulate stages.
 * ========================================================================= */
void
RSPCycleCP2(struct RSPCP2 *cp2) {
  cp2->locked[cp2->accStageDest] = false;     /* "WB" */
  cp2->accStageDest = cp2->mulStageDest;      /* "DF" */
  cp2->mulStageDest = 0;                      /* "EX" */

  RSPVectorFunctionTable[cp2->opcode.id](cp2, cp2->iw);

#ifndef NDEBUG
  cp2->counts[cp2->opcode.id]++;
#endif
}

/* ============================================================================
 *  RSPCP2GetAccumulator: Fetches an accumulator and returns it.
 * ========================================================================= */
#ifndef NDEBUG
void
RSPCP2GetAccumulator(const struct RSPCP2 *cp2, unsigned reg, uint16_t *acc) {
#ifdef USE_SSE
  acc[0] = cp2->accumulatorLow.slices[reg];
  acc[1] = cp2->accumulatorMid.slices[reg];
  acc[2] = cp2->accumulatorHigh.slices[reg];
#else
#warning "Unimplemented function: RSPCP2GetAccumulator (No SSE)."
#endif
}
#endif

/* ============================================================================
 *  RSPCP2GetCarryOut: Fetches the carry-out and returns it.
 * ========================================================================= */
#ifndef NDEBUG
uint16_t
RSPCP2GetCarryOut(const struct RSPCP2 *cp2) {
#ifdef USE_SSE
  __m128i carryOut = _mm_load_si128((__m128i*) cp2->carryOut.slices);
  return _mm_movemask_epi8(carryOut);
#else
#warning "Unimplemented function: RSPCP2GetCarryOut (No SSE)."
  return 0;
#endif
}
#endif

