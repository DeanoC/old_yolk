         `   _        ��������g�l���I���Ki)9U6            u// PR c++/29886

struct A {
  static int x[1];
};

void foo(int i)
{
  if (int(A::x[i])) {}
}

