/* ============================================================================
 *  WBStage.c: Writeback stage.
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
#include "Pipeline.h"

/* ============================================================================
 *  RSPWBStage: Writes results back to the register file.
 * ========================================================================= */
void
RSPWBStage(struct RSP *rsp) {
  struct RSPDFWBLatch *dfwbLatch = &rsp->pipeline.dfwbLatch;
  rsp->regs[dfwbLatch->result.dest] = dfwbLatch->result.data;
}

