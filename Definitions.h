/* ============================================================================
 *  Defs.h: Reality Signal Processor (RSP) Defines.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __RSP__DEFS_H__
#define __RSP__DEFS_H__
#include "Common.h"

/* MI_INTR_REG bits. */
#define MI_INTR_SP                0x01

/* SP_STATUS_REG read bits. */
#define SP_STATUS_HALT            0x0001
#define SP_STATUS_BROKE           0x0002
#define SP_STATUS_DMA_BUSY        0x0004
#define SP_STATUS_DMA_FULL        0x0008
#define SP_STATUS_IO_FULL         0x0010
#define SP_STATUS_SSTEP           0x0020
#define SP_STATUS_INTR_BREAK      0x0040
#define SP_STATUS_SIG0            0x0080
#define SP_STATUS_SIG1            0x0100
#define SP_STATUS_SIG2            0x0200
#define SP_STATUS_SIG3            0x0400
#define SP_STATUS_SIG4            0x0800
#define SP_STATUS_SIG5            0x1000
#define SP_STATUS_SIG6            0x2000
#define SP_STATUS_SIG7            0x4000

/* SP_STATUS_REG write bits. */
#define SP_CLR_HALT               0x00000001
#define SP_SET_HALT               0x00000002
#define SP_CLR_BROKE              0x00000004
#define SP_CLR_INTR               0x00000008
#define SP_SET_INTR               0x00000010
#define SP_CLR_SSTEP              0x00000020
#define SP_SET_SSTEP              0x00000040
#define SP_CLR_INTR_BREAK         0x00000080
#define SP_SET_INTR_BREAK         0x00000100
#define SP_CLR_SIG0               0x00000200
#define SP_SET_SIG0               0x00000400
#define SP_CLR_SIG1               0x00000800
#define SP_SET_SIG1               0x00001000
#define SP_CLR_SIG2               0x00002000
#define SP_SET_SIG2               0x00004000
#define SP_CLR_SIG3               0x00008000
#define SP_SET_SIG3               0x00010000
#define SP_CLR_SIG4               0x00020000
#define SP_SET_SIG4               0x00040000
#define SP_CLR_SIG5               0x00080000
#define SP_SET_SIG5               0x00100000
#define SP_CLR_SIG6               0x00200000
#define SP_SET_SIG6               0x00400000
#define SP_CLR_SIG7               0x00800000
#define SP_SET_SIG7               0x01000000

#endif

