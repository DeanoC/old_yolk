         c   b        ��������!�**�cB�hMt� �R~E�            u// RUN: %llvmgxx %s -S -o /dev/null

namespace A {
  typedef int B;
}
struct B {
};
using ::A::B;
