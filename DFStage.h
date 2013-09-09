/* ============================================================================
 *  DFStage.h: Data fetch stage.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __RSP__DFSTAGE_H__
#define __RSP__DFSTAGE_H__
#include "Common.h"
#include "CP2.h"
#include "Pipeline.h"

void RSPDFStage(struct RSPEXDFLatch *, struct RSPDFWBLatch *, uint8_t []);

#endif

