/* ============================================================================
 *  VectorOpcodes.md: Opcode types, info, and other data.
 *
 *  RSPSIM: Reality Signal Processor SIMulator.
 *  Copyright (C) 2013, Tyler J. Stachecki.
 *  All rights reserved.
 *
 *  This file is subject to the terms and conditions defined in
 *  file 'LICENSE', which is part of this source code package.
 * ========================================================================= */
#ifndef RSP_VECTOR_OPCODE_TABLE
#define RSP_VECTOR_OPCODE_TABLE X(VINV) \
  X(VABS) X(VADD) X(VADDC) X(VAND) X(VCH) X(VCL) X(VCR) X(VEQ) X(VGE) \
  X(VLT) X(VMACF) X(VMACQ) X(VMACU) X(VMADH) X(VMADL) X(VMADM) X(VMADN) \
  X(VMOV) X(VMRG) X(VMUDH) X(VMUDL) X(VMUDM) X(VMUDN) X(VMULF) X(VMULQ) \
  X(VMULU) X(VNAND) X(VNE) X(VNOP) X(VNOR) X(VNXOR) X(VOR) X(VRCP) \
  X(VRCPH) X(VRCPL) X(VRNDN) X(VRNDP) X(VRSQ) X(VRSQH) X(VRSQL) X(VSAR) \
  X(VSUB) X(VSUBC) X(VXOR) 
#endif

RSP_VECTOR_OPCODE_TABLE

