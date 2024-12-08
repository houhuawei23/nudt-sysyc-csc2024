// Automatically generated file, do not edit!

#pragma once
#include "mir/MIR.hpp"

#define GENERIC_NAMESPACE_BEGIN namespace mir::GENERIC {
#define GENERIC_NAMESPACE_END }

GENERIC_NAMESPACE_BEGIN
enum GENERICInst {
  GENERICInstBegin = ISASpecificBegin,

  Jump,
  Branch,
  Unreachable,
  Load,
  Store,
  Add,
  Sub,
  Mul,
  UDiv,
  URem,
  And,
  Or,
  Xor,
  Shl,
  LShr,
  AShr,
  SDiv,
  SRem,
  SMin,
  SMax,
  Neg,
  Abs,
  FAdd,
  FSub,
  FMul,
  FDiv,
  FNeg,
  FAbs,
  FFma,
  ICmp,
  FCmp,
  SExt,
  ZExt,
  Trunc,
  F2U,
  F2S,
  U2F,
  S2F,
  FCast,
  Copy,
  Select,
  LoadGlobalAddress,
  LoadImm,
  LoadStackObjectAddr,
  CopyFromReg,
  CopyToReg,
  LoadImmToReg,
  LoadRegFromStack,
  StoreRegToStack,
  Return,
  AtomicAdd,
  AtomicSub, /* not implemented yet */

  GENERICInstEnd
};

TargetInstInfo& getGENERICInstInfo();

GENERIC_NAMESPACE_END