         Y   X        ��������i�W�f)�6��C��;w�Y/~�            u// RUN: %llvmgcc -xc %s -S -o - | grep -v div

int Diff(int *P, int *Q) { return P-Q; }
