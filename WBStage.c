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
#include "Pipeline.h"

/* ============================================================================
 *  RSPWBStage: Writes results back to the register file.
 * ========================================================================= */
void
RSPWBStage(const struct RSPDFWBLatch *dfwbLatch,
  uint32_t regs[], struct RSPCP2 *cp2) {
  regs[dfwbLatch->result.dest] = dfwbLatch->result.data;
  cp2->locked[dfwbLatch->result.dest] = false;
}

