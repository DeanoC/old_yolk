         j   i        �������������(h�uk��$���8            u// PR c++/27648

void f()
{
  static_cast<float *__attribute((unused))>(0); // { dg-error "expected" }
}
