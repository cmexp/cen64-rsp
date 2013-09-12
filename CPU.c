/* ============================================================================
 *  CPU.c: Reality Shader Processor (RSP).
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
#include "Externs.h"
#include "Pipeline.h"

#ifdef __cplusplus
#include <cstdlib>
#include <cstring>
#else
#include <stdlib.h>
#include <string.h>
#endif

#ifndef USE_SSE
const char *RSPBuildType = "ANSI C";
#elif defined(SSSE3_ONLY)
const char *RSPBuildType = "SSSE3";
#else
const char *RSPBuildType = "SSE4.1";
#endif

static void InitRSP(struct RSP *);

/* ============================================================================
 *  ConnectRSPToBus: Connects a RSP instance to a Bus instance.
 * ========================================================================= */
void
ConnectRSPToBus(struct RSP *rsp, struct BusController *bus) {
  rsp->bus = bus;
}

/* ============================================================================
 *  CreateRSP: Creates and initializes an RSP instance.
 * ========================================================================= */
struct RSP *
CreateRSP(void) {
  struct RSP *rsp;

  if ((rsp = (struct RSP*) malloc(sizeof(struct RSP))) == NULL) {
    debug("Failed to allocate memory.");
    return NULL;
  }

  InitRSP(rsp);
  return rsp;
}

/* ============================================================================
 *  DestroyRSP: Releases any resources allocated for a RSP instance.
 * ========================================================================= */
void
DestroyRSP(struct RSP *rsp) {
  free(rsp);
}

/* ============================================================================
 *  GetRSPDMEMPtr: Returns a pointer to the RSP's DMEM.
 * ========================================================================= */
void *
GetRSPDMEMPtr(const struct RSP *rsp) {
  return (void*) rsp->dmem;
}

/* ============================================================================
 *  GetRSPIMEMPtr: Returns a pointer to the RSP's IMEM.
 * ========================================================================= */
void *
GetRSPIMEMPtr(const struct RSP *rsp) {
  return (void*) rsp->imem;
}

/* ============================================================================
 *  InitRSP: Initializes the RSP.
 * ========================================================================= */
static void
InitRSP(struct RSP *rsp) {
  debug("Initializing CPU.");
  memset(rsp, 0, sizeof(*rsp));

  rsp->bus = NULL;
  RSPInitCP0(&rsp->cp0);
  RSPInitCP2(&rsp->cp2);

  RSPInitPipeline(&rsp->pipeline);
  RDPSetRSPDMEMPointer(rsp->dmem);
}

#ifndef NDEBUG
#include <stdio.h>
/* ============================================================================
 *  RSPDumpInstruction: Prints the instruction's mnemonic and operands.
 * ========================================================================= */
void
RSPDumpInstruction(uint32_t iw) {
  const struct RSPOpcode *opcode = RSPDecodeInstruction(iw);

  if (opcode->infoFlags & OPCODE_INFO_VCOMP) {
    printf("%s ", RSPScalarOpcodeMnemonics[opcode->id]);
    printf("\n");
  }

  else {
    printf("%s ", RSPVectorOpcodeMnemonics[opcode->id]);
    printf("\n");
  }
}

/* ============================================================================
 *  RSPDumpOpcodeCounts: Prints counts of all executed opcodes.
 * ========================================================================= */
void
RSPDumpOpcodeCounts(const struct RSP *rsp) {
  unsigned i, j;
  debug("Scalar opcode counts:");

  for (i = 1; i < NUM_RSP_SCALAR_OPCODES; i += 4) {
    for (j = 0; j < 4 && i + j < NUM_RSP_SCALAR_OPCODES; j++)
      printf("%6s: %010llu  ", RSPScalarOpcodeMnemonics[i + j],
        rsp->pipeline.counts[i + j]);

    putc('\n', stdout);
  }

  putc('\n', stdout);
  debug("Vector opcode counts:");

  for (i = 1; i < NUM_RSP_VECTOR_OPCODES; i += 4) {
    for (j = 0; j < 4 && i + j < NUM_RSP_VECTOR_OPCODES; j++)
      printf("%6s: %010llu  ", RSPVectorOpcodeMnemonics[i + j],
        rsp->cp2.counts[i + j]);

    putc('\n', stdout);
  }

  putc('\n', stdout);
}

/* ============================================================================
 *  RSPDumpRegisters: Prints all registers to stdout.
 * ========================================================================= */
void
RSPDumpRegisters(const struct RSP *rsp) {
  unsigned i, j, k;
  debug("Scalar registers:");

  for (i = 0; i < NUM_RSP_REGISTERS; i += 4) {
    for (j = 0; j < 4 && j < NUM_RSP_REGISTERS; j++)
      printf("R%02u: 0x%.8X     ", i + j, rsp->regs[i + j]);

    putc('\n', stdout);
  }

  printf("PC:  0x%.8x\n", rsp->pipeline.ifrdLatch.pc - 4);

  putc('\n', stdout);
  debug ("Vector registers:                       [RSP]: Accumulators:");

  debugarg("VCO: 0x%.4X", RSPCP2GetCarryOut(&rsp->cp2));

  for (i = 0; i < 8; i++) {
    uint16_t acc[3];

    printf("V%02u: ", i);

    /* Each slice is 2 bytes, and there are 8 slices. */
    /* Print in reverse order on little endian. */
    for (k = 0; k < 8; k++) {
#ifndef LITTLE_ENDIAN
      printf("%04X", rsp->cp2.regs[i].slices[k]);
#else
      uint8_t *slice = (uint8_t*) &rsp->cp2.regs[i].slices[k];
      printf("%02X%02X", slice[0], slice[1]);
#endif

        if (k != 7)
          putc('|', stdout);
    }

    printf("   VACC%01u: ", i);
    RSPCP2GetAccumulator(&rsp->cp2, i, acc);

    /* Each slice is 2 bytes, and there are 3 slices. */
    for (k = 0; k < 3; k++) {
#ifndef LITTLE_ENDIAN
      printf("%04X", acc[k]);
#else
      uint8_t *slice = (uint8_t*) &acc[k];
      printf("%02X%02X", slice[0], slice[1]);
#endif

      if (k != 2)
        putc('|', stdout);
    }

    putc('\n', stdout);
  }

  for (i = 8; i < NUM_RSP_VP_REGISTERS; i++) {
    printf("V%02u: ", i);

    /* Each slice is 2 bytes, and there are 8 slices. */
    /* Print in reverse order on little endian. */
#ifndef LITTLE_ENDIAN
    for (k = 0; k < 8; k++) {
      printf("%04X", rsp->cp2.regs[i].slices[k]);
#else
    for (k = 0; k < 8; k++) {
      uint8_t *slice = (uint8_t*) &rsp->cp2.regs[i].slices[k];
      printf("%02X%02X", slice[0], slice[1]);
#endif

        if (k != 7)
          putc('|', stdout);
    }

    putc('\n', stdout);
  }

  putc('\n', stdout);
}
#endif

