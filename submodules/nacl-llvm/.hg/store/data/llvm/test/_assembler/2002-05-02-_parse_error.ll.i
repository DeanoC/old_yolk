         U   T        ��������2F���J猯�A�op�l            u; RUN: llvm-as %s -o /dev/null

%T = type i32 *

define %T @test() {
	ret %T null
}
