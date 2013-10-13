/* ============================================================================
 *  DFStage.c: Data fetch stage.
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
#include "DFStage.h"
#include "Memory.h"
#include "Pipeline.h"

#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

/* ============================================================================
 *  RSPDFStage: Reads or writes data from or to DMEM.
 * ========================================================================= */
void
RSPDFStage(struct RSP *rsp) {
  struct RSPEXDFLatch *exdfLatch = &rsp->pipeline.exdfLatch;
  struct RSPDFWBLatch *dfwbLatch = &rsp->pipeline.dfwbLatch;
  RSPMemoryFunction function;

  if ((function = exdfLatch->memoryData.function) != NULL) {
    function(&exdfLatch->memoryData, rsp->dmem);
    exdfLatch->memoryData.function = NULL;
  }

  dfwbLatch->result = exdfLatch->result;
}

