         f   e        ���������U�C�
�`b�ǋ�2g�P�            u// RUN: %llvmgcc %s -S -o - | grep noalias

void * __attribute__ ((malloc)) foo (void) { return 0; }
