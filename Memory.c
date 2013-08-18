/* ============================================================================
 *  Memory.c: Memory read/write functions.
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
#include "CPU.h"
#include "Memory.h"

#ifdef __cplusplus
#include <cassert>
#include <cstring>
#else
#include <assert.h>
#include <string.h>
#endif

#ifdef USE_SSE
#include <tmmintrin.h>
#endif

/* SSE-assisted helper functions. */
static void LoadPackedBytes(void *src, void *dest);
static void LoadPackedUBytes(void *src, void *dest);
static void StorePackedBytes(void *src, void *dest);
static void StorePackedUBytes(void *src, void *dest);

/* ============================================================================
 *  Copies the data from src to dest, swapping every other byte.
 * ========================================================================= */
void
CopyVectorSlices(void *src, void *dest) {
#ifdef USE_SSE
  __m128i temp, mask;

  static const uint8_t swapmask[] = {
    0x01, 0x00, 0x03, 0x02, 0x05, 0x04, 0x07, 0x06,
    0x09, 0x08, 0x0B, 0x0A, 0x0D, 0x0C, 0x0F, 0x0E
  };

  mask = _mm_load_si128((__m128i*) swapmask);
  temp = _mm_loadu_si128((__m128i*) src);
  temp = _mm_shuffle_epi8(temp, mask);
  _mm_storeu_si128((__m128i*) dest, temp);
#else
#warning "Unimplemented function: CopyVectorSlices (No SSE)."
#endif
}

/* ============================================================================
 *  Copies the data from src to dest, swapping every other byte and packing.
 * ========================================================================= */
static void
LoadPackedBytes(void *src, void *dest) {
#ifdef USE_SSE
  __m128i temp, mask;

  static const uint8_t swapmask[] = {
    0x80, 0x00, 0x80, 0x01, 0x80, 0x02, 0x80, 0x03,
    0x80, 0x04, 0x80, 0x05, 0x80, 0x06, 0x80, 0x07
  };

  mask = _mm_load_si128((__m128i*) swapmask);
  temp = _mm_loadu_si128((__m128i*) src);
  temp = _mm_shuffle_epi8(temp, mask);
  _mm_storeu_si128((__m128i*) dest, temp);
#else
#warning "Unimplemented function: LoadPackedBytes (No SSE)."
#endif
}

/* ============================================================================
 *  Copies the data from src to dest, swapping every other byte and packing.
 * ========================================================================= */
static void
LoadPackedUBytes(void *src, void *dest) {
#ifdef USE_SSE
  __m128i temp, mask;

  static const uint8_t swapmask[] = {
    0x00, 0x80, 0x01, 0x80, 0x02, 0x80, 0x03, 0x80,
    0x04, 0x80, 0x05, 0x80, 0x06, 0x80, 0x07, 0x80
  };

  mask = _mm_load_si128((__m128i*) swapmask);
  temp = _mm_loadu_si128((__m128i*) src);
  temp = _mm_shuffle_epi8(temp, mask);
  temp = _mm_slli_epi16(temp, 7);
  _mm_storeu_si128((__m128i*) dest, temp);
#else
#warning "Unimplemented function: LoadPackedUBytes (No SSE)."
#endif
}

/* ============================================================================
 *  Copies the data from src to dest, swapping every other byte and packing.
 * ========================================================================= */
static void
StorePackedBytes(void *src, void *dest) {
#ifdef USE_SSE
  __m128i temp, mask;

  static const uint8_t swapmask[] = {
    0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
  };

  mask = _mm_load_si128((__m128i*) swapmask);
  temp = _mm_loadu_si128((__m128i*) src);
  temp = _mm_shuffle_epi8(temp, mask);
  _mm_storel_epi64((__m128i*) dest, temp);
#else
#warning "Unimplemented function: StorePackedBytes (No SSE)."
#endif
}

/* ============================================================================
 *  Copies the data from src to dest, swapping every other byte and packing.
 * ========================================================================= */
static void
StorePackedUBytes(void *src, void *dest) {
#ifdef USE_SSE
  __m128i temp, mask;

  static const uint8_t swapmask[] = {
    0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
  };

  mask = _mm_load_si128((__m128i*) swapmask);
  temp = _mm_loadu_si128((__m128i*) src);
  temp = _mm_srai_epi16(temp, 7);
  temp = _mm_shuffle_epi8(temp, mask);
  _mm_storel_epi64((__m128i*) dest, temp);
#else
#warning "Unimplemented function: StorePackedUBytes (No SSE)."
#endif
}

/* ============================================================================
 *  RSPMemoryFunction: LoadByte
 * ========================================================================= */
void
LoadByte(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  uint32_t *target = (uint32_t*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  int8_t byte;

  /* Load and sign extend. */
  memcpy(&byte, dmem + offset, sizeof(byte));
  *target = (int32_t) byte;
}

/* ============================================================================
 *  RSPMemoryFunction: LoadByteVector
 * ========================================================================= */
void
LoadByteVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  uint8_t *slice = (uint8_t*) vector->slices;
  unsigned element = memoryData->element;

  /* TODO: Check. */
  assert(0);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadByteUnsigned
 * ========================================================================= */
void
LoadByteUnsigned(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  uint32_t *target = (uint32_t*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  uint8_t byte;

  /* Load and DO NOT sign extend. */
  memcpy(&byte, dmem + offset, sizeof(byte));
  *target = (uint32_t) byte;
}

/* ============================================================================
 *  RSPMemoryFunction: LoadDoubleVector
 * ========================================================================= */
void
LoadDoubleVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element, start;
  uint8_t slices[16];

  start = offset & 0x7;

  /* Currently dont even bother to handle either of these. */
  assert((element & 0x1) == 0 && "Element references odd byte of slice?");
  assert(element <= 8 && "Would load past the 128-bit boundary.");

  if ((start & 0x1) == 1)
    debug("WARNING: LDV: Address not halfword aligned?");

  CopyVectorSlices(dmem + offset, slices);
  memcpy(vector->slices + (element >> 1), slices, 8);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadHalf
 * ========================================================================= */
void
LoadHalf(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  uint32_t *target = (uint32_t*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  int16_t half;

  /* Load and sign extend. */
  memcpy(&half, dmem + offset, sizeof(half));
  *target = (int32_t) ByteOrderSwap16(half);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadHalfUnsigned
 * ========================================================================= */
void
LoadHalfUnsigned(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  uint32_t *target = (uint32_t*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  uint16_t half;

  /* Load and DO NOT sign extend. */
  memcpy(&half, dmem + offset, sizeof(half));
  *target = (uint32_t) ByteOrderSwap16(half);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadLongVector
 * ========================================================================= */
void
LoadLongVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element, start;
  uint8_t slices[16];

  start = offset & 0x3;

  /* Currently dont even bother to handle either of these. */
  assert((element & 0x1) == 0 && "Element references odd byte of slice?");
  assert(element <= 12 && "Would load past the 128-bit boundary.");

  if ((start & 0x1) == 1)
    debug("WARNING: LLV: Address not halfword aligned?");

  CopyVectorSlices(dmem + offset, slices);
  memcpy(vector->slices + (element >> 1), slices + start, 4);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadPackedByteVector
 * ========================================================================= */
void
LoadPackedByteVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element, start;

  start = offset & 0x7;
  offset &= 0xFF8;

  /* Currently dont even bother to handle either of these. */
  assert(element == 0 && "Element something other than zero?");

  /* Aligned reads. */
  if (likely(start == 0))
    LoadPackedBytes(dmem + offset, vector->slices);

  /* Unaligned reads. */
  else {
    uint16_t slices[8];

    /* TODO: FIXME. */
    assert(0);

    LoadPackedBytes(dmem + offset + start, slices);
    memcpy(vector->slices, slices, 16);
  }
}

/* ============================================================================
 *  RSPMemoryFunction: LoadPackedFourthVector
 * ========================================================================= */
void
LoadPackedFourthVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element;

  assert(0);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadPackedHalfVector
 * ========================================================================= */
void
LoadPackedHalfVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  assert(0);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadPackedVector
 * ========================================================================= */
void
LoadPackedVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element, start;

  start = offset & 0x7;
  /*offset &= 0xFF8;*/

  /* Currently dont even bother to handle either of these. */
  assert(element == 0 && "Element something other than zero?");

  /* Aligned reads. */
  if (likely(start == 0))
    LoadPackedUBytes(dmem + offset, vector->slices);

  /* Unaligned reads. */
  else {
    uint16_t slices[8];

    /* TODO: FIXME. */
    assert(0);

    LoadPackedUBytes(dmem + offset + start, slices);
    memcpy(vector->slices, slices, 16 - start);
  }
}

/* ============================================================================
 *  RSPMemoryFunction: LoadQuadVector
 * ========================================================================= */
void
LoadQuadVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element, start;

  start = offset & 0xF;
  offset &= 0xFF0;

  /* Currently dont even bother to handle either of these. */
  assert(element == 0 && "Element something other than zero?");

  /* Aligned reads. */
  if (likely(start == 0))
    CopyVectorSlices(dmem + offset, vector->slices);

  /* Unaligned reads. */
  else {
    uint16_t slices[8];

    if ((start & 0x1) == 1)
      debug("WARNING: LQV: Address not halfword aligned?");

    CopyVectorSlices(dmem + offset, slices);
    memcpy(vector->slices, slices + (start >> 1), 16 - start);
  }
}

/* ============================================================================
 *  RSPMemoryFunction: LoadRestVector
 * ========================================================================= */
void
LoadRestVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element, start;
  uint16_t slices[8];

  start = offset & 0xF;
  offset &= 0xFF0;

  CopyVectorSlices(dmem + offset, slices);
  memcpy(&vector->slices[8 - (start >> 1)], slices, start);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadShortVector
 * ========================================================================= */
void
LoadShortVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK, start;
  unsigned element = memoryData->element;
  uint8_t slices[16];

  start = offset & 0x1;

  /* Currently dont even bother to handle either of these. */
  assert((element & 0x1) == 0 && "Element references odd byte of slice?");
  assert(element <= 14 && "Would load past the 128-bit boundary.");

  if ((start & 0x1) == 1)
    debug("WARNING: LSV: Address not halfword aligned?");

  CopyVectorSlices(dmem + offset, slices);
  memcpy(vector->slices + (element >> 1), slices + start, 2);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadTransposeVector
 * ========================================================================= */
void
LoadTransposeVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  unsigned element = memoryData->element, dest, i, j;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  struct RSPCP2 *cp2 = memoryData->cp2;
  uint16_t slices[8];

  dest = (struct RSPVector*) memoryData->target - cp2->regs;
  offset &= 0xFF0;

  /* Currently dont even bother to handle either of these. */
  assert((offset & 0xF) == 0 && "Destination is not 128-bit aligned?");
  assert((element & 0x1) == 0 && "Element references odd byte of slice?");
  CopyVectorSlices(dmem + offset, slices);

  for (i = 0, j = 8 - (element >> 1); i < 8; i++, j = (j + 1) & 7)
    cp2->regs[dest + i].slices[j] = slices[i];
}

/* ============================================================================
 *  RSPMemoryFunction: LoadWord
 * ========================================================================= */
void
LoadWord(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  uint32_t *target = (uint32_t*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  uint32_t word;

  /* Load and sign extend. */
  memcpy(&word, dmem + offset, sizeof(word));
  *target = (uint32_t) ByteOrderSwap32(word);
}

/* ============================================================================
 *  RSPMemoryFunction: StoreByte
 * ========================================================================= */
void
StoreByte(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  uint8_t byte = memoryData->data;

  memcpy(dmem + offset, &byte, sizeof(byte));
}

/* ============================================================================
 *  RSPMemoryFunction: StoreByteVector
 * ========================================================================= */
void
StoreByteVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element;
  uint8_t slice[2];

  memcpy(&slice, vector->slices + (element >> 1), sizeof(slice));
  dmem[offset] = slice[(element & 1) ^ 1];
}

/* ============================================================================
 *  RSPMemoryFunction: StoreDoubleVector
 * ========================================================================= */
void
StoreDoubleVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element, start;
  uint16_t slices[16];

  start = offset & 0x7;

  /* Currently dont even bother to handle either of these. */
  assert((element & 0x1) == 0 && "Element references odd byte of slice?");
  assert(element <= 8 && "Would store past the 128-bit boundary.");

  if ((start & 0x1) == 1)
    debug("WARNING: SDV: Address not halfword aligned?");

  CopyVectorSlices(vector->slices, slices);
  memcpy(dmem + offset, slices + (element >> 1), 8);
}

/* ============================================================================
 *  RSPMemoryFunction: StoreHalf
 * ========================================================================= */
void
StoreHalf(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  uint16_t half = ByteOrderSwap16(memoryData->data);

  memcpy(dmem + offset, &half, sizeof(half));
}

/* ============================================================================
 *  RSPMemoryFunction: StoreLongVector
 * ========================================================================= */
void
StoreLongVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element, start;
  uint8_t slices[16];

  start = offset & 0x3;

  /* Currently dont even bother to handle either of these. */
  assert((element & 0x1) == 0 && "Element references odd byte of slice?");
  assert(element <= 12 && "Would store past the 128-bit boundary.");

  if ((start & 0x1) == 1)
    debug("WARNING: SLV: Address not halfword aligned?");

  /* TODO: Shift the element right 1? */
  CopyVectorSlices(vector->slices, slices);
  memcpy(dmem + offset, slices + element, 4);
}

/* ============================================================================
 *  RSPMemoryFunction: StorePackedByteVector
 * ========================================================================= */
void
StorePackedByteVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element, start;

  start = offset & 0x7;
  offset &= 0xFF8;

  /* Currently dont even bother to handle either of these. */
  assert(element == 0 && "Element something other than zero?");

  /* Aligned reads. */
  if (likely(start == 0))
    StorePackedBytes(vector->slices, dmem + offset);

  /* Unaligned reads. */
  else {
    uint16_t slices[8];

    /* TODO: FIXME. */
    assert(0);

    StorePackedBytes(dmem + offset + start, slices);
    memcpy(vector->slices, slices, 16 - start);
  }
}

/* ============================================================================
 *  RSPMemoryFunction: StorePackedFourthVector
 * ========================================================================= */
void
StorePackedFourthVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  assert(0);
}

/* ============================================================================
 *  RSPMemoryFunction: StorePackedHalfVector
 * ========================================================================= */
void
StorePackedHalfVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  assert(0);
}

/* ============================================================================
 *  RSPMemoryFunction: StorePackedVector
 * ========================================================================= */
void
StorePackedVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element, start;

  start = offset & 0x7;
  offset &= 0xFF8;

  /* Currently dont even bother to handle either of these. */
  assert(element == 0 && "Element something other than zero?");

  /* Aligned reads. */
  if (likely(start == 0))
    StorePackedUBytes(vector->slices, dmem + offset);

  /* Unaligned reads. */
  else {
    uint16_t slices[8];

    /* TODO: FIXME. */
    assert(0);

    StorePackedUBytes(dmem + offset + start, slices);
    memcpy(vector->slices, slices, 16 - start);
  }
}

/* ============================================================================
 *  RSPMemoryFunction: StoreQuadVector
 * ========================================================================= */
void
StoreQuadVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element, start;

  start = offset & 0xF;

  /* Currently dont even bother to handle either of these. */
  assert(element == 0 && "Element something other than zero?");
  assert((start & 0xF) < 8 && "Patent says this isn't valid?");

  /* Aligned writes. */
  if (likely(start == 0))
    CopyVectorSlices(vector->slices, dmem + offset);

  /* Unaligned writes. */
  else {
    uint16_t slices[8];

    if ((start & 0x1) == 1)
      debug("WARNING: SQV: Address not halfword aligned?");

    CopyVectorSlices(vector->slices, slices);
    memcpy(dmem + offset, slices, 16 - start);
  }
}

/* ============================================================================
 *  RSPMemoryFunction: StoreRestVector
 * ========================================================================= */
void
StoreRestVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  assert(0);
}

/* ============================================================================
 *  RSPMemoryFunction: StoreShortVector
 * ========================================================================= */
void
StoreShortVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element, start;
  uint8_t slices[16];

  start = offset & 0x1;

  /* Currently dont even bother to handle either of these. */
  assert(element <= 14 && "Would store past the 128-bit boundary.");

  if ((start & 0x1) == 1)
    debug("WARNING: SSV: Address not halfword aligned?");

  /* TODO: Shift the element right 1? */
  CopyVectorSlices(vector->slices, slices);
  memcpy(dmem + offset, slices + element, 2);
}

/* ============================================================================
 *  RSPMemoryFunction: StoreTransposeVector
 * ========================================================================= */
void
StoreTransposeVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  /* Currently dont even bother to handle either of these. */
  assert((offset & 0xF) == 0 && "Destination is not 128-bit aligned?");

  /* TODO: Check resulting byte ordering and output. */
  CopyVectorSlices(vector->slices, dmem + offset);
}

/* ============================================================================
 *  RSPMemoryFunction: StoreWord
 * ========================================================================= */
void
StoreWord(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  uint32_t word = ByteOrderSwap32(memoryData->data);

  memcpy(dmem + offset, &word, sizeof(word));
}

