/* ============================================================================
 *  IFStage.h: Instruction fetch stage.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __VR4300__IFSTAGE_H__
#define __VR4300__IFSTAGE_H__
#include "Common.h"
#include "CPU.h"
#include "Pipeline.h"

void RSPIFStage(struct RSPIFRDLatch *, const uint8_t []);

#endif

