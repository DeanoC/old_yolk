         u   t        ��������1�+�Pn` �)u745���            u/* RUN: %llvmgcc -w -x objective-c -S %s -o - | not grep {__ustring}
   rdar://7589850 */

void *P = @"good\0bye";

