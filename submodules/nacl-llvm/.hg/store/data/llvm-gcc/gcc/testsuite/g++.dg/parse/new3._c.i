         f   e        ����������Y.Yȍ#��.!d�Z��4            u// PR c++/28593
// { dg-do compile }

void *p = new (::X;  // { dg-error "declared|type-specifier" }
