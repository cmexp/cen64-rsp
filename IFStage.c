/* ============================================================================
 *  IFStage.c: Instruction fetch stage.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Common.h"
#include "CPU.h"
#include "IFStage.h"
#include "Pipeline.h"

#ifdef __cplusplus
#include <cassert>
#include <cstring>
#else
#include <assert.h>
#include <string.h>
#endif

/* ============================================================================
 *  FetchInstructions: Fetches two instructions from a source address.
 * ========================================================================= */
static void
FetchInstructions(const uint8_t *source, uint32_t *iw1, uint32_t *iw2) {
  memcpy(iw1, source + 0, sizeof(*iw1));
  memcpy(iw2, source + 4, sizeof(*iw2));
  *iw1 = ByteOrderSwap32(*iw1);
  *iw2 = ByteOrderSwap32(*iw2);
}

/* ============================================================================
 *  RSPIFStage: Fetches, decodes, and checks for stall conditions.
 *  TODO: This is a massive hack as-is... fix it/the decoder.
 * ========================================================================= */
void
RSPIFStage(struct RSPIFRDLatch *ifrdLatch, const uint8_t mem[]) {
  uint32_t *firstIW = &ifrdLatch->firstIW;
  uint32_t *secondIW = &ifrdLatch->secondIW;
  uint32_t pc = ifrdLatch->pc;

  /* Save the PC of the fetched instructions. */
  ifrdLatch->fetchedPC = pc;

  /* Fetch a pair of instructions, bump the PC. */
  FetchInstructions(mem + ifrdLatch->pc, firstIW, secondIW);
  ifrdLatch->pc = ((pc + 4) & 0xFFC) | 0x1000;
}

