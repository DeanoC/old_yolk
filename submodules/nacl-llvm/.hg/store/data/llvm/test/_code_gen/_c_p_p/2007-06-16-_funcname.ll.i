         s   r        ��������}QM�t�HIft�< _R�_            u; RUN: llc < %s -march=cpp -cppfname=WAKKA | not grep makeLLVMModule
; PR1515

define void @foo() {
  ret void
}

