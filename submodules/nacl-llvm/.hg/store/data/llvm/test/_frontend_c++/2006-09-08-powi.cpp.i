         m   l        ����������}�:Ψ�2���޻Dִ�            u// RUN: %llvmgxx -O3 -S -o - %s

#include <cmath>

double foo(double X, int Y) {
  return std::pow(X, Y);
}
