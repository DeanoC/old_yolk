#ifndef __STUB_WRITER_H
#define __STUB_WRITER_H

#include "ELFStub.h"

namespace llvm {

void *WriteELFStub(const ELFStub *Stub, size_t *size_out);

}

#endif
