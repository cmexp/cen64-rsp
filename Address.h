/* ============================================================================
 *  Address.h: Device address list.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __RSP__ADDRESS_H__
#define __RSP__ADDRESS_H__

/* RSP/DMEM. */
#define RSP_DMEM_BASE_ADDRESS     0x04000000
#define RSP_DMEM_ADDRESS_LEN      0x00001000

/* RSP/IMEM. */
#define RSP_IMEM_BASE_ADDRESS     0x04001000
#define RSP_IMEM_ADDRESS_LEN      0x00001000

/* Serial Interface Registers. */
#define SI_REGS_BASE_ADDRESS      0x04800000

/* SP Interface Registers. */
#define SP_REGS_BASE_ADDRESS      0x04040000
#define SP_REGS_ADDRESS_LEN       0x00000020

/* SP Interface Registers [2]. */
#define SP_REGS2_BASE_ADDRESS     0x04080000
#define SP_REGS2_ADDRESS_LEN      0x00000008

#endif

