         W   V        ���������v�a�?�S��U����s�            u// { dg-do assemble  }
// prms-id: 9506

char * volatile p;
void foo() {
  --p = 0;
}
