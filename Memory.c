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
static void CopyVectorSlices(void *src, void *dest);

/* ============================================================================
 *  Swaps the byte order of all slices within a vector.
 * ========================================================================= */
static void
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
  unsigned i;

  for (i = 0; i < 8; i += 2) {
    ((uint8_t*) dest)[i + 0] = ((uint8_t*) src)[i + 1];
    ((uint8_t*) dest)[i + 1] = ((uint8_t*) src)[i + 0];
  }
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

  memcpy(slice + element, dmem + offset, sizeof(*dmem));
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
  unsigned offsetMask, element = memoryData->element;
  uint8_t data[16];

  /* Might have to do split the element in half. */
  uint8_t *slice = (uint8_t*) vector->slices + element;
  CopyVectorSlices(dmem + (offset & 0xF), data);

  /* Partial copy? */
  if (unlikely(((offsetMask = (offset & 0xF))) > 8 || element > 8)) {
    unsigned readSize = (offsetMask > element)
      ? 16 - offsetMask
      : 16 - element;

    memcpy(slice, data + (offset & 0xF), readSize);
    return;
  }

  /* Full copy. */
  memcpy(slice, data + (offset & 0xF), sizeof(uint16_t) * 4);
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
  unsigned element = memoryData->element;

  /* TODO: This function is likely buggy/wrong. */
  debug("WARNING: Unimplemented llv not correct...");
}

/* ============================================================================
 *  RSPMemoryFunction: LoadPackedByteVector
 * ========================================================================= */
void
LoadPackedByteVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  /* TODO: This function is likely buggy/wrong. */
  debug("WARNING: Unaligned lpv not correct...");
}

/* ============================================================================
 *  RSPMemoryFunction: LoadPackedFourthVector
 * ========================================================================= */
void
LoadPackedFourthVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element;

  /* TODO: Check. */
  assert(0);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadPackedHalfVector
 * ========================================================================= */
void
LoadPackedHalfVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  /* TODO: Check. */
  assert(0);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadPackedVector
 * ========================================================================= */
void
LoadPackedVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  /* TODO: This function is likely buggy/wrong. */
  debug("WARNING: Unaligned lpv not correct...");
}

/* ============================================================================
 *  RSPMemoryFunction: LoadQuadVector
 * ========================================================================= */
void
LoadQuadVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element;

  /* Unaligned loads. */
  if (unlikely(offset & 0xF || element != 0)) {
    unsigned i, end = (offset + 15) & 0xFF0;

    /* TODO: Handle edge case (when element != 0). */
    uint8_t *slice = (uint8_t*) vector->slices + element, data[16];
    CopyVectorSlices(data, dmem + (offset & 0xFF0));
    assert(element == 0);

    /* TODO: This function is likely buggy/wrong. */
    debug("WARNING: Unaligned lqv not correct...");

    for (i = 0; i < end - offset; i++)
      slice[i] = data[element + i];

    return;
  }

  /* Aligned loads. */
  CopyVectorSlices(dmem + offset, vector->slices);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadRestVector
 * ========================================================================= */
void
LoadRestVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  uint8_t *slice = (uint8_t*) vector->slices;
  unsigned start = offset & 0xFF0;

  /* TODO: Check. */
  assert(0);

  memcpy(slice + (16 - offset), dmem + start, offset - start);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadShortVector
 * ========================================================================= */
void
LoadShortVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element;

  /* TODO: This function is likely buggy/wrong. */
  debug("WARNING: Unimplemented lsv not correct...");
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
  uint8_t *slice = (uint8_t*) vector->slices;
  unsigned byte = memoryData->element;

  /* TODO: Check for correctness. */
  dmem[offset] = slice[byte ^ 1];
}

/* ============================================================================
 *  RSPMemoryFunction: StoreDoubleVector
 * ========================================================================= */
void
StoreDoubleVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element;

  /* TODO: This function is likely buggy/wrong. */
  debug("WARNING: Unimplemented sdv not correct...");
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
  unsigned element = memoryData->element;

  /* TODO: This function is likely buggy/wrong. */
  debug("WARNING: Unimplemented slv not correct...");
}

/* ============================================================================
 *  RSPMemoryFunction: StorePackedByteVector
 * ========================================================================= */
void
StorePackedByteVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  /* TODO: This function is likely buggy/wrong. */
  debug("WARNING: Unimplemented spv not correct...");
}

/* ============================================================================
 *  RSPMemoryFunction: StorePackedFourthVector
 * ========================================================================= */
void
StorePackedFourthVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element;

  /* TODO: Check. */
  assert(0);
}

/* ============================================================================
 *  RSPMemoryFunction: StorePackedHalfVector
 * ========================================================================= */
void
StorePackedHalfVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  /* TODO: Check. */
  assert(0);
}

/* ============================================================================
 *  RSPMemoryFunction: StorePackedVector
 * ========================================================================= */
void
StorePackedVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  /* TODO: This function is likely buggy/wrong. */
  debug("WARNING: Unimplemented spv not correct...");
}

/* ============================================================================
 *  RSPMemoryFunction: StoreQuadVector
 * ========================================================================= */
void
StoreQuadVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element;

  /* Unaligned stores. */
  if (unlikely(offset & 0xF || element != 0)) {
    unsigned i, end = element + 16 - (offset & 0xF);

    /* TODO: Handle edge case (when element != 0). */
    uint8_t *slice = (uint8_t*) vector->slices + element, data[16];
    CopyVectorSlices(dmem + (offset & 0xF), data);
    assert(element == 0);

    /* TODO: This function is likely buggy/wrong. */
    debug("WARNING: Unaligned sqv not correct...");
    for (i = element; i < end; i++)
      dmem[offset + i] = slice[i];

    return;
  }

  /* Aligned stores. */
  CopyVectorSlices(vector->slices, dmem + offset);
}

/* ============================================================================
 *  RSPMemoryFunction: StoreRestVector
 * ========================================================================= */
void
StoreRestVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  uint8_t *slice = (uint8_t*) vector->slices;
  unsigned start = offset & 0xFF0;

  /* TODO: Check. */
  assert(0);

  memcpy(dmem + start, slice + (16 - offset), offset - start);
}

/* ============================================================================
 *  RSPMemoryFunction: StoreShortVector
 * ========================================================================= */
void
StoreShortVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element;

  /* TODO: This function is likely buggy/wrong. */
  debug("WARNING: Unimplemented lsv not correct...");
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

