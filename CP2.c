/* ============================================================================
 *  CP2.c: RSP Coprocessor #2.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  TODO: Add conditional-compilation support for SSSE3 (now @ 4.1)...
 *  Offending intrinsics: _mm_packus_epi32, _mm_blend_epi16, _mm_blendv_epi8, 
 *                        _mm_cvtep{i,u}16_epi32, _mm_mullo_epi32.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Common.h"
#include "CP2.h"
#include "Decoder.h"
#include "Opcodes.h"

#ifdef __cplusplus
#include <cstring>
#else
#include <string.h>
#endif

#ifdef USE_SSE
#include <smmintrin.h>

/* pshufb (_mm_shuffle_epi8) keys. */
typedef const uint8_t ShuffleKey[16];

/* ============================================================================
 *  SSEGetVectorOperands: Builds and returns the proper configuration of the
 *  `vt` vector for instructions that require the use of a element specifier.
 * ========================================================================= */
static const ShuffleKey VectorOperandsArray[16] = {
  /* -- */ {0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF},
  /* 0q */ {0x0,0x1,0x0,0x1,0x4,0x5,0x4,0x5,0x8,0x9,0x8,0x9,0xC,0xD,0xC,0xD},
  /* 1q */ {0x2,0x3,0x2,0x3,0x6,0x7,0x6,0x7,0xA,0xB,0xA,0xB,0xE,0xF,0xF,0xF},
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

static const ShuffleKey VectorOperandsArrayBS[16] = {
  /* -- */ {0x1,0x0,0x3,0x2,0x5,0x4,0x7,0x6,0x9,0x8,0xB,0xA,0xD,0xC,0xF,0xE},
  /* 0q */ {0x1,0x0,0x1,0x0,0x5,0x4,0x5,0x4,0x9,0x8,0x9,0x8,0xD,0xC,0xD,0xC},
  /* 1q */ {0x3,0x2,0x3,0x2,0x7,0x6,0x7,0x6,0xB,0xA,0xB,0xA,0xF,0xE,0xF,0xE},
  /* 0h */ {0x1,0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x9,0x8,0x9,0x8,0x9,0x8,0x9,0x8},
  /* 1h */ {0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2,0xB,0xA,0xB,0xA,0xB,0xA,0xB,0xA},
  /* 2h */ {0x5,0x4,0x5,0x4,0x5,0x4,0x5,0x4,0xD,0xC,0xD,0xC,0xD,0xC,0xD,0xC},
  /* 3h */ {0x7,0x6,0x7,0x6,0x7,0x6,0x7,0x6,0xF,0xE,0xF,0xE,0xF,0xE,0xF,0xE},
  /* 0w */ {0x1,0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x1,0x0},
  /* 1w */ {0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2,0x3,0x2},
  /* 2w */ {0x5,0x4,0x5,0x4,0x5,0x4,0x5,0x4,0x5,0x4,0x5,0x4,0x5,0x4,0x5,0x4},
  /* 3w */ {0x7,0x6,0x7,0x6,0x7,0x6,0x7,0x6,0x7,0x6,0x7,0x6,0x7,0x6,0x7,0x6},
  /* 4w */ {0x9,0x8,0x9,0x8,0x9,0x8,0x9,0x8,0x9,0x8,0x9,0x8,0x9,0x8,0x9,0x8},
  /* 5w */ {0xB,0xA,0xB,0xA,0xB,0xA,0xB,0xA,0xB,0xA,0xB,0xA,0xB,0xA,0xB,0xA},
  /* 6w */ {0xD,0xC,0xD,0xC,0xD,0xC,0xD,0xC,0xD,0xC,0xD,0xC,0xD,0xC,0xD,0xC},
  /* 7w */ {0xF,0xE,0xF,0xE,0xF,0xE,0xF,0xE,0xF,0xE,0xF,0xE,0xF,0xE,0xF,0xE}
};

static __m128i
SSEGetVectorOperands(__m128i vt, unsigned element) {
  __m128i key = _mm_load_si128((__m128i*) VectorOperandsArray[element]);
  return _mm_shuffle_epi8(vt, key);
}

static __m128i
SSEGetByteswappedVectorOperands(__m128i vt, unsigned element) {
  __m128i key = _mm_load_si128((__m128i*) VectorOperandsArrayBS[element]);
  return _mm_shuffle_epi8(vt, key);
}

/* ============================================================================
 *  SSEClampToVal: Saturates the accumulator using signed saturation.
 * ========================================================================= */
static __m128i
SSEClampToVal(__m128i val, __m128i accumulatorMid, __m128i accumulatorHi) {
  __m128i setMask = _mm_cmpeq_epi16(_mm_setzero_si128(), _mm_setzero_si128());
  __m128i unsignedClamp = _mm_packus_epi32(accumulatorMid, accumulatorHi);
  __m128i signedClamp = _mm_packs_epi32(accumulatorMid, accumulatorHi);
  __m128i signedClampCheck, unsignedClampCheck, selectMask1, selectMask2;
  __m128i useValMask;

  /* If [unsignedClamp == ~0 && signedClamp < 0] => negative. */
  signedClampCheck = _mm_cmplt_epi16(signedClamp,  _mm_setzero_si128());
  unsignedClampCheck = _mm_cmpeq_epi16(unsignedClamp, setMask);
  selectMask1 = _mm_and_si128(signedClampCheck, unsignedClampCheck);

  /* If [signedClamp >= 0 && unsignedClampCheck != ~0] => positive. */
  selectMask2 = _mm_or_si128(signedClampCheck, unsignedClampCheck);
  selectMask2 = _mm_xor_si128(selectMask2, setMask);

  /* Build the outputs, return the appropriate one. */
  useValMask = _mm_or_si128(selectMask1, selectMask2);
  return _mm_blendv_epi8(signedClamp, val, useValMask);
}

/* ============================================================================
 *  SSEUClampToVal: Saturates the accumulator using unsigned saturation.
 * ========================================================================= */
static __m128i
SSEUClampToVal(__m128i val, __m128i accumulatorMid, __m128i accumulatorHi) {
  __m128i setMask = _mm_cmpeq_epi16(_mm_setzero_si128(), _mm_setzero_si128());
  __m128i unsignedClamp = _mm_packus_epi32(accumulatorMid, accumulatorHi);
  __m128i signedClamp = _mm_packs_epi32(accumulatorMid, accumulatorHi);
  __m128i signedClampCheck, unsignedClampCheck, selectMask1, selectMask2;
  __m128i useValMask;

  /* If [unsignedClamp == ~0 && signedClamp < 0] => negative, no clamp. */
  signedClampCheck = _mm_cmplt_epi16(signedClamp,  _mm_setzero_si128());
  unsignedClampCheck = _mm_cmpeq_epi16(unsignedClamp, setMask);
  selectMask1 = _mm_and_si128(signedClampCheck, unsignedClampCheck);

  /* If [signedClamp >= 0 && unsignedClampCheck != ~0] => positive, no clamp. */
  selectMask2 = _mm_or_si128(signedClampCheck, unsignedClampCheck);
  selectMask2 = _mm_xor_si128(selectMask2, setMask);

  /* Build the outputs, return the appropriate one. */
  useValMask = _mm_or_si128(selectMask1, selectMask2);
  return _mm_blendv_epi8(unsignedClamp, val, useValMask);
}

/* ============================================================================
 *  SSEMAC32: Performs a + (b x c) on 32-bit slices.
 * ========================================================================= */
static __m128i
SSEMAC32(__m128i a, __m128i b, __m128i c) {
  return _mm_add_epi32(a, _mm_mullo_epi32(b, c));
}

/* ============================================================================
 *  SSEMACHigh16: a + ((b x c) >> 16) on 32-bit slices.
 * ========================================================================= */
static __m128i
SSEMACHigh16(__m128i a, __m128i b, __m128i c) {
  return _mm_add_epi32(a, _mm_srli_epi32(_mm_mullo_epi32(b, c), 16));
}

/* ============================================================================
 *  SSESwapByteOrder: Changes the byte ordering of an SSE register.
 * ========================================================================= */
static __m128i
SSESwapByteOrder(__m128i reg) {
  __m128i key = _mm_load_si128((__m128i*) VectorOperandsArrayBS[0]);
  return _mm_shuffle_epi8(reg, key);
}

/* ============================================================================
 *  SSESignExtend16to32: Sign extends 16-bit quantities to 32-bit quantities.
 * ========================================================================= */
static void
SSESignExtend16to32(__m128i src, __m128i *lo, __m128i *hi) {
  *hi = _mm_cvtepi16_epi32(_mm_srli_si128(src, 8));
  *lo = _mm_cvtepi16_epi32(src);
}

/* ============================================================================
 *  SSEPackAccumulators: Takes the 32-bit vectors from lo/hi and packs to ret.
 * ========================================================================= */
static void
SSEPackAccumulators(__m128i *lo, __m128i *hi) {
  __m128i hiPackReady1 = _mm_srli_epi32(*lo, 16);
  __m128i hiPackReady2 = _mm_srli_epi32(*hi, 16);
  __m128i loPackReady1 = _mm_blend_epi16(*lo, _mm_setzero_si128(), 0xAA);
  __m128i loPackReady2 = _mm_blend_epi16(*hi, _mm_setzero_si128(), 0xAA);
  *lo = _mm_packus_epi32(loPackReady1, loPackReady2);
  *hi = _mm_packus_epi32(hiPackReady1, hiPackReady2);
}

/* ============================================================================
 *  SSEPack32to16: Packs 32-bit values to 16-bit quantities without saturation.
 * ========================================================================= */
static __m128i
SSEPack32to16(__m128i lo, __m128i hi) {
  lo = _mm_blend_epi16(lo, _mm_setzero_si128(), 0xAA);
  hi = _mm_blend_epi16(hi, _mm_setzero_si128(), 0xAA);
  return _mm_packus_epi32(lo, hi);
}

/* ============================================================================
 *  SSEZeroExtend16to32: Zero extends 16-bit quantities to 32-bit quantities.
 * ========================================================================= */
static void
SSEZeroExtend16to32(__m128i src, __m128i *lo, __m128i *hi) {
  *hi = _mm_cvtepu16_epi32(_mm_srli_si128(src, 8));
  *lo = _mm_cvtepu16_epi32(src);
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
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i vdVector;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  vdVector = _mm_sign_epi16(vtVector, vsVector);
  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(vdVector));
  _mm_store_si128((__m128i*) acc, vdVector);
#else
#warning "Unimplemented function: RSPVABS (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VADD (Vector Add of Short Elements)
 *  TODO: Test this with edge cases on optimized builds.
 * ========================================================================= */
void
RSPVADD(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  const uint16_t *vco = cp2->carryOut.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;
  uint16_t *acc = cp2->accumulatorLow.slices;

#ifdef USE_SSE
  __m128i carryIn = _mm_load_si128((__m128i*) vco);
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i vdVector, vtVectorCarryIns, vsVectorCarryIns;
  __m128i accumulator;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  /* Have to add the carry from the min of (vs,vt) */
  /* for each slice, or we might clamp incorrectly. */
  vtVectorCarryIns = _mm_cmplt_epi16(vtVector, vsVector);
  vtVectorCarryIns = _mm_and_si128(vtVectorCarryIns, carryIn);
  vsVectorCarryIns = _mm_xor_si128(vtVectorCarryIns, carryIn);

  /* Write the accumulator; don't sign extend. */
  accumulator = _mm_add_epi16(vsVector, vtVector);
  accumulator = _mm_add_epi16(accumulator, carryIn);
  _mm_store_si128((__m128i*) acc, accumulator);

  /* Compute the clamped result and store it. */
  vtVector = _mm_adds_epi16(vtVector, vtVectorCarryIns);
  vsVector = _mm_adds_epi16(vsVector, vsVectorCarryIns);
  vdVector = _mm_adds_epi16(vsVector, vtVector);

  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(vdVector));
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
  const uint16_t *vco = cp2->carryOut.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;
  uint16_t *acc = cp2->accumulatorLow.slices;

#ifdef USE_SSE
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i slicesLo, slicesHi, vdVector, carryOut;
  __m128i vtSlicesLo, vtSlicesHi;
  __m128i vsSlicesLo, vsSlicesHi;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  /* Zero-extend so we can check for carry out. */
  SSEZeroExtend16to32(vtVector, &vtSlicesLo, &vtSlicesHi);
  SSEZeroExtend16to32(vsVector, &vsSlicesLo, &vsSlicesHi);
  slicesLo = _mm_add_epi32(vtSlicesLo, vsSlicesLo);
  slicesHi = _mm_add_epi32(vtSlicesHi, vsSlicesHi);
  vdVector = SSEPack32to16(slicesLo, slicesHi);

  /* If any high bits are set, then carry out. */
  slicesLo = _mm_srli_epi32(slicesLo, 16);
  slicesHi = _mm_srli_epi32(slicesHi, 16);
  carryOut = SSEPack32to16(slicesLo, slicesHi);
  carryOut = _mm_cmpeq_epi16(carryOut, _mm_setzero_si128());
  carryOut = _mm_cmpeq_epi16(carryOut, _mm_setzero_si128());

  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(vdVector));
  _mm_store_si128((__m128i*) acc, vdVector);
  _mm_store_si128((__m128i*) vco, carryOut);
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
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i vdVector;

  vtVector = SSEGetVectorOperands(vtVector, element);

  vdVector = _mm_and_si128(vtVector, vsVector);
  _mm_store_si128((__m128i*) vd, vdVector);
  _mm_store_si128((__m128i*) acc, vdVector);
#else
#warning "Unimplemented function: RSPVAND (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VEQ (Vector Select Equal)
 * ========================================================================= */
void
RSPVEQ(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *acc = cp2->accumulatorLow.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i compareCode = _mm_load_si128((__m128i*) cp2->compareCode.slices);
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i vdVector, compareCodeMask, compareVectorMask;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  /* Update the vector control register accordingly. */
  compareVectorMask = _mm_cmpeq_epi16(vtVector, vsVector);
  compareCodeMask = _mm_packs_epi16(compareVectorMask, _mm_setzero_si128());
  compareCodeMask = _mm_or_si128(compareCodeMask, compareCode);
  _mm_store_si128((__m128i*) cp2->compareCode.slices, compareCodeMask);

  /* Select the appropriate slices from vs/vt and write it out. */
  vdVector = _mm_blendv_epi8(vtVector, vsVector, compareVectorMask);
  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(vdVector));
  _mm_store_si128((__m128i*) acc, vdVector);
#else
#warning "Unimplemented function: RSPVEQ (No SSE)."
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
 *  Instruction: VGE (Vector Select Greater Than or Equal)
 *  TODO: Not tested, need to account for VCC/VCO/VCE logic.
 * ========================================================================= */
void
RSPVGE(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i greaterThanMask, equalToMask;
  __m128i notSelectMask, selectMask;
  __m128i vtComponents, vsComponents;
  __m128i vdVector;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  greaterThanMask = _mm_cmpgt_epi16(vsVector, vtVector);
  equalToMask = _mm_cmpeq_epi16(vsVector, vtVector);
  selectMask = _mm_or_si128(greaterThanMask, equalToMask);
  notSelectMask = _mm_cmpeq_epi16(selectMask, _mm_setzero_si128()); 

  vsComponents = _mm_and_si128(notSelectMask, vsVector);
  vtComponents = _mm_and_si128(selectMask, vtVector);
  vdVector = _mm_or_si128(vsComponents, vtComponents);

  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(vdVector));
#else
#warning "Unimplemented function: RSPVGE (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
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
 *  TODO: Not tested, need to account for VCC/VCO/VCE logic.
 * ========================================================================= */
void
RSPVLT(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i greaterThanMask, equalToMask;
  __m128i notSelectMask, selectMask;
  __m128i vtComponents, vsComponents;
  __m128i vdVector;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  greaterThanMask = _mm_cmplt_epi16(vsVector, vtVector);
  equalToMask = _mm_cmpeq_epi16(vsVector, vtVector);
  selectMask = _mm_or_si128(greaterThanMask, equalToMask);
  notSelectMask = _mm_cmpeq_epi16(selectMask, _mm_setzero_si128()); 

  vsComponents = _mm_and_si128(notSelectMask, vsVector);
  vtComponents = _mm_and_si128(selectMask, vtVector);
  vdVector = _mm_or_si128(vsComponents, vtComponents);

  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(vdVector));
#else
#warning "Unimplemented function: RSPVLT (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VMACF (Vector Multiply-Accumulate of Signed Fractions)
 * ========================================================================= */
void
RSPVMACF(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *accMid = cp2->accumulatorMid.slices;
  uint16_t *accHi = cp2->accumulatorHigh.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i accumulatorSlicesMid = _mm_load_si128((__m128i*) accMid);
  __m128i accumulatorSlicesHi = _mm_load_si128((__m128i*) accHi);
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i unpackedLo, unpackedHi, vdVector;
  __m128i accumulatorLo, accumulatorHi;
  __m128i vtSlicesLo, vtSlicesHi;
  __m128i vsSlicesLo, vsSlicesHi;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  /* Sign-extend the 16b slices to 32b. */
  SSESignExtend16to32(vtVector, &vtSlicesLo, &vtSlicesHi);
  SSESignExtend16to32(vsVector, &vsSlicesLo, &vsSlicesHi);

  /* Multiply and prepare to accumulate the results. */
  accumulatorLo = _mm_mullo_epi32(vtSlicesLo, vsSlicesLo);
  accumulatorHi = _mm_mullo_epi32(vtSlicesHi, vsSlicesHi);
  accumulatorLo = _mm_slli_epi32(accumulatorLo, 1);
  accumulatorHi = _mm_slli_epi32(accumulatorHi, 1);

  /* Accumulate the results. */
  unpackedLo = _mm_unpacklo_epi16(accumulatorSlicesMid, accumulatorSlicesHi);
  unpackedHi = _mm_unpackhi_epi16(accumulatorSlicesMid, accumulatorSlicesHi);
  accumulatorLo = unpackedLo = _mm_add_epi32(accumulatorLo, unpackedLo);
  accumulatorHi = unpackedHi = _mm_add_epi32(accumulatorHi, unpackedLo);
  SSEPackAccumulators(&accumulatorLo, &accumulatorHi);

  /* Clamp and write out the results accordingly. */
  vdVector = SSEClampToVal(accumulatorLo, unpackedLo, unpackedHi);
  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(vdVector));
  _mm_store_si128((__m128i*) accMid, accumulatorLo);
  _mm_store_si128((__m128i*) accHi, accumulatorHi);
#else
#warning "Unimplemented function: RSPVMACF (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
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
RSPVMADH(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *accMid = cp2->accumulatorMid.slices;
  uint16_t *accHi = cp2->accumulatorHigh.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i accumulator = _mm_load_si128((__m128i*) accHi);
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i accumulatorLo, accumulatorHi;
  __m128i unpackedLo, unpackedHi;
  __m128i vtSlicesLo, vtSlicesHi;
  __m128i vsSlicesLo, vsSlicesHi;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  /* Unpack to prepare for 32-bit arithmetic. */
  SSESignExtend16to32(accumulator, &accumulatorLo, &accumulatorHi);
  SSESignExtend16to32(vtVector, &vtSlicesLo, &vtSlicesHi);
  SSESignExtend16to32(vsVector, &vsSlicesLo, &vsSlicesHi);

  /* Multiply, accumulate, write the accumulator. */
  accumulatorLo = SSEMAC32(accumulatorLo, vtSlicesLo, vsSlicesLo);
  accumulatorHi = SSEMAC32(accumulatorHi, vtSlicesHi, vsSlicesHi);
  accumulatorHi = SSEPack32to16(accumulatorLo, accumulatorHi);
  _mm_store_si128((__m128i*) accHi, accumulatorHi);

  /* Clamp and store to the destination. */
  accumulatorLo = _mm_load_si128((__m128i*) accMid);
  unpackedLo = _mm_unpacklo_epi16(accumulatorLo, accumulatorHi);
  unpackedHi = _mm_unpackhi_epi16(accumulatorLo, accumulatorHi);
  accumulator = _mm_packs_epi32(unpackedLo, unpackedHi);
  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(accumulator));
#else
#warning "Unimplemented function: RSPVMADH (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VMADL (Vector Multiply-Accumulate of Lower Partial Products).
 * ========================================================================= */
void
RSPVMADL(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *accHigh = cp2->accumulatorHigh.slices;
  uint16_t *accMid = cp2->accumulatorMid.slices;
  uint16_t *accLow = cp2->accumulatorLow.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i hiAccumulatorSlices = _mm_load_si128((__m128i*) accHigh);
  __m128i midAccumulatorSlices = _mm_load_si128((__m128i*) accMid);
  __m128i loAccumulatorSlices = _mm_load_si128((__m128i*) accLow);
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i accumulatorLo, accumulatorHi, accumulator;
  __m128i oldAccumulatorLo, oldAccumulatorHi;
  __m128i vtSlicesLo, vtSlicesHi;
  __m128i vsSlicesLo, vsSlicesHi;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  /* Zero-extend the 16b slices to 32b. */
  SSEZeroExtend16to32(loAccumulatorSlices, &accumulatorLo, &accumulatorHi);
  SSEZeroExtend16to32(vtVector, &vtSlicesLo, &vtSlicesHi);
  SSEZeroExtend16to32(vsVector, &vsSlicesLo, &vsSlicesHi);

  /* Accumulate and write out the lowest slice. */
  accumulatorLo = SSEMACHigh16(accumulatorLo, vtSlicesLo, vsSlicesLo);
  accumulatorHi = SSEMACHigh16(accumulatorHi, vtSlicesHi, vsSlicesLo);
  accumulator = SSEPack32to16(accumulatorLo, accumulatorHi); 
  _mm_store_si128((__m128i*) accLow, accumulator);
  oldAccumulatorLo = _mm_srli_epi32(accumulatorLo, 16);
  oldAccumulatorHi = _mm_srli_epi32(accumulatorHi, 16);
  accumulatorHi = hiAccumulatorSlices;

  /* Finish off the upper two slices of the accumulator. */
  accumulatorLo = _mm_unpacklo_epi16(midAccumulatorSlices, accumulatorHi);
  accumulatorHi = _mm_unpackhi_epi16(midAccumulatorSlices, accumulatorHi);
  accumulatorLo = _mm_add_epi32(oldAccumulatorLo, accumulatorLo);
  accumulatorHi = _mm_add_epi32(oldAccumulatorHi, accumulatorHi);
  accumulator = SSEUClampToVal(accumulator, accumulatorLo, accumulatorHi);
  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(accumulator));

  /* Pack the accumulators back up, write them out. */
  SSEPackAccumulators(&accumulatorLo, &accumulatorHi);
  _mm_store_si128((__m128i*) accMid, accumulatorLo);
  _mm_store_si128((__m128i*) accHigh, accumulatorHi);
#else
#warning "Unimplemented function: RSPVMADL (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VMADM (Vector Multiply-Accumulate of Mid Partial Products)
 * ========================================================================= */
void
RSPVMADM(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *accHigh = cp2->accumulatorHigh.slices;
  uint16_t *accMid = cp2->accumulatorMid.slices;
  uint16_t *accLow = cp2->accumulatorLow.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i hiAccumulatorSlices = _mm_load_si128((__m128i*) accHigh);
  __m128i midAccumulatorSlices = _mm_load_si128((__m128i*) accMid);
  __m128i loAccumulatorSlices = _mm_load_si128((__m128i*) accLow);
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i accumulatorLo, accumulatorHi, vdVector;
  __m128i vtSlicesLo, vtSlicesHi;
  __m128i vsSlicesLo, vsSlicesHi;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  /* Unpack to prepare for 32-bit arithmetic. */
  accumulatorLo = _mm_unpacklo_epi16(midAccumulatorSlices, hiAccumulatorSlices);
  accumulatorHi = _mm_unpackhi_epi16(midAccumulatorSlices, hiAccumulatorSlices);
  SSEZeroExtend16to32(vtVector, &vtSlicesLo, &vtSlicesHi);
  SSESignExtend16to32(vsVector, &vsSlicesLo, &vsSlicesHi);

  /* Multiply and accumulate the results. */
  accumulatorLo = SSEMAC32(accumulatorLo, vtSlicesLo, vsSlicesLo);
  accumulatorHi = SSEMAC32(accumulatorHi, vtSlicesHi, vsSlicesHi);

  /* Clamp the accumulator, pack up the results, and write everything. */
  vdVector = SSEUClampToVal(loAccumulatorSlices, accumulatorLo, accumulatorHi);
  SSEPackAccumulators(&accumulatorLo, &accumulatorHi);

  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(vdVector));
  _mm_store_si128((__m128i*) accHigh, accumulatorHi);
  _mm_store_si128((__m128i*) accMid, accumulatorLo);
#else
#warning "Unimplemented function: RSPVMADM (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VMADN (Vector Multiply-Accumulate of Mid Partial Products)
 * ========================================================================= */
void
RSPVMADN(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *accHigh = cp2->accumulatorHigh.slices;
  uint16_t *accMid = cp2->accumulatorMid.slices;
  uint16_t *accLow = cp2->accumulatorLow.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i hiAccumulatorSlices = _mm_load_si128((__m128i*) accHigh);
  __m128i midAccumulatorSlices = _mm_load_si128((__m128i*) accMid);
  __m128i loAccumulatorSlices = _mm_load_si128((__m128i*) accLow);
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i accumulatorLo, accumulatorHi, vdVector;
  __m128i vtSlicesLo, vtSlicesHi;
  __m128i vsSlicesLo, vsSlicesHi;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  /* Unpack to prepare for 32-bit arithmetic. */
  accumulatorLo = _mm_unpacklo_epi16(midAccumulatorSlices, hiAccumulatorSlices);
  accumulatorHi = _mm_unpackhi_epi16(midAccumulatorSlices, hiAccumulatorSlices);
  SSESignExtend16to32(vtVector, &vtSlicesLo, &vtSlicesHi);
  SSEZeroExtend16to32(vsVector, &vsSlicesLo, &vsSlicesHi);

  /* Multiply and accumulate the results. */
  accumulatorLo = SSEMAC32(accumulatorLo, vtSlicesLo, vsSlicesLo);
  accumulatorHi = SSEMAC32(accumulatorHi, vtSlicesHi, vsSlicesHi);

  /* Clamp the accumulator, pack up the results, and write everything. */
  vdVector = SSEUClampToVal(loAccumulatorSlices, accumulatorLo, accumulatorHi);
  SSEPackAccumulators(&accumulatorLo, &accumulatorHi);

  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(vdVector));
  _mm_store_si128((__m128i*) accHigh, accumulatorHi);
  _mm_store_si128((__m128i*) accMid, accumulatorLo);
#else
#warning "Unimplemented function: RSPVMADN (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VMOV (Vector Element Scalar Move)
 * ========================================================================= */
void
RSPVMOV(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vtRegister = iw >> 16 & 0x1F;
  unsigned vdRegister = iw >> 6 & 0x1F;

  const uint16_t *vt = cp2->regs[vtRegister].slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;
  unsigned de = iw >> 11 & 0x7;
  unsigned e = iw >> 21 & 0x7;

  memcpy(cp2->accumulatorLow.slices, vt, sizeof(__m128i));
  vd[de] = vt[e];

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VMRG (Vector Select Merge)
 * ========================================================================= */
void
RSPVMRG(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i vccMask = _mm_load_si128((__m128i*) cp2->compareCode.slices);
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);

  vtVector = SSEGetVectorOperands(vtVector, element);
  vccMask = _mm_cvtepi8_epi16(vccMask);

  __m128i vdVector = _mm_blendv_epi8(vtVector, vsVector, vccMask);
  _mm_store_si128((__m128i*) vd, vdVector);
#else
#warning "Unimplemented function: RSPVMRG (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VMUDH (Vector Multiply of High Partial Products)
 * ========================================================================= */
void
RSPVMUDH(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *accHigh = cp2->accumulatorHigh.slices;
  uint16_t *accMid = cp2->accumulatorMid.slices;
  uint16_t *accLow = cp2->accumulatorLow.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i accumulatorLo, accumulatorHi, accumulator;
  __m128i vtSlicesLo, vtSlicesHi;
  __m128i vsSlicesLo, vsSlicesHi;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  /* Zero-extend the 16b slices to 32b. */
  SSESignExtend16to32(vtVector, &vtSlicesLo, &vtSlicesHi);
  SSESignExtend16to32(vsVector, &vsSlicesLo, &vsSlicesHi);

  /* Multiply, pack, and write out the accumulator/result. */
  accumulatorLo = _mm_mullo_epi32(vtSlicesLo, vsSlicesLo);
  accumulatorHi = _mm_mullo_epi32(vtSlicesHi, vsSlicesHi);
  accumulator = _mm_packs_epi32(accumulatorLo, accumulatorHi);
  SSEPackAccumulators(&accumulatorLo, &accumulatorHi);

  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(accumulator));
  _mm_store_si128((__m128i*) accLow, _mm_setzero_si128());
  _mm_store_si128((__m128i*) accHigh, accumulatorHi);
  _mm_store_si128((__m128i*) accMid, accumulatorLo);
#else
#warning "Unimplemented function: RSPVMUDH (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VMUDL (Vector Multiply of Low Partial Products)
 * ========================================================================= */
void
RSPVMUDL(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *accHigh = cp2->accumulatorHigh.slices;
  uint16_t *accMid = cp2->accumulatorMid.slices;
  uint16_t *accLow = cp2->accumulatorLow.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i accumulatorLo, accumulatorHi, vdVector;
  __m128i vtSlicesLo, vtSlicesHi;
  __m128i vsSlicesLo, vsSlicesHi;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  /* Zero-extend the 16b slices to 32b. */
  SSEZeroExtend16to32(vtVector, &vtSlicesLo, &vtSlicesHi);
  SSEZeroExtend16to32(vsVector, &vsSlicesLo, &vsSlicesHi);

  /* Multiply, pack, and write out the accumulator/result. */
  accumulatorLo = _mm_mullo_epi32(vtSlicesLo, vsSlicesLo);
  accumulatorHi = _mm_mullo_epi32(vtSlicesHi, vsSlicesHi);
  vdVector = SSEPack32to16(accumulatorLo, accumulatorHi);

  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(vdVector));
  _mm_store_si128((__m128i*) accHigh, _mm_setzero_si128());
  _mm_store_si128((__m128i*) accMid, _mm_setzero_si128());
  _mm_store_si128((__m128i*) accLow, vdVector);
#else
#warning "Unimplemented function: RSPVMUDL (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VMUDM (Vector Multiply of Middle Partial Products)
 * ========================================================================= */
void
RSPVMUDM(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *accHigh = cp2->accumulatorHigh.slices;
  uint16_t *accMid = cp2->accumulatorMid.slices;
  uint16_t *accLow = cp2->accumulatorLow.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i accumulatorLo, accumulatorHi, accumulator;
  __m128i vtSlicesLo, vtSlicesHi;
  __m128i vsSlicesLo, vsSlicesHi;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  /* Zero-extend the 16b slices to 32b. */
  SSEZeroExtend16to32(vtVector, &vtSlicesLo, &vtSlicesHi);
  SSESignExtend16to32(vsVector, &vsSlicesLo, &vsSlicesHi);

  /* Multiply and write out the lowest slice. */
  accumulatorLo = _mm_mullo_epi32(vtSlicesLo, vsSlicesLo);
  accumulatorHi = _mm_mullo_epi32(vtSlicesHi, vsSlicesHi);
  accumulator = SSEPack32to16(accumulatorLo, accumulatorHi);

  /* Sign-extend, pack, and write out the accumulator/result. */
  accumulatorLo = _mm_srai_epi32(accumulatorLo, 16);
  accumulatorHi = _mm_srai_epi32(accumulatorHi, 16);
  SSEPackAccumulators(&accumulatorLo, &accumulatorHi);

  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(accumulatorLo));
  _mm_store_si128((__m128i*) accHigh, accumulatorHi);
  _mm_store_si128((__m128i*) accMid, accumulatorLo);
  _mm_store_si128((__m128i*) accLow, accumulator);
#else
#warning "Unimplemented function: RSPVMUDM (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
}

/* ============================================================================
 *  Instruction: VMUDN (Vector Multiply of Middle Partial Products)
 * ========================================================================= */
void
RSPVMUDN(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  uint16_t *accHigh = cp2->accumulatorHigh.slices;
  uint16_t *accMid = cp2->accumulatorMid.slices;
  uint16_t *accLow = cp2->accumulatorLow.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i accumulatorLo, accumulatorHi, accumulator;
  __m128i vtSlicesLo, vtSlicesHi;
  __m128i vsSlicesLo, vsSlicesHi;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  /* Zero-extend the 16b slices to 32b. */
  SSESignExtend16to32(vtVector, &vtSlicesLo, &vtSlicesHi);
  SSEZeroExtend16to32(vsVector, &vsSlicesLo, &vsSlicesHi);

  /* Multiply and write out the lowest slice. */
  accumulatorLo = _mm_mullo_epi32(vtSlicesLo, vsSlicesLo);
  accumulatorHi = _mm_mullo_epi32(vtSlicesHi, vsSlicesHi);
  accumulator = SSEPack32to16(accumulatorLo, accumulatorHi);

  /* Sign-extend, pack, and write out the accumulator/result. */
  accumulatorLo = _mm_srai_epi32(accumulatorLo, 16);
  accumulatorHi = _mm_srai_epi32(accumulatorHi, 16);
  SSEPackAccumulators(&accumulatorLo, &accumulatorHi);

  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(accumulatorLo));
  _mm_store_si128((__m128i*) accHigh, accumulatorHi);
  _mm_store_si128((__m128i*) accMid, accumulatorLo);
  _mm_store_si128((__m128i*) accLow, accumulator);
#else
#warning "Unimplemented function: RSPVMUDN (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
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
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i vdVector;

  vtVector = SSEGetVectorOperands(vtVector, element);

  vdVector = _mm_nand_si128(vsVector, vtVector);
  _mm_store_si128((__m128i*) acc, vdVector);
  _mm_store_si128((__m128i*) vd, vdVector);
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
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i vdVector;

  vtVector = SSEGetVectorOperands(vtVector, element);

  vdVector = _mm_nor_si128(vsVector, vtVector);
  _mm_store_si128((__m128i*) acc, vdVector);
  _mm_store_si128((__m128i*) vd, vdVector);
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
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i vdVector;

  vtVector = SSEGetVectorOperands(vtVector, element);
  vdVector = _mm_or_si128(vtVector, vsVector);
  _mm_store_si128((__m128i*) acc, vdVector);
  _mm_store_si128((__m128i*) vd, vdVector);
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
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i vdVector;

  vtVector = SSEGetVectorOperands(vtVector, element);

  vdVector = _mm_nxor_si128(vsVector, vtVector);
  _mm_store_si128((__m128i*) acc, vdVector);
  _mm_store_si128((__m128i*) vd, vdVector);
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
RSPVRCPH(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VRCPH.");
  cp2->mulStageDest = 0;
}

/* ============================================================================
 *  Instruction: VRCPL (Vector Element Scalar Reciprocal (Double Prec. Low))
 * ========================================================================= */
void
RSPVRCPL(struct RSPCP2 *cp2, uint32_t unused(iw)) {
  debug("Unimplemented instruction: VRCPL.");
  cp2->mulStageDest = 0;
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
RSPVSUB(struct RSPCP2 *cp2, uint32_t iw) {
  unsigned vdRegister = iw >> 6 & 0x1F;
  unsigned element = iw >> 21 & 0xF;

  const uint16_t *vt = cp2->regs[iw >> 16 & 0x1F].slices;
  const uint16_t *vs = cp2->regs[iw >> 11 & 0x1F].slices;
  const uint16_t *vco = cp2->carryOut.slices;
  uint16_t *acc = cp2->accumulatorLow.slices;
  uint16_t *vd = cp2->regs[vdRegister].slices;

#ifdef USE_SSE
  __m128i carryIn = _mm_load_si128((__m128i*) vco);
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i vdVector, vtVectorCarryIns, vsVectorCarryIns;
  __m128i accumulator;

  vtVector = SSEGetByteswappedVectorOperands(vtVector, element);
  vsVector = SSESwapByteOrder(vsVector);

  /* Have to add the carry from the min of (vs,vt) */
  /* for each slice, or we might clamp incorrectly. */
  vtVectorCarryIns = _mm_cmplt_epi16(vtVector, vsVector);
  vtVectorCarryIns = _mm_and_si128(vtVectorCarryIns, carryIn);
  vsVectorCarryIns = _mm_xor_si128(vtVectorCarryIns, carryIn);

  /* Write the accumulator; don't sign extend. */
  accumulator = _mm_sub_epi16(vsVector, vtVector);
  accumulator = _mm_sub_epi16(accumulator, carryIn);
  _mm_store_si128((__m128i*) acc, accumulator);

  /* Compute the clamped result and store it. */
  vtVector = _mm_subs_epi16(vtVector, vtVectorCarryIns);
  vsVector = _mm_subs_epi16(vsVector, vsVectorCarryIns);
  vdVector = _mm_subs_epi16(vsVector, vtVector);

  _mm_store_si128((__m128i*) vd, SSESwapByteOrder(vdVector));
#else
#warning "Unimplemented function: RSPVSUB (No SSE)."
#endif

  cp2->mulStageDest = vdRegister;
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
  __m128i vtVector = _mm_load_si128((__m128i*) vt);
  __m128i vsVector = _mm_load_si128((__m128i*) vs);
  __m128i vdVector;

  vtVector = SSEGetVectorOperands(vtVector, element);

  vdVector = _mm_xor_si128(vsVector, vtVector);
  _mm_store_si128((__m128i*) acc, vdVector);
  _mm_store_si128((__m128i*) vd, vdVector);
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
  cp2->mulStageDest = 0;                       /* "EX" */

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

