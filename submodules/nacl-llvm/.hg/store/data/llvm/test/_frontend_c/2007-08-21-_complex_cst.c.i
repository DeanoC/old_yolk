         Y   X        ��������;?tv8:N��+&�]���O�            u// RUN: %llvmgcc -O2 -S %s -o /dev/null
void f(_Complex float z);
void g() { f(1.0i); }
