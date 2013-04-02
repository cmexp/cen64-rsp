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
#include <cstring>
#else
#include <string.h>
#endif

#ifdef USE_SSE
#include <tmmintrin.h>
#endif

/* SSE-assisted helper functions. */
static void PackedHalfLoadVector(const uint16_t* src, uint16_t *dest);
static void PackedFourthLoadVector(const uint16_t* src, uint16_t *dest);
static void PackedLoadVector(const uint16_t* src, uint16_t *dest);
static void PackedByteLoadVector(const uint16_t* src, uint16_t *dest);
static void PackedFourthStoreVector(const uint16_t* src, uint16_t *dest);
static void PackedHalfStoreVector(const uint16_t* src, uint16_t *dest);
static void PackedStoreVector(const uint16_t* src, uint16_t *dest);
static void PackedByteStoreVector(const uint16_t* src, uint16_t *dest);

/* ============================================================================
 *  Loads odd bytes of 128-bit word into the upper byte of each vector slice.
 *  The bytes are loaded into the slices st the MSB is positioned at bit 14.
 * ========================================================================= */
static void
PackedHalfLoadVector(const uint16_t* src, uint16_t *dest) {
#ifdef USE_SSE
  __m128i temp, mask;

  static const uint8_t swapmask[] = {
    0x00, 0x80, 0x02, 0x80, 0x04, 0x80, 0x06, 0x80,
    0x08, 0x80, 0x0A, 0x80, 0x0C, 0x80, 0x0E, 0x80
  };

  temp = _mm_loadu_si128((__m128i*) src);
  mask = _mm_load_si128((__m128i*) swapmask);
  temp = _mm_shuffle_epi8(temp, mask);
  temp = _mm_slli_epi16(temp, 7);
  _mm_store_si128((__m128i*) dest, temp);

#else
#error "Unimplemented function: PackedHalfLoadVector (No SSE)."
#endif
}

/* ============================================================================
 *  Loads every fourth byte of 128-bit word into odd bytes at destination..
 *  The bytes are loaded into the slices st the MSB is positioned at bit 14.
 * ========================================================================= */
static void
PackedFourthLoadVector(const uint16_t* src, uint16_t *dest) {
#ifdef USE_SSE
  __m128i temp, mask;

  static const uint8_t swapmask[] = {
    0x00, 0x80, 0x04, 0x80, 0x08, 0x80, 0x0C, 0x80,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
  };

  temp = _mm_loadu_si128((__m128i*) src);
  mask = _mm_load_si128((__m128i*) swapmask);
  temp = _mm_shuffle_epi8(temp, mask);
  temp = _mm_slli_epi16(temp, 7);
  _mm_storel_epi64((__m128i*) dest, temp);

#else
#error "Unimplemented function: PackedFourthLoadVector (No SSE)."
#endif
}

/* ============================================================================
 *  Loads 8 consecutive bytes into the upper byte of each vector slice.
 *  The bytes are loaded into the slices st the MSB is positioned at bit 14.
 * ========================================================================= */
static void
PackedLoadVector(const uint16_t* src, uint16_t *dest) {
#ifdef USE_SSE
  __m128i temp, mask;

  static const uint8_t swapmask[] = {
    0x00, 0x80, 0x01, 0x80, 0x02, 0x80, 0x03, 0x80,
    0x04, 0x80, 0x05, 0x80, 0x06, 0x80, 0x07, 0x80
  };

  mask = _mm_load_si128((__m128i*) swapmask);
  temp = _mm_loadl_epi64((__m128i*) src);
  temp = _mm_shuffle_epi8(temp, mask);
  temp = _mm_slli_epi16(temp, 7);
  _mm_store_si128((__m128i*) dest, temp);

#else
#error "Unimplemented function: PackedLoadVector (No SSE)."
#endif
}

/* ============================================================================
 *  Loads 8 consecutive bytes into the upper byte of each vector slice.
 * ========================================================================= */
static void
PackedByteLoadVector(const uint16_t* src, uint16_t *dest) {
#ifdef USE_SSE
  __m128i temp, mask;

  static const uint8_t swapmask[] = {
    0x80, 0x00, 0x80, 0x01, 0x80, 0x02, 0x80, 0x03,
    0x80, 0x04, 0x80, 0x05, 0x80, 0x06, 0x80, 0x07
  };

  mask = _mm_load_si128((__m128i*) swapmask);
  temp = _mm_loadl_epi64((__m128i*) src);
  temp = _mm_shuffle_epi8(temp, mask);
  _mm_store_si128((__m128i*) dest, temp);

#else
#error "Unimplemented function: PackedByteLoadVector (No SSE)."
#endif
}

/* ============================================================================
 *  Stores upper byte of each half vector slice into 4th bytes of 128-bit word.
 *  Bytes are read from each slice st the MSB is positioned at bit 14.
 * ========================================================================= */
static void
PackedFourthStoreVector(const uint16_t* src, uint16_t *dest) {
#ifdef USE_SSE
  __m128i temp, mask;

  static const uint8_t swapmask[] = {
    0x01, 0x80, 0x80, 0x80, 0x03, 0x80, 0x80, 0x80,
    0x05, 0x80, 0x80, 0x80, 0x07, 0x80, 0x80, 0x80
  };

  temp = _mm_load_si128((__m128i*) src);
  mask = _mm_load_si128((__m128i*) swapmask);
  temp = _mm_slli_epi16(temp, 1);
  temp = _mm_shuffle_epi8(temp, mask);
  _mm_storeu_si128((__m128i*) dest, temp);

#else
#error "Unimplemented function: PackedFourthStoreVector (No SSE)."
#endif
}

/* ============================================================================
 *  Stores upper byte of each vector slice into odd bytes of 128-bit word.
 *  Bytes are read from each slice st the MSB is positioned at bit 14.
 * ========================================================================= */
static void
PackedHalfStoreVector(const uint16_t* src, uint16_t *dest) {
#ifdef USE_SSE
  __m128i temp, mask;

  static const uint8_t swapmask[] = {
    0x01, 0x80, 0x03, 0x80, 0x05, 0x80, 0x07, 0x80,
    0x09, 0x80, 0x0B, 0x80, 0x0D, 0x80, 0x0F, 0x80
  };

  temp = _mm_load_si128((__m128i*) src);
  mask = _mm_load_si128((__m128i*) swapmask);
  temp = _mm_slli_epi16(temp, 1);
  temp = _mm_shuffle_epi8(temp, mask);
  _mm_storeu_si128((__m128i*) dest, temp);

#else
#error "Unimplemented function: PackedHalfStoreVector (No SSE)."
#endif
}

/* ============================================================================
 *  Stores upper byte of each vector slice into 8 consecutive bytes.
 *  Bytes are read from each slice st the MSB is positioned at bit 14.
 * ========================================================================= */
static void
PackedStoreVector(const uint16_t* src, uint16_t *dest) {
#ifdef USE_SSE
  __m128i temp, mask;

  static const uint8_t swapmask[] = {
    0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
  };

  mask = _mm_load_si128((__m128i*) swapmask);
  temp = _mm_load_si128((__m128i*) src);
  temp = _mm_srli_epi16(temp, 7);
  temp = _mm_shuffle_epi8(temp, mask);
  _mm_storel_epi64((__m128i*) dest, temp);

#else
#error "Unimplemented function: PackedStoreVector (No SSE)."
#endif
}

/* ============================================================================
 *  Stores upper byte of each vector slice into 8 consecutive bytes.
 * ========================================================================= */
static void
PackedByteStoreVector(const uint16_t* src, uint16_t *dest) {
#ifdef USE_SSE
  __m128i temp, mask;

  static const uint8_t swapmask[] = {
    0x01, 0x03, 0x05, 0x07, 0x09, 0x0B, 0x0D, 0x0F,
    0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80
  };

  mask = _mm_load_si128((__m128i*) swapmask);
  temp = _mm_load_si128((__m128i*) src);
  temp = _mm_shuffle_epi8(temp, mask);
  _mm_storel_epi64((__m128i*) dest, temp);

#else
#error "Unimplemented function: PackedByteStoreVector (No SSE)."
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
  unsigned element = memoryData->element;
  unsigned offsetMask;

  /* Might have to do split the element in half. */
  uint8_t *slice = (uint8_t*) vector->slices + element;

  /* Partial copy? */
  if (unlikely(((offsetMask = (offset & 0xF))) > 8 || element > 8)) {
    unsigned readSize = (offsetMask > element)
      ? 16 - offsetMask
      : 16 - element;

    memcpy(slice, dmem + offset, readSize);
    return;
  }

  /* Full copy. */
  memcpy(slice, dmem + offset, sizeof(uint16_t) * 4);
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
  unsigned offsetMask;

  /* Might have to do split the element in half. */
  uint8_t *slice = (uint8_t*) vector->slices + element;

  /* Partial copy? */
  if (unlikely(((offsetMask = (offset & 0xF))) > 12 || element > 12)) {
    unsigned readSize = (offsetMask > element)
      ? 16 - offsetMask
      : 16 - element;

    memcpy(slice, dmem + offset, readSize);
    return;
  }

  /* Full copy. */
  memcpy(slice, dmem + offset, sizeof(uint16_t) * 2);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadPackedByteVector
 * ========================================================================= */
void
LoadPackedByteVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  PackedByteLoadVector((uint16_t*) (dmem + offset), vector->slices);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadPackedFourthVector
 * ========================================================================= */
void
LoadPackedFourthVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element;

  PackedFourthLoadVector((uint16_t*) (dmem + offset), vector->slices + element);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadPackedHalfVector
 * ========================================================================= */
void
LoadPackedHalfVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  PackedHalfLoadVector((uint16_t*) (dmem + offset), vector->slices);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadPackedVector
 * ========================================================================= */
void
LoadPackedVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  PackedLoadVector((uint16_t*) (dmem + offset), vector->slices);
}

/* ============================================================================
 *  RSPMemoryFunction: LoadQuadVector
 * ========================================================================= */
void
LoadQuadVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  /* Unaligned loads. */
  if (unlikely(offset & 0xF)) {
    uint8_t *slice = (uint8_t*) vector->slices;
    unsigned end = (offset + 15) & 0xFF0;

    memcpy(slice - (offset & 0xFF0), dmem + offset, end - offset);
    return;
  }

  /* Aligned loads. */
  memcpy(vector->slices, dmem + offset, sizeof(vector->slices));
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
  unsigned offsetMask;

  /* Might have to do split the element in half. */
  uint8_t *slice = (uint8_t*) vector->slices + element;

  /* Partial copy? */
  if (unlikely(((offsetMask = (offset & 0xF))) > 14 || element > 14)) {
    unsigned readSize = (offsetMask > element)
      ? 16 - offsetMask
      : 16 - element;

    memcpy(slice, dmem + offset, readSize);
    return;
  }

  /* Full copy. */
  memcpy(slice, dmem + offset, sizeof(uint16_t));
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

  memcpy(dmem + offset, slice + byte, sizeof(*dmem));
}

/* ============================================================================
 *  RSPMemoryFunction: StoreDoubleVector
 * ========================================================================= */
void
StoreDoubleVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element;
  unsigned offsetMask;

  /* Might have to do split the element in half. */
  uint8_t *slice = (uint8_t*) vector->slices + element;

  /* Partial copy? */
  if (unlikely(((offsetMask = (offset & 0xF))) > 8 || element > 8)) {
    unsigned readSize = (offsetMask > element)
      ? 16 - offsetMask
      : 16 - element;

    memcpy(dmem + offset, slice, readSize);
    return;
  }

  /* Full copy. */
  memcpy(dmem + offset, slice, sizeof(uint16_t) * 4);
}

/* ============================================================================
 *  RSPMemoryFunction: StoreHalf
 * ========================================================================= */
void
StoreHalf(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  uint16_t half = memoryData->data;

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
  unsigned offsetMask;

  /* Might have to do split the element in half. */
  uint8_t *slice = (uint8_t*) vector->slices + element;

  /* Partial copy? */
  if (unlikely(((offsetMask = (offset & 0xF))) > 12 || element > 12)) {
    unsigned readSize = (offsetMask > element)
      ? 16 - offsetMask
      : 16 - element;

    memcpy(dmem + offset, slice, readSize);
    return;
  }

  /* Full copy. */
  memcpy(dmem + offset, slice, sizeof(uint16_t) * 2);
}

/* ============================================================================
 *  RSPMemoryFunction: StorePackedByteVector
 * ========================================================================= */
void
StorePackedByteVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  PackedByteStoreVector(vector->slices, (uint16_t*) (dmem + offset));
}

/* ============================================================================
 *  RSPMemoryFunction: StorePackedFourthVector
 * ========================================================================= */
void
StorePackedFourthVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  unsigned element = memoryData->element;

  PackedFourthStoreVector(vector->slices + element, (uint16_t*)(dmem + offset));
}

/* ============================================================================
 *  RSPMemoryFunction: StorePackedHalfVector
 * ========================================================================= */
void
StorePackedHalfVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  PackedHalfStoreVector(vector->slices, (uint16_t*) (dmem + offset));
}

/* ============================================================================
 *  RSPMemoryFunction: StorePackedVector
 * ========================================================================= */
void
StorePackedVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  PackedStoreVector(vector->slices, (uint16_t*) (dmem + offset));
}

/* ============================================================================
 *  RSPMemoryFunction: StoreQuadVector
 * ========================================================================= */
void
StoreQuadVector(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  struct RSPVector *vector = (struct RSPVector*) memoryData->target;
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;

  /* Unaligned loads. */
  if (unlikely(offset & 0xF)) {
    uint8_t *slice = (uint8_t*) vector->slices;
    unsigned end = (offset + 15) & 0xFF0;

    memcpy(dmem + offset, slice - (offset & 0xFF0), end - offset);
    return;
  }

  /* Aligned loads. */
  memcpy(dmem + offset, vector->slices, sizeof(vector->slices));
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
  unsigned offsetMask;

  /* Might have to do split the element in half. */
  uint8_t *slice = (uint8_t*) vector->slices + element;

  /* Partial copy? */
  if (unlikely(((offsetMask = (offset & 0xF))) > 14 || element > 14)) {
    unsigned readSize = (offsetMask > element)
      ? 16 - offsetMask
      : 16 - element;

    memcpy(dmem + offset, slice, readSize);
    return;
  }

  /* Full copy. */
  memcpy(dmem + offset, slice, sizeof(uint16_t));
}

/* ============================================================================
 *  RSPMemoryFunction: StoreWord
 * ========================================================================= */
void
StoreWord(const struct RSPMemoryData *memoryData, uint8_t *dmem) {
  unsigned offset = memoryData->offset & RSP_DMEM_MASK;
  uint32_t word = memoryData->data;

  memcpy(dmem + offset, &word, sizeof(word));
}

