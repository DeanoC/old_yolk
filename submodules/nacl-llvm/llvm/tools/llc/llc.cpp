//===-- llc.cpp - Implement the LLVM Native Code Generator ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is the llc code generator driver. It provides a convenient
// command-line interface for generating native assembly-language code
// or C code, given LLVM bitcode.
//
//===----------------------------------------------------------------------===//

#include "llvm/LLVMContext.h"
#include "llvm/Module.h"
#include "llvm/PassManager.h"
#include "llvm/Pass.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/DataStream.h"
#include "llvm/Support/IRReader.h"
#include "llvm/CodeGen/IntrinsicLowering.h" // @LOCALMOD
#include "llvm/CodeGen/LinkAllAsmWriterComponents.h"
#include "llvm/CodeGen/LinkAllCodegenComponents.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/ManagedStatic.h"
#if !defined(__native_client__)
#include "llvm/Support/PluginLoader.h"
#endif
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/ToolOutputFile.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetMachine.h"
#include <memory>

// @LOCALMOD-BEGIN
#include "StubMaker.h"
#include "StubWriter.h"
#include "TextStubWriter.h"
// @LOCALMOD-END

using namespace llvm;

// @LOCALMOD-BEGIN
// NOTE: this tool can be build as a "sandboxed" translator.
//       There are two ways to build the translator
//       SRPC-style:  no file operations are allowed
//                    see nacl_file.cc for support code
//       non-SRPC-style: some basic file operations are allowed
//                       This can be useful for debugging but will
//                       not be deployed.
#if defined(__native_client__) && defined(NACL_SRPC)
MemoryBuffer* NaClGetMemoryBufferForFile(const char* filename);
void NaClOutputStringToFile(const char* filename, const std::string& data);
// The following two functions communicate metadata to the SRPC wrapper for LLC.
void NaClRecordObjectInformation(bool is_shared, const std::string& soname);
void NaClRecordSharedLibraryDependency(const std::string& library_name);
DataStreamer* NaClBitcodeStreamer;
#endif
// @LOCALMOD-END




// General options for llc.  Other pass-specific options are specified
// within the corresponding llc passes, and target-specific options
// and back-end code generation options are specified with the target machine.
//
static cl::opt<std::string>
InputFilename(cl::Positional, cl::desc("<input bitcode>"), cl::init("-"));

static cl::opt<std::string>
OutputFilename("o", cl::desc("Output filename"), cl::value_desc("filename"));

// @LOCALMOD-BEGIN
static cl::opt<std::string>
MetadataOutputFilename("metadata-out", cl::desc("Metadata output filename"),
                       cl::value_desc("filename"));
static cl::opt<std::string>
MetadataTextFilename("metadata-text", cl::desc("Metadata as text, out filename"),
                     cl::value_desc("filename"));
// @LOCALMOD-END

// Determine optimization level.
static cl::opt<char>
OptLevel("O",
         cl::desc("Optimization level. [-O0, -O1, -O2, or -O3] "
                  "(default = '-O2')"),
         cl::Prefix,
         cl::ZeroOrMore,
         cl::init(' '));

// @LOCALMOD-BEGIN

static cl::opt<std::string>
TargetTriple("mtriple", cl::desc("Override target triple for module"));

static cl::opt<std::string>
MArch("march", cl::desc("Architecture to generate code for (see --version)"));

static cl::opt<std::string>
MCPU("mcpu",
  cl::desc("Target a specific cpu type (-mcpu=help for details)"),
  cl::value_desc("cpu-name"),
  cl::init(""));

static cl::list<std::string>
MAttrs("mattr",
  cl::CommaSeparated,
  cl::desc("Target specific attributes (-mattr=help for details)"),
  cl::value_desc("a1,+a2,-a3,..."));

static cl::opt<Reloc::Model>
RelocModel("relocation-model",
             cl::desc("Choose relocation model"),
             cl::init(Reloc::Default),
             cl::values(
            clEnumValN(Reloc::Default, "default",
                       "Target default relocation model"),
            clEnumValN(Reloc::Static, "static",
                       "Non-relocatable code"),
            clEnumValN(Reloc::PIC_, "pic",
                       "Fully relocatable, position independent code"),
            clEnumValN(Reloc::DynamicNoPIC, "dynamic-no-pic",
                       "Relocatable external references, non-relocatable code"),
            clEnumValEnd));

static cl::opt<llvm::CodeModel::Model>
CMModel("code-model",
        cl::desc("Choose code model"),
        cl::init(CodeModel::Default),
        cl::values(clEnumValN(CodeModel::Default, "default",
                              "Target default code model"),
                   clEnumValN(CodeModel::Small, "small",
                              "Small code model"),
                   clEnumValN(CodeModel::Kernel, "kernel",
                              "Kernel code model"),
                   clEnumValN(CodeModel::Medium, "medium",
                              "Medium code model"),
                   clEnumValN(CodeModel::Large, "large",
                              "Large code model"),
                   clEnumValEnd));

static cl::opt<bool>
RelaxAll("mc-relax-all",
  cl::desc("When used with filetype=obj, "
           "relax all fixups in the emitted object file"));

cl::opt<TargetMachine::CodeGenFileType>
FileType("filetype", cl::init(TargetMachine::CGFT_AssemblyFile),
  cl::desc("Choose a file type (not all types are supported by all targets):"),
  cl::values(
       clEnumValN(TargetMachine::CGFT_AssemblyFile, "asm",
                  "Emit an assembly ('.s') file"),
       clEnumValN(TargetMachine::CGFT_ObjectFile, "obj",
                  "Emit a native object ('.o') file [experimental]"),
       clEnumValN(TargetMachine::CGFT_Null, "null",
                  "Emit nothing, for performance testing"),
       clEnumValEnd));

cl::opt<bool> NoVerify("disable-verify", cl::Hidden,
                       cl::desc("Do not verify input module"));

cl::opt<bool> DisableDotLoc("disable-dot-loc", cl::Hidden,
                            cl::desc("Do not use .loc entries"));

cl::opt<bool> DisableCFI("disable-cfi", cl::Hidden,
                         cl::desc("Do not use .cfi_* directives"));

cl::opt<bool> EnableDwarfDirectory("enable-dwarf-directory", cl::Hidden,
    cl::desc("Use .file directives with an explicit directory."));

static cl::opt<bool>
DisableRedZone("disable-red-zone",
  cl::desc("Do not emit code that uses the red zone."),
  cl::init(false));

static cl::opt<bool>
EnableFPMAD("enable-fp-mad",
  cl::desc("Enable less precise MAD instructions to be generated"),
  cl::init(false));

static cl::opt<bool>
PrintCode("print-machineinstrs",
  cl::desc("Print generated machine code"),
  cl::init(false));

static cl::opt<bool>
DisableFPElim("disable-fp-elim",
  cl::desc("Disable frame pointer elimination optimization"),
  cl::init(false));

static cl::opt<bool>
DisableFPElimNonLeaf("disable-non-leaf-fp-elim",
  cl::desc("Disable frame pointer elimination optimization for non-leaf funcs"),
  cl::init(false));

static cl::opt<bool>
DisableExcessPrecision("disable-excess-fp-precision",
  cl::desc("Disable optimizations that may increase FP precision"),
  cl::init(false));

static cl::opt<bool>
EnableUnsafeFPMath("enable-unsafe-fp-math",
  cl::desc("Enable optimizations that may decrease FP precision"),
  cl::init(false));

static cl::opt<bool>
EnableNoInfsFPMath("enable-no-infs-fp-math",
  cl::desc("Enable FP math optimizations that assume no +-Infs"),
  cl::init(false));

static cl::opt<bool>
EnableNoNaNsFPMath("enable-no-nans-fp-math",
  cl::desc("Enable FP math optimizations that assume no NaNs"),
  cl::init(false));

static cl::opt<bool>
EnableHonorSignDependentRoundingFPMath("enable-sign-dependent-rounding-fp-math",
  cl::Hidden,
  cl::desc("Force codegen to assume rounding mode can change dynamically"),
  cl::init(false));

static cl::opt<bool>
GenerateSoftFloatCalls("soft-float",
  cl::desc("Generate software floating point library calls"),
  cl::init(false));

static cl::opt<llvm::FloatABI::ABIType>
FloatABIForCalls("float-abi",
  cl::desc("Choose float ABI type"),
  cl::init(FloatABI::Default),
  cl::values(
    clEnumValN(FloatABI::Default, "default",
               "Target default float ABI type"),
    clEnumValN(FloatABI::Soft, "soft",
               "Soft float ABI (implied by -soft-float)"),
    clEnumValN(FloatABI::Hard, "hard",
               "Hard float ABI (uses FP registers)"),
    clEnumValEnd));

static cl::opt<bool>
DontPlaceZerosInBSS("nozero-initialized-in-bss",
  cl::desc("Don't place zero-initialized symbols into bss section"),
  cl::init(false));

static cl::opt<bool>
EnableGuaranteedTailCallOpt("tailcallopt",
  cl::desc("Turn fastcc calls into tail calls by (potentially) changing ABI."),
  cl::init(false));

static cl::opt<bool>
DisableTailCalls("disable-tail-calls",
  cl::desc("Never emit tail calls"),
  cl::init(false));

static cl::opt<unsigned>
OverrideStackAlignment("stack-alignment",
  cl::desc("Override default stack alignment"),
  cl::init(0));

static cl::opt<bool>
EnableRealignStack("realign-stack",
  cl::desc("Realign stack if needed"),
  cl::init(true));

static cl::opt<bool>
DisableSwitchTables(cl::Hidden, "disable-jump-tables",
  cl::desc("Do not generate jump tables."),
  cl::init(false));

static cl::opt<std::string>
TrapFuncName("trap-func", cl::Hidden,
  cl::desc("Emit a call to trap function rather than a trap instruction"),
  cl::init(""));

static cl::opt<bool>
EnablePIE("enable-pie",
  cl::desc("Assume the creation of a position independent executable."),
  cl::init(false));

static cl::opt<bool>
SegmentedStacks("segmented-stacks",
  cl::desc("Use segmented stacks if possible."),
  cl::init(false));


// Using bitcode streaming has a couple of ramifications. Primarily it means
// that the module in the file will be compiled one function at a time rather
// than the whole module. This allows earlier functions to be compiled before
// later functions are read from the bitcode but of course means no whole-module
// optimizations. For now, streaming is only supported for files and stdin.
static cl::opt<bool>
LazyBitcode("streaming-bitcode",
  cl::desc("Use lazy bitcode streaming for file inputs"),
  cl::init(false));

// GetFileNameRoot - Helper function to get the basename of a filename.
static inline std::string
GetFileNameRoot(const std::string &InputFilename) {
  std::string IFN = InputFilename;
  std::string outputFilename;
  int Len = IFN.length();
  if ((Len > 2) &&
      IFN[Len-3] == '.' &&
      ((IFN[Len-2] == 'b' && IFN[Len-1] == 'c') ||
       (IFN[Len-2] == 'l' && IFN[Len-1] == 'l'))) {
    outputFilename = std::string(IFN.begin(), IFN.end()-3); // s/.bc/.s/
  } else {
    outputFilename = IFN;
  }
  return outputFilename;
}

static tool_output_file *GetOutputStream(const char *TargetName,
                                         Triple::OSType OS,
                                         const char *ProgName) {
  // If we don't yet have an output filename, make one.
  if (OutputFilename.empty()) {
    if (InputFilename == "-")
      OutputFilename = "-";
    else {
      OutputFilename = GetFileNameRoot(InputFilename);

      switch (FileType) {
      case TargetMachine::CGFT_AssemblyFile:
        if (TargetName[0] == 'c') {
          if (TargetName[1] == 0)
            OutputFilename += ".cbe.c";
          else if (TargetName[1] == 'p' && TargetName[2] == 'p')
            OutputFilename += ".cpp";
          else
            OutputFilename += ".s";
        } else
          OutputFilename += ".s";
        break;
      case TargetMachine::CGFT_ObjectFile:
        if (OS == Triple::Win32)
          OutputFilename += ".obj";
        else
          OutputFilename += ".o";
        break;
      case TargetMachine::CGFT_Null:
        OutputFilename += ".null";
        break;
      }
    }
  }

  // Decide if we need "binary" output.
  bool Binary = false;
  switch (FileType) {
  case TargetMachine::CGFT_AssemblyFile:
    break;
  case TargetMachine::CGFT_ObjectFile:
  case TargetMachine::CGFT_Null:
    Binary = true;
    break;
  }

  // Open the file.
  std::string error;
  unsigned OpenFlags = 0;
  if (Binary) OpenFlags |= raw_fd_ostream::F_Binary;
  tool_output_file *FDOut = new tool_output_file(OutputFilename.c_str(), error,
                                                 OpenFlags);
  if (!error.empty()) {
    errs() << error << '\n';
    delete FDOut;
    return 0;
  }

  return FDOut;
}

// @LOCALMOD-BEGIN
#if defined(__native_client__) && defined(NACL_SRPC)
void RecordMetadataForSrpc(const Module &mod) {
  bool is_shared = (mod.getOutputFormat() == Module::SharedOutputFormat);
  std::string soname = mod.getSOName();
  NaClRecordObjectInformation(is_shared, soname);
  for (Module::lib_iterator L = mod.lib_begin(),
                            E = mod.lib_end();
       L != E; ++L) {
    NaClRecordSharedLibraryDependency(*L);
  }
}
#endif  // defined(__native_client__) && defined(NACL_SRPC)
// @LOCALMOD-END


// @LOCALMOD-BEGIN

static const int NACL_FILE_MAGIC = 0xfadefade;

// Keep this structure a multiple of 8-bytes
// This must match the same structure in nacl_file.c (in binutils/bfd)
typedef struct {
  uint32_t magic;   // NACL_FILE_MAGIC
  uint32_t size;    // Size of this file (not including padding)
  char     filename[64]; // Padded with nils
} FileEntry;

// Write the ELF Stubs to the metadata file
// Returns 0 on success, non-zero on error.
int WriteMetadataFile(const Module &M, const Triple &TheTriple) {
/*  // Build the ELF stubs (in high level format)
  SmallVector<ELFStub*, 8> StubList;
  MakeAllStubs(M, TheTriple, &StubList);

  // For each stub, write the ELF object to the metadata file
  std::string s;
  for (unsigned i = 0; i < StubList.size(); i++) {
    FileEntry FE;
    StringRef SOName = StubList[i]->SOName;
    size_t size;
    void *mem = WriteELFStub(StubList[i], &size);
    FE.magic = NACL_FILE_MAGIC;
    FE.size = size;
    assert(SOName.size() < sizeof(FE.filename));
    memset(FE.filename, 0, sizeof(FE.filename));
    strcpy(FE.filename, SOName.str().c_str());
    s.append((const char*)&FE, sizeof(FE));
    s.append((const char*)mem, size);
    free(mem);
    // Pad up to 8-byte alignment.
    size_t padding = 8*((size + 7)/8) - size;
    s.append(padding, '\0');
  }
  FreeStubList(&StubList);

#if defined(__native_client__) && defined(NACL_SRPC)
  NaClOutputStringToFile(MetadataOutputFilename.c_str(), s);
#else
  std::string error;
  OwningPtr<tool_output_file> MOut(
      new tool_output_file(MetadataOutputFilename.c_str(), error,
                           raw_fd_ostream::F_Binary));
  if (!error.empty()) {
    errs() << error << '\n';
    return 1;
  }
  MOut->os().write(s.data(), s.size());
  MOut->keep();
#endif*/
  return 0;
}


// Write the ELF Stubs to the metadata file, in text format
// Returns 0 on success, non-zero on error.
int WriteTextMetadataFile(const Module &M, const Triple &TheTriple) {
  // Build the ELF stubs (in high level format)
  SmallVector<ELFStub*, 8> StubList;
  // NOTE: The triple is unnecessary for the text version.
  MakeAllStubs(M, TheTriple, &StubList);
  // For each stub, write the ELF object to the metadata file.
  std::string s;
  for (unsigned i = 0; i < StubList.size(); i++) {
    WriteTextELFStub(StubList[i], &s);
  }
  FreeStubList(&StubList);

#if defined(__native_client__) && defined(NACL_SRPC)
  NaClOutputStringToFile(MetadataTextFilename.c_str(), s);
#else
  std::string error;
  OwningPtr<tool_output_file> MOut(
      new tool_output_file(MetadataTextFilename.c_str(), error,
                           raw_fd_ostream::F_Binary));
  if (!error.empty()) {
    errs() << error << '\n';
    return 1;
  }
  MOut->os().write(s.data(), s.size());
  MOut->keep();
#endif
  return 0;
}

// @LOCALMOD-END

// main - Entry point for the llc compiler.
//
int llc_main(int argc, char **argv) {
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);

  // Enable debug stream buffering.
  EnableDebugBuffering = true;

  LLVMContext &Context = getGlobalContext();
  llvm_shutdown_obj Y;  // Call llvm_shutdown() on exit.

  // Initialize targets first, so that --version shows registered targets.
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmPrinters();
  InitializeAllAsmParsers();

  // Register the target printer for --version.
  cl::AddExtraVersionPrinter(TargetRegistry::printRegisteredTargetsForVersion);

  cl::ParseCommandLineOptions(argc, argv, "llvm system compiler\n");

  // Load the module to be compiled...
  SMDiagnostic Err;
  std::auto_ptr<Module> M;

  // @LOCALMOD-BEGIN
#if defined(__native_client__) && defined(NACL_SRPC)
  if (LazyBitcode) {
    std::string StrError;
    M.reset(getStreamedBitcodeModule(std::string("<SRPC stream>"),
                                     NaClBitcodeStreamer, Context, &StrError));
    if (!StrError.empty()) {
      Err = SMDiagnostic(InputFilename, SourceMgr::DK_Error, StrError);
    }
  } else {
    // In the NACL_SRPC case, fake a memory mapped file
    // TODO(jvoung): revert changes in MemoryBuffer.cpp, no longer needed
    M.reset(ParseIR(NaClGetMemoryBufferForFile(InputFilename.c_str()),
                    Err,
                    Context));
    M->setModuleIdentifier(InputFilename);
  }
#else
  if (LazyBitcode) {
    std::string StrError;
    DataStreamer *streamer = getDataFileStreamer(InputFilename, &StrError);
    if (streamer) {
      M.reset(getStreamedBitcodeModule(InputFilename, streamer, Context,
                                       &StrError));
    }
    if (!StrError.empty()) {
      Err = SMDiagnostic(InputFilename, SourceMgr::DK_Error, StrError);
    }
  } else {
    M.reset(ParseIRFile(InputFilename, Err, Context));
  }
#endif
  // @LOCALMOD-END

  if (M.get() == 0) {
    Err.print(argv[0], errs());
    return 1;
  }
  Module &mod = *M.get();

  // @LOCALMOD-BEGIN
#if defined(__native_client__) && defined(NACL_SRPC)
  RecordMetadataForSrpc(mod);

  // To determine if we should compile PIC or not, we needed to load at
  // least the metadata. Since we've already constructed the commandline,
  // we have to hack this in after commandline processing.
  if (mod.getOutputFormat() == Module::SharedOutputFormat) {
    RelocModel = Reloc::PIC_;
  }
  // Also set PIC_ for dynamic executables:
  // BUG= http://code.google.com/p/nativeclient/issues/detail?id=2351
  if (mod.lib_size() > 0) {
    RelocModel = Reloc::PIC_;
  }
#endif  // defined(__native_client__) && defined(NACL_SRPC)
  // @LOCALMOD-END

  // If we are supposed to override the target triple, do so now.
  if (!TargetTriple.empty())
    mod.setTargetTriple(Triple::normalize(TargetTriple));

  // Figure out the target triple.
  Triple TheTriple(mod.getTargetTriple());
  if (TheTriple.getTriple().empty())
    TheTriple.setTriple(sys::getDefaultTargetTriple());

  // Get the target specific parser.
  std::string Error;
  const Target *TheTarget = TargetRegistry::lookupTarget(MArch, TheTriple,
                                                         Error);
  if (!TheTarget) {
    errs() << argv[0] << ": " << Error;
    return 1;
  }

  // Package up features to be passed to target/subtarget
  std::string FeaturesStr;
  if (MAttrs.size()) {
    SubtargetFeatures Features;
    // @LOCALMOD-BEGIN
    // Use the same default attribute settings as libLTO.
    // TODO(pdox): Figure out why this isn't done for upstream llc.
    Features.getDefaultSubtargetFeatures(TheTriple);
    // @LOCALMOD-END
    for (unsigned i = 0; i != MAttrs.size(); ++i)
      Features.AddFeature(MAttrs[i]);
    FeaturesStr = Features.getString();
  }

  CodeGenOpt::Level OLvl = CodeGenOpt::Default;
  switch (OptLevel) {
  default:
    errs() << argv[0] << ": invalid optimization level.\n";
    return 1;
  case ' ': break;
  case '0': OLvl = CodeGenOpt::None; break;
  case '1': OLvl = CodeGenOpt::Less; break;
  case '2': OLvl = CodeGenOpt::Default; break;
  case '3': OLvl = CodeGenOpt::Aggressive; break;
  }

  TargetOptions Options;
  Options.LessPreciseFPMADOption = EnableFPMAD;
  Options.PrintMachineCode = PrintCode;
  Options.NoFramePointerElim = DisableFPElim;
  Options.NoFramePointerElimNonLeaf = DisableFPElimNonLeaf;
  Options.NoExcessFPPrecision = DisableExcessPrecision;
  Options.UnsafeFPMath = EnableUnsafeFPMath;
  Options.NoInfsFPMath = EnableNoInfsFPMath;
  Options.NoNaNsFPMath = EnableNoNaNsFPMath;
  Options.HonorSignDependentRoundingFPMathOption =
      EnableHonorSignDependentRoundingFPMath;
  Options.UseSoftFloat = GenerateSoftFloatCalls;
  if (FloatABIForCalls != FloatABI::Default)
    Options.FloatABIType = FloatABIForCalls;
  Options.NoZerosInBSS = DontPlaceZerosInBSS;
  Options.GuaranteedTailCallOpt = EnableGuaranteedTailCallOpt;
  Options.DisableTailCalls = DisableTailCalls;
  Options.StackAlignmentOverride = OverrideStackAlignment;
  Options.RealignStack = EnableRealignStack;
  Options.DisableJumpTables = DisableSwitchTables;
  Options.TrapFuncName = TrapFuncName;
  Options.PositionIndependentExecutable = EnablePIE;
  Options.EnableSegmentedStacks = SegmentedStacks;

  std::auto_ptr<TargetMachine>
    target(TheTarget->createTargetMachine(TheTriple.getTriple(),
                                          MCPU, FeaturesStr, Options,
                                          RelocModel, CMModel, OLvl));
  assert(target.get() && "Could not allocate target machine!");
  TargetMachine &Target = *target.get();

  if (DisableDotLoc)
    Target.setMCUseLoc(false);

  if (DisableCFI)
    Target.setMCUseCFI(false);

  if (EnableDwarfDirectory)
    Target.setMCUseDwarfDirectory(true);

  if (GenerateSoftFloatCalls)
    FloatABIForCalls = FloatABI::Soft;

  // Disable .loc support for older OS X versions.
  if (TheTriple.isMacOSX() &&
      TheTriple.isMacOSXVersionLT(10, 6))
    Target.setMCUseLoc(false);

#if !defined(NACL_SRPC)
  // Figure out where we are going to send the output...
  OwningPtr<tool_output_file> Out
    (GetOutputStream(TheTarget->getName(), TheTriple.getOS(), argv[0]));
  if (!Out) return 1;
#endif
  
  // Build up all of the passes that we want to do to the module.
  // @LOCALMOD-BEGIN
  OwningPtr<PassManagerBase> PM;
  if (LazyBitcode)
    PM.reset(new FunctionPassManager(&mod));
  else
    PM.reset(new PassManager());
  // @LOCALMOD-END

  // Add the target data from the target machine, if it exists, or the module.
  if (const TargetData *TD = Target.getTargetData())
    PM->add(new TargetData(*TD));
  else
    PM->add(new TargetData(&mod));

  // Override default to generate verbose assembly.
  Target.setAsmVerbosityDefault(true);

  if (RelaxAll) {
    if (FileType != TargetMachine::CGFT_ObjectFile)
      errs() << argv[0]
             << ": warning: ignoring -mc-relax-all because filetype != obj";
    else
      Target.setMCRelaxAll(true);
  }


  
#if defined __native_client__ && defined(NACL_SRPC)
  {
    std::string s;
    raw_string_ostream ROS(s);
    formatted_raw_ostream FOS(ROS);
    // Ask the target to add backend passes as necessary.
    if (Target.addPassesToEmitFile(*PM, FOS, FileType, NoVerify)) {
      errs() << argv[0] << ": target does not support generation of this"
             << " file type!\n";
      return 1;
    }

    if (LazyBitcode) {
      FunctionPassManager* P = static_cast<FunctionPassManager*>(PM.get());
      P->doInitialization();
      for (Module::iterator I = mod.begin(), E = mod.end(); I != E; ++I) {
        P->run(*I);
      }
      P->doFinalization();
    } else {
      static_cast<PassManager*>(PM.get())->run(mod);
    }
    FOS.flush();
    ROS.flush();
    NaClOutputStringToFile(OutputFilename.c_str(), ROS.str());
  }
#else
      
  {
    formatted_raw_ostream FOS(Out->os());

    // Ask the target to add backend passes as necessary.
    if (Target.addPassesToEmitFile(*PM, FOS, FileType, NoVerify)) {
      errs() << argv[0] << ": target does not support generation of this"
             << " file type!\n";
      return 1;
    }

    // Before executing passes, print the final values of the LLVM options.
    cl::PrintOptionValues();

    if (LazyBitcode) {
      FunctionPassManager *P = static_cast<FunctionPassManager*>(PM.get());
      P->doInitialization();
      for (Module::iterator I = mod.begin(), E = mod.end(); I != E; ++I) {
        P->run(*I);
      }
      P->doFinalization();
    } else {
      static_cast<PassManager*>(PM.get())->run(mod);
    }
  }

  // Declare success.
  Out->keep();
#endif

  // @LOCALMOD-BEGIN
  // Write out the metadata.
  //
  // We need to ensure that intrinsic prototypes are available, in case
  // we have a NeededRecord for one of them.
  // They may have been eliminated by the StripDeadPrototypes pass,
  // or some other pass that is unaware of NeededRecords / IntrinsicLowering.
  IntrinsicLowering IL(*target->getTargetData());
  IL.AddPrototypes(*M);

  if (!MetadataOutputFilename.empty()) {
    int err = WriteMetadataFile(*M.get(), TheTriple);
    if (err != 0)
      return err;
  }

  if (!MetadataTextFilename.empty()) {
    int err = WriteTextMetadataFile(*M.get(), TheTriple);
    if (err != 0)
      return err;
  }
  // @LOCALMOD-END

  return 0;
}

#if !defined(NACL_SRPC)
int
main (int argc, char **argv) {
  return llc_main(argc, argv);
}
#else
// main() is in nacl_file.cpp.
#endif
