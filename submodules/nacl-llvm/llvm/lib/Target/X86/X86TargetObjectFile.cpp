//===-- X86TargetObjectFile.cpp - X86 Object Info -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "X86TargetObjectFile.h"
#include "X86TargetMachine.h"
#include "X86Subtarget.h"  // @LOCALMOD
#include "llvm/CodeGen/MachineModuleInfoImpls.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCSectionELF.h" // @LOCALMOD
#include "llvm/MC/MCSectionMachO.h"
#include "llvm/Target/Mangler.h"
#include "llvm/Support/Dwarf.h"
using namespace llvm;
using namespace dwarf;

const MCExpr *X8664_MachoTargetObjectFile::
getExprForDwarfGlobalReference(const GlobalValue *GV, Mangler *Mang,
                               MachineModuleInfo *MMI, unsigned Encoding,
                               MCStreamer &Streamer) const {

  // On Darwin/X86-64, we can reference dwarf symbols with foo@GOTPCREL+4, which
  // is an indirect pc-relative reference.
  if (Encoding & (DW_EH_PE_indirect | DW_EH_PE_pcrel)) {
    const MCSymbol *Sym = Mang->getSymbol(GV);
    const MCExpr *Res =
      MCSymbolRefExpr::Create(Sym, MCSymbolRefExpr::VK_GOTPCREL, getContext());
    const MCExpr *Four = MCConstantExpr::Create(4, getContext());
    return MCBinaryExpr::CreateAdd(Res, Four, getContext());
  }

  return TargetLoweringObjectFileMachO::
    getExprForDwarfGlobalReference(GV, Mang, MMI, Encoding, Streamer);
}

MCSymbol *X8664_MachoTargetObjectFile::
getCFIPersonalitySymbol(const GlobalValue *GV, Mangler *Mang,
                        MachineModuleInfo *MMI) const {
  return Mang->getSymbol(GV);
}

// @LOCALMOD-START
// NOTE: this was largely lifted from
// lib/Target/ARM/ARMTargetObjectFile.cpp
//
// The default is .ctors/.dtors while the arm backend uses
// .init_array/.fini_array
//
// Without this the linker defined symbols __fini_array_start and
// __fini_array_end do not have useful values. c.f.:
// http://code.google.com/p/nativeclient/issues/detail?id=805
void TargetLoweringObjectFileNaCl::Initialize(MCContext &Ctx,
                                              const TargetMachine &TM) {
  TargetLoweringObjectFileELF::Initialize(Ctx, TM);

  StaticCtorSection =
    getContext().getELFSection(".init_array", ELF::SHT_INIT_ARRAY,
                               ELF::SHF_WRITE |
                               ELF::SHF_ALLOC,
                               SectionKind::getDataRel());
  StaticDtorSection =
    getContext().getELFSection(".fini_array", ELF::SHT_FINI_ARRAY,
                               ELF::SHF_WRITE |
                               ELF::SHF_ALLOC,
                               SectionKind::getDataRel());
}
// @LOCALMOD-END
