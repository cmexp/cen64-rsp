/* ============================================================================
 *  CP2.h: RSP Coprocessor #2.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __RSP__CP2_H__
#define __RSP__CP2_H__
#include "Common.h"
#include "Decoder.h"

enum RSPVPRegister {
  RSP_VP_REGISTER_V0, RSP_VP_REGISTER_V1, RSP_VP_REGISTER_V2, 
  RSP_VP_REGISTER_V3, RSP_VP_REGISTER_V4, RSP_VP_REGISTER_V5,
  RSP_VP_REGISTER_V6, RSP_VP_REGISTER_V7, RSP_VP_REGISTER_V8,
  RSP_VP_REGISTER_V9, RSP_VP_REGISTER_V10, RSP_VP_REGISTER_V11,
  RSP_VP_REGISTER_V12, RSP_VP_REGISTER_V13, RSP_VP_REGISTER_V14,
  RSP_VP_REGISTER_V15, RSP_VP_REGISTER_V16, RSP_VP_REGISTER_V17,
  RSP_VP_REGISTER_V18, RSP_VP_REGISTER_V19, RSP_VP_REGISTER_V20,
  RSP_VP_REGISTER_V21, RSP_VP_REGISTER_V22, RSP_VP_REGISTER_V23,
  RSP_VP_REGISTER_V24, RSP_VP_REGISTER_V25, RSP_VP_REGISTER_V26,
  RSP_VP_REGISTER_V27, RSP_VP_REGISTER_V28, RSP_VP_REGISTER_V29,
  RSP_VP_REGISTER_V30, RSP_VP_REGISTER_V31, NUM_RSP_VP_REGISTERS,
};

struct RSPVector {
  uint16_t slices[8];
};

struct RSPCP2 {
  struct RSPVector regs[NUM_RSP_VP_REGISTERS];
  struct RSPVector accumulatorHigh;
  struct RSPVector accumulatorMid;
  struct RSPVector accumulatorLow;

  /* Be lazy; leave VCC/VCO ready for vector functions. */
  /* When it comes time to generate, use: _mm_movemask_epi8. */
  struct RSPVector compareCode;
  struct RSPVector carryOut;

  /* Having a larger array than necessary allows us to eliminate */
  /* a costly branch in the writeback stage every cycle. */
  bool locked[32 /* = NUM_RSP_SP_REGISTERS */ + NUM_RSP_VP_REGISTERS];

  /* Registers locked in the pipeline. */
  unsigned mulStageDest;
  unsigned accStageDest;

  /* Execution unit. */
  struct RSPVOpcode opcode;
  uint32_t iw;

#ifndef NDEBUG
  unsigned long long counts[NUM_RSP_VECTOR_OPCODES];
#endif
};

#ifndef NDEBUG
void RSPCP2GetAccumulator(const struct RSPCP2 *, unsigned , uint16_t *);
uint16_t RSPCP2GetCarryOut(const struct RSPCP2 *);
#endif

void RSPCycleCP2(struct RSPCP2 *);
void RSPInitCP2(struct RSPCP2 *);

#endif

