         p   o        ��������q0s���'�����龄��Okg            u// RUN: %llvmgcc -S %s -o /dev/null
void bork(void **data) {
  (*(unsigned short *) (&(data[37])[927]) = 0);
}
