         c   b        ��������%��8Ste�\,(����N�            u// PR c++/20153

template <typename T>
void
foo()
{
  union { struct { }; }; // { dg-error "" }
}
