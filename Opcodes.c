/* ============================================================================
 *  Opcodes.c: Opcode types, info, and other data.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#include "Opcodes.h"

/* ============================================================================
 *  Use a cross macro to build both tables.
 * ========================================================================= */
const RSPScalarFunction RSPScalarFunctionTable[NUM_RSP_SCALAR_OPCODES] = {
#define X(op) RSP##op,
#include "ScalarOpcodes.md"
#undef X
};

const RSPVectorFunction RSPVectorFunctionTable[NUM_RSP_VECTOR_OPCODES] = {
#define X(op) RSP##op,
#include "VectorOpcodes.md"
#undef X
};

#ifndef NDEBUG
const char *RSPScalarOpcodeMnemonics[NUM_RSP_SCALAR_OPCODES] = {
#define X(op) #op,
#include "ScalarOpcodes.md"
#undef X
};
#endif

#ifndef NDEBUG
const char *RSPVectorOpcodeMnemonics[NUM_RSP_VECTOR_OPCODES] = {
#define X(op) #op,
#include "VectorOpcodes.md"
#undef X
};
#endif

