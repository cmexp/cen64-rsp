/* ============================================================================
 *  Interface.h: Reality Shader Processor (RSP) Interface.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef __RSP__INTERFACE_H__
#define __RSP__INTERFACE_H__
#include "Common.h"
#include "CPU.h"

int SPRegRead(void *, uint32_t, void *);
int SPRegWrite(void *, uint32_t, void *);

#endif

