         o   n        ���������t�d>�=�at��5n            u; RUN: llc < %s -march=c

define void @func() nounwind {
  entry:
  %0 = fsub double -0.0, undef
  ret void
}
