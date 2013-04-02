/* ============================================================================
 *  WBStage.h: Writeback stage.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __RSP__WBSTAGE_H__
#define __RSP__WBSTAGE_H__
#include "Common.h"
#include "CPU.h"
#include "CP2.h"
#include "Pipeline.h"

void RSPWBStage(const struct RSPDFWBLatch *,
  uint32_t [], struct RSPCP2 *);

#endif

