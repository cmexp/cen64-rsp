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
#ifndef __RSP__MEMORY_H__
#define __RSP__MEMORY_H__
#include "Common.h"
#include "CP2.h"

struct RSPMemoryData;
typedef void (*RSPMemoryFunction)(const struct RSPMemoryData *, uint8_t *);

struct RSPMemoryData{
  RSPMemoryFunction function;

  void *target;
  unsigned element;
  unsigned offset;
  uint32_t data;
};

void LoadByte(const struct RSPMemoryData *, uint8_t *);
void LoadByteVector(const struct RSPMemoryData *, uint8_t *);
void LoadByteUnsigned(const struct RSPMemoryData *, uint8_t *);
void LoadDoubleVector(const struct RSPMemoryData *, uint8_t *);
void LoadHalf(const struct RSPMemoryData *, uint8_t *);
void LoadHalfUnsigned(const struct RSPMemoryData *, uint8_t *);
void LoadLongVector(const struct RSPMemoryData *, uint8_t *);
void LoadPackedByteVector(const struct RSPMemoryData *, uint8_t *);
void LoadPackedFourthVector(const struct RSPMemoryData *, uint8_t *);
void LoadPackedHalfVector(const struct RSPMemoryData *, uint8_t *);
void LoadPackedVector(const struct RSPMemoryData *, uint8_t *);
void LoadQuadVector(const struct RSPMemoryData *, uint8_t *);
void LoadRestVector(const struct RSPMemoryData *, uint8_t *);
void LoadShortVector(const struct RSPMemoryData *, uint8_t *);
void LoadTransposeVector(const struct RSPMemoryData *, uint8_t *);
void LoadWord(const struct RSPMemoryData *, uint8_t *);
void StoreByte(const struct RSPMemoryData *, uint8_t *);
void StoreByteVector(const struct RSPMemoryData *, uint8_t *);
void StoreDoubleVector(const struct RSPMemoryData *, uint8_t *);
void StoreHalf(const struct RSPMemoryData *, uint8_t *);
void StoreLongVector(const struct RSPMemoryData *, uint8_t *);
void StorePackedByteVector(const struct RSPMemoryData *, uint8_t *);
void StorePackedFourthVector(const struct RSPMemoryData *, uint8_t *);
void StorePackedHalfVector(const struct RSPMemoryData *, uint8_t *);
void StorePackedVector(const struct RSPMemoryData *, uint8_t *);
void StoreQuadVector(const struct RSPMemoryData *, uint8_t *);
void StoreRestVector(const struct RSPMemoryData *, uint8_t *);
void StoreShortVector(const struct RSPMemoryData *, uint8_t *);
void StoreTransposeVector(const struct RSPMemoryData *, uint8_t *);
void StoreWord(const struct RSPMemoryData *, uint8_t *);

#endif

