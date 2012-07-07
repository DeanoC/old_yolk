// Using the high-level representation of an ELF stub, create the actual
// ELF shared object.

#include "StubWriter.h"

//#include <libelf.h>
//#include <gelf.h>
#include <cstdio>

#include "ELFStub.h"

using namespace llvm;

#if 0

#define gelf_sizeof(_type)  (gelf_fsize(elf, (_type), 1, EV_CURRENT))

static size_t add_to_strtab(const char *str, Elf_Scn *strtab, size_t *poffset);

// Helper macro, to fill in elf, shstrtab, and shstrtab_offset automatically.
// This can only be used after the shstrtab (section header string table) has
// been initialized.
#define init_section(_sec, _name, _type, _link, _align, _entsize) \
  init_section_real(elf, shstrtab, &shstrtab_offset, \
                    (_sec), (_name), (_type), (_link), (_align), (_entsize))

static void
init_section_real(Elf *elf, Elf_Scn *shstrtab, size_t *shstrtab_offset,
                  Elf_Scn *sec, const char *name, Elf32_Word type,
                  GElf_Word link, GElf_Xword addralign,
                  GElf_Xword entsize) {
  GElf_Shdr shdr;
  gelf_getshdr(sec, &shdr);
  shdr.sh_name = add_to_strtab(name, shstrtab, shstrtab_offset);
  shdr.sh_type = type;
  shdr.sh_link = link;
  shdr.sh_addralign = addralign;
  shdr.sh_entsize = entsize;
  gelf_update_shdr(sec, &shdr);
}


// This creates a variable "data" in the caller's scope.
static Elf_Data *init_data(Elf *elf, Elf_Scn *sec,
                           Elf_Type type, size_t count,
                           size_t align, loff_t offset) {
  Elf_Data *data = elf_newdata(sec);
  data->d_type = type;
  data->d_size = gelf_fsize(elf, type, count, EV_CURRENT);
  data->d_buf = malloc(data->d_size);
  data->d_align = align;
  data->d_off = offset;
  return data;
}

// Initialize the string table.
static void init_strtab(Elf_Scn *strtab, size_t *poffset) {
  // First byte of a string table should be '\0'.
  Elf_Data *data = elf_newdata(strtab);
  data->d_type = ELF_T_BYTE;
  data->d_buf = (void*)"\0";
  data->d_size = 1;
  data->d_align = 1;
  data->d_off = 0;
  *poffset = 1;
}

// 'str' must stay valid for the duration of WriteELFStub()
static size_t add_to_strtab(const char *str, Elf_Scn *strtab, size_t *poffset) {
  Elf_Data *data = elf_newdata(strtab);
  size_t len = strlen(str);
  data->d_type = ELF_T_BYTE;
  data->d_buf = (void*)str;
  data->d_size = len + 1;
  data->d_align = 1;
  data->d_off = *poffset;
  *poffset += len + 1;
  return data->d_off;
}

namespace llvm {

void *WriteELFStub(const ELFStub *Stub, size_t *size_out) {
  bool is64Bit = (Stub->Machine == EM_X86_64);
  size_t sysalign = is64Bit ? 8 : 4;

  // Use file descriptor -1 since it is an invalid fd. If libelf tries
  // to perform a file operation on it, it will fail/crash.
  elf_version(EV_CURRENT);
  Elf *elf = elf_begin(-1, ELF_C_WRITE_MEM, NULL);

  // Create the ELF header
  // TODO(jvoung): When we switch to always using ELF32 (even on X86-64),
  // then the is64Bit flag can go away.
  // BUG= http://code.google.com/p/nativeclient/issues/detail?id=349
  gelf_newehdr(elf, is64Bit ? ELFCLASS64 : ELFCLASS32);

  // Create the section header string table.
  Elf_Scn *shstrtab = elf_newscn(elf);
  size_t shstrtab_offset;
  init_strtab(shstrtab, &shstrtab_offset);
  init_section(shstrtab, ".shstrtab", SHT_STRTAB, SHN_UNDEF, 1, 1);

  // Update the ELF header
  {
    GElf_Ehdr ehdr;
    gelf_getehdr(elf, &ehdr);
    ehdr.e_ident[EI_DATA] = ELFDATA2LSB;
    ehdr.e_machine = Stub->Machine;
    ehdr.e_type = ET_DYN;
    ehdr.e_shstrndx = elf_ndxscn(shstrtab);
    ehdr.e_version = EV_CURRENT;
    gelf_update_ehdr(elf, &ehdr);
  }

  // Create a dummy text section
  Elf_Scn *dot_text = elf_newscn(elf);
  init_section(dot_text, ".text", SHT_PROGBITS, 0, 32, 0);

  // Create a dummy data section
  Elf_Scn *dot_data = elf_newscn(elf);
  init_section(dot_data, ".data", SHT_PROGBITS, 0, 32, 0);

  // Create the dynamic string table (.dynstr)
  Elf_Scn *dynstr = elf_newscn(elf);
  size_t dynstr_offset;
  init_strtab(dynstr, &dynstr_offset);
  init_section(dynstr, ".dynstr", SHT_STRTAB, SHN_UNDEF, 1, 1);

  //-------------------------------------------------------------------------
  // Create the dynamic symbol table (.dynsym)
  Elf_Scn *dynsym = elf_newscn(elf);
  init_section(dynsym, ".dynsym", SHT_DYNSYM, elf_ndxscn(dynstr),
               sysalign,
               gelf_sizeof(ELF_T_SYM));
  // Populate the dynamic symbol table
  {
    GElf_Sym sym;
    // Reserve Symbols + 1 slots, because the first entry is reserved.
    Elf_Data *data =
        init_data(elf, dynsym, ELF_T_SYM, Stub->Symbols.size() + 1, sysalign,
                  0);

    // The first symbol table entry is reserved (initialize to 0).
    memset(&sym, 0, sizeof(sym));
    gelf_update_sym(data, 0, &sym);

    // Insert the real symbols
    for (unsigned i = 0; i < Stub->Symbols.size(); ++i) {
      const SymbolStub &SS = Stub->Symbols[i];
      sym.st_name = add_to_strtab(SS.Name.c_str(), dynstr, &dynstr_offset);
      sym.st_value = 0; /* Dummy value */
      sym.st_size = SS.Size; /* May be unknown. */
      sym.st_info = SS.Type | (SS.Binding << 4);
      sym.st_other = SS.Visibility;
      /* This is not precisely correct. Hopefully bfd doesn't look at it. */
      sym.st_shndx = (SS.Type == STT_FUNC) ? elf_ndxscn(dot_text) :
                                             elf_ndxscn(dot_data);
      gelf_update_sym(data, i+1, &sym);
    }
  }

  //-------------------------------------------------------------------------
  // Create the GNU version table (.gnu.version)
  Elf_Scn *gnu_version = elf_newscn(elf);
  init_section(gnu_version, ".gnu.version", SHT_GNU_versym, elf_ndxscn(dynsym),
               gelf_sizeof(ELF_T_HALF),
               gelf_sizeof(ELF_T_HALF));
  {
    GElf_Versym versym;
    Elf_Data *data =
        init_data(elf, gnu_version, ELF_T_HALF, Stub->Symbols.size() + 1,
                  gelf_sizeof(ELF_T_HALF), 0);

    // First entry is a dummy.
    versym = 0;
    gelf_update_versym(data, 0, &versym);

    // Fill in the symbol indexes
    for (unsigned i = 0; i < Stub->Symbols.size(); ++i) {
      versym = Stub->Symbols[i].VersionIndex;
      versym |= Stub->Symbols[i].IsDefault ? 0 : 0x8000; /* VERSYM_HIDDEN */
      gelf_update_versym(data, i+1, &versym);
    }
  }

  //-------------------------------------------------------------------------
  // Create the GNU version definition table (.gnu.version_d)
  // We're going to place the Verdaux entries after all the Verdef entries.
  Elf_Scn *gnu_version_d = elf_newscn(elf);
  init_section(gnu_version_d, ".gnu.version_d", SHT_GNU_verdef,
               elf_ndxscn(dynstr), sysalign, 0);

  // We have to put the number of verdef entries in sh_info.
  {
    GElf_Shdr shdr;
    gelf_getshdr(gnu_version_d, &shdr);
    shdr.sh_info = Stub->VerDefs.size() + 1;
    gelf_update_shdr(gnu_version_d, &shdr);
  }

  // Write the version definitions
  size_t sizeof_verdef = gelf_sizeof(ELF_T_VDEF);
  size_t sizeof_verdaux = gelf_sizeof(ELF_T_VDAUX);
  size_t verdaux_offset = sizeof_verdef * (Stub->VerDefs.size()+1);
  {
    // We need to reserve space for both the VDEF and VDAUX entries.
    Elf_Data *data = elf_newdata(gnu_version_d);
    data->d_type = ELF_T_VDEF;
    data->d_size = (sizeof_verdef + sizeof_verdaux) * (Stub->VerDefs.size()+1);
    data->d_buf = malloc(data->d_size);
    data->d_align = sysalign;
    data->d_off = 0;
    GElf_Verdef verdef;
    // The first entry is the version definition of the file itself.
    verdef.vd_version = VER_DEF_CURRENT;
    verdef.vd_flags = VER_FLG_BASE;
    verdef.vd_ndx = 1;
    verdef.vd_cnt = 1;
    verdef.vd_hash = elf_gnu_hash(Stub->SOName.c_str());
    verdef.vd_aux = verdaux_offset;
    verdef.vd_next = sizeof_verdef;
    gelf_update_verdef(data, 0, &verdef);
    // Populate with the real versions
    for (unsigned i = 0; i < Stub->VerDefs.size(); ++i) {
      const VersionDefinition &VD = Stub->VerDefs[i];
      verdef.vd_version = VER_DEF_CURRENT;
      verdef.vd_flags = VD.IsWeak ? VER_FLG_WEAK : 0;
      verdef.vd_ndx = VD.Index;
      verdef.vd_cnt = 1;
      verdef.vd_hash = elf_gnu_hash(VD.Name.c_str());
      verdef.vd_aux = verdaux_offset +
                      (i+1)*sizeof_verdaux - (i+1)*sizeof_verdef;
      verdef.vd_next = sizeof_verdef;
      gelf_update_verdef(data, sizeof_verdef*(1+i), &verdef);
    }
    // Now push the verdaux entries into the same section.
    GElf_Verdaux verdaux;
    // The first entry is the version definition of the file itself.
    // Each verdef only has 1 verdaux entry, which is why vda_next is 0.
    verdaux.vda_name = add_to_strtab(Stub->SOName.c_str(),
                                     dynstr, &dynstr_offset);
    verdaux.vda_next = 0;
    gelf_update_verdaux(data, verdaux_offset, &verdaux);

    // Fill in the symbol version names
    for (unsigned i = 0; i < Stub->VerDefs.size(); ++i) {
      const VersionDefinition &VD = Stub->VerDefs[i];
      verdaux.vda_name = add_to_strtab(VD.Name.c_str(), dynstr, &dynstr_offset);
      verdaux.vda_next = 0;
      gelf_update_verdaux(data, verdaux_offset + sizeof_verdaux*(1+i),
                          &verdaux);
    }
  }

  //-------------------------------------------------------------------------
  // Create .dynamic section
  Elf_Scn *dynamic = elf_newscn(elf);
  init_section(dynamic, ".dynamic", SHT_DYNAMIC, elf_ndxscn(dynstr),
               sysalign,
               gelf_sizeof(ELF_T_DYN));
  Elf_Data *dynamic_data;
  {
    // Reserve space for 2 entries: DT_SONAME, DT_NONE
    // The code below adds more to this section.
    // If it is uncommented, the number of elements needs to change.
    Elf_Data *data = init_data(elf, dynamic, ELF_T_DYN, 2, sysalign, 0);
    dynamic_data = data; // Store for later use

    // Set DT_SONAME
    GElf_Dyn dyn;
    dyn.d_tag = DT_SONAME;
    dyn.d_un.d_val = add_to_strtab(Stub->SOName.c_str(), dynstr, &dynstr_offset);
    gelf_update_dyn(data, 0, &dyn);

    // Set DT_NULL
    dyn.d_tag = DT_NULL;
    dyn.d_un.d_val = 0;
    gelf_update_dyn(dynamic_data, 1, &dyn);

  }

// This portion is not functioning correctly yet.
#if 0
  // Create the program headers
  gelf_newphdr(elf, 0); // LOAD
  gelf_newphdr(elf, 1); // DYNAMIC

  // Compute offsets of everything
  int elf_file_size = elf_update(elf, ELF_C_NULL);

  // Fill in the program headers
  GElf_Phdr phdr;

  // First PT_LOAD entry. Load the whole file.
  // There are normally separate segments for different memory types (code,
  // data, rodata, tls), but we don't really care about laying it out precisely.
  // This is just to give us basic virtual address resolution.
  {
    gelf_getphdr(elf, 0, &phdr);
    phdr.p_type = PT_LOAD;
    phdr.p_offset = 0;
    phdr.p_vaddr = 0;
    phdr.p_paddr = 0;
    phdr.p_filesz = elf_file_size;
    phdr.p_memsz = elf_file_size;
    phdr.p_flags = PF_R;
    phdr.p_align = 0x10000; /* arbitrary */
    gelf_update_phdr(elf, 0, &phdr);
  }

  // PT_DYNAMIC. Get the offset/size of .dynamic from the shdr.
  {
    GElf_Shdr shdr;
    gelf_getshdr(dynamic, &shdr);
    gelf_getphdr(elf, 1, &phdr);
    phdr.p_type = PT_LOAD;
    phdr.p_offset = shdr.sh_offset;
    phdr.p_vaddr = elf_file_size; // After the LOAD
    phdr.p_paddr = elf_file_size;
    phdr.p_filesz = shdr.sh_size;
    phdr.p_memsz = shdr.sh_size;
    phdr.p_flags = PF_RW;
    phdr.p_align = sysalign;
    gelf_update_phdr(elf, 1, &phdr);
  }

  // Fill in the .dynamic entries, now that we have
  // the addresses.
  {
    GElf_Dyn dyn;
    GElf_Shdr shdr;

    // DT_STRTAB points to .dynstr
    gelf_getshdr(dynstr, &shdr);
    dyn.d_tag = DT_STRTAB;
    dyn.d_un.d_ptr = shdr.sh_offset;
    gelf_update_dyn(dynamic_data, 1, &dyn);

    // DT_SYMTAB to .dynsym
    gelf_getshdr(dynsym, &shdr);
    dyn.d_tag = DT_SYMTAB;
    dyn.d_un.d_ptr = shdr.sh_offset;
    gelf_update_dyn(dynamic_data, 2, &dyn);

    // DT_STRSZ, size in bytes of .dynstr
    gelf_getshdr(dynstr, &shdr);
    dyn.d_tag = DT_STRSZ;
    dyn.d_un.d_val = shdr.sh_size;
    gelf_update_dyn(dynamic_data, 3, &dyn);

    // DT_SYMENT, size in bytes of a symbol table entry
    gelf_getshdr(dynsym, &shdr);
    dyn.d_tag = DT_SYMENT;
    dyn.d_un.d_val = shdr.sh_entsize;
    gelf_update_dyn(dynamic_data, 4, &dyn);

    // DT_VERDEF, points to GNU version defs
    gelf_getshdr(gnu_version_d, &shdr);
    dyn.d_tag = DT_VERDEF;
    dyn.d_un.d_ptr = shdr.sh_offset;
    gelf_update_dyn(dynamic_data, 5, &dyn);

    // DT_VERDEFNUM, number of GNU version defs
    gelf_getshdr(gnu_version_d, &shdr);
    dyn.d_tag = DT_VERDEFNUM;
    dyn.d_un.d_val = shdr.sh_info; // Special meaning
    gelf_update_dyn(dynamic_data, 6, &dyn);

    // DT_VERSYM, GNU symbol versions
    gelf_getshdr(gnu_version, &shdr);
    dyn.d_tag = DT_VERSYM;
    dyn.d_un.d_ptr = shdr.sh_offset;
    gelf_update_dyn(dynamic_data, 7, &dyn);

    // DT_NULL, terminates .dynamic
    dyn.d_tag = DT_NULL;
    dyn.d_un.d_val = 0;
    gelf_update_dyn(dynamic_data, 8, &dyn);
    elf_flagdata(dynamic_data, ELF_C_SET, ELF_F_DIRTY);
  }
  elf_flagelf(elf, ELF_C_SET, ELF_F_LAYOUT);
#endif

  // Write the ELF file into memory
  int elf_file_size = elf_update(elf, ELF_C_WRITE_MEM);
  void *mem = elf_rawfile(elf, NULL);
  elf_end(elf);
  *size_out = elf_file_size;
  return mem;
}

} // namespace

#endif