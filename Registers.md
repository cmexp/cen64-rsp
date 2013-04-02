/* ============================================================================
 *  Registers.md: RSP registers.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef SP_REGISTER_LIST
#define SP_REGISTER_LIST \
  X(SP_MEM_ADDR_REG) \
  X(SP_DRAM_ADDR_REG) \
  X(SP_RD_LEN_REG) \
  X(SP_WR_LEN_REG) \
  X(SP_STATUS_REG) \
  X(SP_DMA_FULL_REG) \
  X(SP_DMA_BUSY_REG) \
  X(SP_SEMAPHORE_REG) \
  X(CMD_START) \
  X(CMD_END) \
  X(CMD_CURRENT) \
  X(CMD_STATUS) \
  X(CMD_CLOCK) \
  X(CMD_BUSY) \
  X(CMD_PIPE_BUSY) \
  X(CMD_TMEM_BUSY) \
  X(SP_PC_REG) \
  X(SP_IBIST_REG)
#endif

SP_REGISTER_LIST

