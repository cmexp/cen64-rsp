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
RSPDFStage(struct RSPEXDFLatch *exdfLatch, struct RSPDFWBLatch *dfwbLatch,
  uint8_t dmem[], struct RSPVector vregs[]) {
  RSPMemoryFunction function;

  if ((function = exdfLatch->memoryData.function) != NULL) {
    unsigned dest = exdfLatch->result.dest;

    exdfLatch->memoryData.target = IS_VECTOR_DEST(dest)
      ? (void*) (&vregs[GET_VECTOR_DEST(exdfLatch->result.dest)])
      : (void*) (&exdfLatch->result.data);

    function(&exdfLatch->memoryData, dmem);
  }

  dfwbLatch->result = exdfLatch->result;
}

