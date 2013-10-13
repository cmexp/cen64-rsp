/* ============================================================================
 *  RDStage.c: Register access and instruction decode stage.
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
#include "Decoder.h"
#include "Opcodes.h"
#include "RDStage.h"

static bool IsBranch(const struct RSPOpcode *);
static bool IsVector(const struct RSPOpcode *);

/* ============================================================================
 *  CanDualIssue: Returns true if the conditions are met for dual-issuing.
 * ========================================================================= */
static bool
CanDualIssue(bool isBranch, bool didBranch, bool inDelaySlot, uint32_t pc) {
  return !(inDelaySlot || isBranch || (didBranch && (pc & 0x7)));
}

/* ============================================================================
 *  IsBranch: Returns true if the opcode is a branch opcode.
 * ========================================================================= */
static bool
IsBranch(const struct RSPOpcode *opcode) {
  return opcode->infoFlags & OPCODE_INFO_BRANCH;
}

/* ============================================================================
 *  IsVector: Returns true if the opcode is a vector computational opcode..
 * ========================================================================= */
static bool
IsVector(const struct RSPOpcode *opcode) {
  return opcode->infoFlags & OPCODE_INFO_VCOMP;
}

/* ============================================================================
 *  RSPRDStage: Decodes the instruction words and checks for stalls.
 * ========================================================================= */
void
RSPRDStage(struct RSP *rsp) {
  struct RSPIFRDLatch *ifrdLatch = &rsp->pipeline.ifrdLatch;
  struct RSPRDEXLatch *rdexLatch = &rsp->pipeline.rdexLatch;
  bool isFirstIWBranchType, isFirstIWVectorType, inDelaySlot;
  const struct RSPOpcode *firstOpcode, *secondOpcode;
  uint32_t fetchedPC = ifrdLatch->fetchedPC;

  /* Classify the instruction that was fetched first. */
  firstOpcode = RSPDecodeInstruction(ifrdLatch->firstIW);
  isFirstIWBranchType = IsBranch(firstOpcode);
  isFirstIWVectorType = IsVector(firstOpcode);
  inDelaySlot = IsBranch(&rdexLatch->opcode);

#if 0
  /* Might not dual issue based on prior logic in some (niche?) cases. */
  /* Examples: Target of taken branch and not doubleword aligned, etc... */
  if (CanDualIssue(isFirstIWBranchType, didBranch, inDelaySlot, fetchedPC)) {
    secondOpcode = RSPDecodeInstruction(ifrdLatch->secondIW);

    /* We can dual-issue; just make sure types differ. */
    if (isFirstIWVectorType != IsVector(secondOpcode)) {

      if (isFirstIWVectorType) {
        rdexLatch->iw = ifrdLatch->secondIW;
        cp2->iw = ifrdLatch->firstIW;

        rdexLatch->opcode = *secondOpcode;
        cp2->opcode = *RSPDecodeVectorInstruction(cp2->iw);
      }

      else {
        rdexLatch->iw = ifrdLatch->firstIW;
        cp2->iw = ifrdLatch->secondIW;

        rdexLatch->opcode = *firstOpcode;
        cp2->opcode = *RSPDecodeVectorInstruction(cp2->iw);
      }

      /* TODO: HACK! Fix it, and the fetcher... */
      ifrdLatch->pc = (ifrdLatch->fetchedPC + 8) & 0xFFC;
      return;
    }
  }
#endif

  /* Cannot dual issue. */
  if (isFirstIWVectorType) {
    rsp->cp2.iw = ifrdLatch->firstIW;

    rsp->cp2.opcode = *RSPDecodeVectorInstruction(rsp->cp2.iw);
    RSPInvalidateOpcode(&rdexLatch->opcode);
  }

  else {
    rdexLatch->iw = ifrdLatch->firstIW;

    rdexLatch->opcode = *firstOpcode;
    RSPInvalidateVectorOpcode(&rsp->cp2.opcode);
  }
}

