/* ============================================================================
 *  CPU.h: Reality Signal Processor (RSP).
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __RSP__CPU_H__
#define __RSP__CPU_H__
#include "Common.h"
#include "CP0.h"
#include "CP2.h"
#include "Externs.h"
#include "Pipeline.h"

#define RSP_DMEM_SIZE 4096
#define RSP_IMEM_SIZE 4096
#define RSP_DMEM_MASK (RSP_DMEM_SIZE - 1)
#define RSP_IMEM_MASK (RSP_IMEM_SIZE - 1)

extern const char *RSPBuildType;

#define RSP_LINK_REGISTER RSP_REGISTER_RA
enum RSPRegister {
  RSP_REGISTER_ZERO, RSP_REGISTER_AT, RSP_REGISTER_V0, RSP_REGISTER_V1,
  RSP_REGISTER_A0, RSP_REGISTER_A1, RSP_REGISTER_A2, RSP_REGISTER_A3,
  RSP_REGISTER_T0, RSP_REGISTER_T1, RSP_REGISTER_T2, RSP_REGISTER_T3,
  RSP_REGISTER_T4, RSP_REGISTER_R5, RSP_REGISTER_T6, RSP_REGISTER_T7,
  RSP_REGISTER_S0, RSP_REGISTER_S1, RSP_REGISTER_S2, RSP_REGISTER_S3,
  RSP_REGISTER_S4, RSP_REGISTER_S5, RSP_REGISTER_S6, RSP_REGISTER_S7,
  RSP_REGISTER_T8, RSP_REGISTER_T9, RSP_REGISTER_K0, RSP_REGISTER_K1,
  RSP_REGISTER_GP, RSP_REGISTER_SP, RSP_REGISTER_FP, RSP_REGISTER_RA,
  NUM_RSP_REGISTERS
};

struct RSP {
  uint8_t dmem[RSP_DMEM_SIZE];
  uint8_t imem[RSP_IMEM_SIZE];

  /* Having a larger array than necessary allows us to eliminate */
  /* a costly branch in the writeback stage every cycle. */
  uint32_t regs[NUM_RSP_REGISTERS + NUM_RSP_VP_REGISTERS + 1];

  struct BusController *bus;
  struct RSPCP0 cp0;
  struct RSPCP2 cp2;

  struct RSPPipeline pipeline;
  struct RDP *rdp;
};

struct RSP *CreateRSP(void);
void DestroyRSP(struct RSP *);
void *GetRSPDMEMPtr(const struct RSP *);
void *GetRSPIMEMPtr(const struct RSP *);

#ifdef DEBUG
void RSPDumpInstruction(uint32_t iw);
void RSPDumpOpcodeCounts(const struct RSP *rsp);
void RSPDumpRegisters(const struct RSP *);
#endif

#endif

