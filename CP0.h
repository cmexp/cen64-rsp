/* ============================================================================
 *  CP0.h: RSP Coprocessor #0.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __RSP__CP0_H__
#define __RSP__CP0_H__
#include "Common.h"

enum SPRegister {
#define X(reg) reg,
#include "Registers.md"
#undef X
  NUM_SP_REGISTERS
};

#ifndef NDEBUG
extern const char *SPRegisterMnemonics[NUM_SP_REGISTERS];
#endif

struct RSPCP0 {
  uint32_t regs[NUM_SP_REGISTERS];
};

void RSPInitCP0(struct RSPCP0 *);

#endif

