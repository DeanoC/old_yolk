         a   `        ��������8��
^��H����7��GB �            u; RUN: not llvm-as < %s >& /dev/null
; PR2711
	%s = type opaque
declare void @h(%s* byval %num)
