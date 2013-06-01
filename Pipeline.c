/* ============================================================================
 *  Pipeline.c: Processor pipeline.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Common.h"
#include "CP0.h"
#include "CP2.h"
#include "CPU.h"
#include "Decoder.h"
#include "DFStage.h"
#include "EXStage.h"
#include "IFStage.h"
#include "Opcodes.h"
#include "Pipeline.h"
#include "RDStage.h"
#include "WBStage.h"

#ifdef __cplusplus
#include <cassert>
#include <cstring>
#else
#include <assert.h>
#include <string.h>
#endif

/* ============================================================================
 *  IsLoadStoreStall: Determines if a scalar load/store condition is present.
 * ========================================================================= */
static bool
IsLoadStoreStall(uint32_t rdInfoFlags, uint32_t dfInfoFlags) {
  uint32_t copsOpcodeInfoMask = OPCODE_INFO_CP0 | OPCODE_INFO_CP2;
  uint32_t dfStallConditionMask = OPCODE_INFO_LOAD | copsOpcodeInfoMask;
  uint32_t rdStallConditionMask = OPCODE_INFO_STORE | copsOpcodeInfoMask;

  return (dfInfoFlags & dfStallConditionMask) &&
         (rdInfoFlags & rdStallConditionMask)
         ? true : false;
}

/* ============================================================================
 *  IsLoadUseStall: Determines a scalar load/use condition is present.
 * ========================================================================= */
static bool
IsLoadUseStall(const struct RSPRDEXLatch *rdexLatch, uint32_t exInfoFlags,
  uint32_t dfInfoFlags, uint32_t exDestination, uint32_t dfDestination) {
  uint32_t rdInfoFlags = rdexLatch->opcode.infoFlags;
  bool rdRequiresRS = rdInfoFlags & OPCODE_INFO_NEED_RS;
  bool rdRequiresRT = rdInfoFlags & OPCODE_INFO_NEED_RT;
  uint32_t rsSource = GET_RS(rdexLatch->iw);
  uint32_t rtSource = GET_RT(rdexLatch->iw);

  if ((exInfoFlags & OPCODE_INFO_LOAD) && 
    ((rdRequiresRS && rsSource == exDestination) ||
    (rdRequiresRT && rtSource == exDestination)))
    return true;

  else if ((dfInfoFlags & OPCODE_INFO_LOAD) &&
    ((rdRequiresRS && rsSource == dfDestination) ||
    (rdRequiresRT && rtSource == dfDestination)))
    return true;

  return false;
}

/* ============================================================================
 *  CycleRSP: Advances the state of the processor pipeline one cycle.
 * ========================================================================= */
void
CycleRSP(struct RSP *rsp) {
  struct RSPIFRDLatch *ifrdLatch = &rsp->pipeline.ifrdLatch;
  struct RSPRDEXLatch *rdexLatch = &rsp->pipeline.rdexLatch;
  struct RSPEXDFLatch *exdfLatch = &rsp->pipeline.exdfLatch;
  struct RSPDFWBLatch *dfwbLatch = &rsp->pipeline.dfwbLatch;

  /* Generate outputs for later stages. */ 
  struct RSPOpcode dfOpcode = exdfLatch->opcode;
  struct RSPOpcode exOpcode = rdexLatch->opcode;

  /* If we're halted, just bail out. */
  if (rsp->cp0.regs[SP_STATUS_REG] & 0x1)
    return;

  RSPWBStage(dfwbLatch, rsp->regs, &rsp->cp2);
  RSPDFStage(exdfLatch, dfwbLatch, rsp->dmem, rsp->cp2.regs);

  /* Execute and bump opcode counters. */
  uint32_t pcBeforeExecute = ifrdLatch->pc;
  RSPEXStage(rsp);
  RSPCycleCP2(&rsp->cp2);

#ifndef NDEBUG
  rsp->pipeline.counts[exOpcode.id]++;
#endif

  /* Decode the instructions sitting in the issue queue. */
  bool didBranch = pcBeforeExecute != ifrdLatch->pc;
  RSPRDStage(ifrdLatch, rdexLatch, didBranch, &rsp->cp2);

  /* Check for stall conditions. */
  bool ldStoreStall, ldUseStall;

  struct RSPOpcode rfOpcode = rdexLatch->opcode;
  ldStoreStall = IsLoadStoreStall(rfOpcode.infoFlags, dfOpcode.infoFlags);
  ldUseStall = IsLoadUseStall(rdexLatch, exOpcode.infoFlags, exOpcode.infoFlags,
    exdfLatch->result.dest, dfwbLatch->result.dest);

  /* Fetch if there were no stalls. */
  if (!ldStoreStall && !ldUseStall)
    RSPIFStage(ifrdLatch, rsp->dmem);
  else {
    RSPInvalidateOpcode(&rdexLatch->opcode);
    RSPInvalidateVectorOpcode(&rsp->cp2.opcode);
  }
}

/* ============================================================================
 *  RSPInitPipeline: Initializes the pipeline.
 * ========================================================================= */
void
RSPInitPipeline(struct RSPPipeline *pipeline) {
  memset(pipeline, 0, sizeof(*pipeline));
  pipeline->rdexLatch.pc = &pipeline->ifrdLatch.pc;
  RSPInvalidateOpcode(&pipeline->rdexLatch.opcode);
  RSPInvalidateOpcode(&pipeline->exdfLatch.opcode);
  pipeline->ifrdLatch.pc = 0x1000;
}

