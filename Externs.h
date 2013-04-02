/* ============================================================================
 *  Externs.h: External definitions for the RSP plugin.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __RSP__EXTERNS_H__
#define __RSP__EXTERNS_H__
#ifdef __cplusplus
#include <cstddef>
#else
#include <stddef.h>
#endif

struct BusController;

uint32_t BusReadWord(struct BusController *, uint32_t);
void BusWriteWord(const struct BusController *, uint32_t, uint32_t);

void DMAFromDRAM(struct BusController *, void *, uint32_t, uint32_t);
void DMAToDRAM(struct BusController *, uint32_t, const void *, size_t);

void BusClearRCPInterrupt(struct BusController *, unsigned);
void BusRaiseRCPInterrupt(struct BusController *, unsigned);

#endif

