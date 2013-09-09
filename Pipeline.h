/* ============================================================================
 *  Pipeline.h: Processor pipeline.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __RSP__PIPELINE_H__
#define __RSP__PIPELINE_H__
#include "Common.h"
#include "Decoder.h"
#include "Memory.h"

enum RSPPipelineStages {
  RSP_PIPELINE_STAGE_IF,
  RSP_PIPELINE_STAGE_RD,
  RSP_PIPELINE_STAGE_EX,
  RSP_PIPELINE_STAGE_DC,
  RSP_PIPELINE_STAGE_WB,
  NUM_RSP_PIPELINE_STAGES
};

struct RSPScalarResult {
  uint32_t data, dest;
};

struct RSPIFRDLatch {
  uint32_t firstIW, secondIW;
  uint32_t fetchedPC, pc;
};

struct RSPRDEXLatch {
  uint32_t *pc, iw;
  struct RSPOpcode opcode;
};

struct RSPEXDFLatch {
  struct RSPOpcode opcode;
  struct RSPScalarResult result;
  struct RSPMemoryData memoryData;
};

struct RSPDFWBLatch {
  struct RSPOpcode opcode;
  struct RSPScalarResult result;
};

struct RSPPipeline {
  struct RSPIFRDLatch ifrdLatch;
  struct RSPDFWBLatch dfwbLatch;
  struct RSPRDEXLatch rdexLatch;
  struct RSPEXDFLatch exdfLatch;

#ifndef NDEBUG
  unsigned long long counts[NUM_RSP_SCALAR_OPCODES];
  unsigned long long cycles, stalls;
#endif
};

struct RSP;

void CycleRSP(struct RSP *);
void RSPInitPipeline(struct RSPPipeline *);

#endif

