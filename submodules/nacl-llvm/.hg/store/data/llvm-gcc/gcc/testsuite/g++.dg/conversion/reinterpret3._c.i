         U   T        �����������Wg��˂�0�%�z��Q.            ustruct S {};

S s;

void f() {
  reinterpret_cast<const S>(s); // { dg-error "" }
}
