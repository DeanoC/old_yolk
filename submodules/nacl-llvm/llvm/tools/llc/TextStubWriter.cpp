// Using the high-level representation of an ELF stub, create a text version
// of the ELF stub object.

#include "TextStubWriter.h"

#include <sstream>

#include "ELFStub.h"
#include "llvm/Support/ELF.h"

using namespace llvm;

namespace {

std::string LibShortname(const std::string &fullname) {
  std::string result = fullname;
  if (result.find("lib") != std::string::npos) {
    result = result.substr(3);
  }
  size_t so_pos = result.find(".so");
  if (so_pos != std::string::npos) {
    result = result.substr(0, so_pos);
  }
  return result;
}

}  // namespace

namespace llvm {


// Write out the dynamic symbol table information.  The format must be kept
// in sync with the changes in NaCl's version of gold (see gold/metadata.cc).
void WriteTextELFStub(const ELFStub *Stub, std::string *output) {
  std::stringstream ss;

  // TODO(jvoung): Figure out if we need to avoid stubbing out ld.so for
  // pnacl.  When that is done with gold in the non-nacl case, we ended up
  // jumping to address 0 on startup. Usually ld.so is not in the list of
  // DT_NEEDED anyway, and having a meta-wrapper makes it show up in DT_NEEDED.
  // For now, skip.
  std::string ld_so_prefix("ld-nacl-");
  if (Stub->SOName.compare(0, ld_so_prefix.length(), ld_so_prefix) == 0) {
    return;
  }

  ss << "####\n";
  ss << "@obj " << LibShortname(Stub->SOName) << " " << Stub->SOName << "\n";

  // st_value is usually a relative address for .so, and .exe files.
  // So, make some up.
  ELF::Elf32_Addr fake_relative_addr = 0;
  for (size_t i = 0; i < Stub->Symbols.size(); ++i) {
    const SymbolStub &sym = Stub->Symbols[i];
    ss << "@sym "
       << sym.Name << " " // Representative for st_name.
       << fake_relative_addr << " " // st_value.
       << sym.Size << " " // st_size (may be unknown, or 0)
       << (sym.Type | (sym.Binding << 4)) << " " // st_info
       << (int)(sym.Visibility) << " " // st_other
       << ((sym.Type == ELF::STT_FUNC) ? "5" : "6") // Dummy st_shndx.
       << "\n";
    fake_relative_addr += (sym.Size == 0 ? 4 : sym.Size);
  }
  ss << "\n";

  // TODO(jvoung): handle versions.

  output->append(ss.str());
}

} // namespace llvm
