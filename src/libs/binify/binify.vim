" binify
" Language:binify
" Maintainer:Ben Campbell (ben.campbell@justaddmonsters.com)
" Last Change:2003 Feb 12
" Location: http://binify.sf.net

" still needs lots of work!


" For version 5.x: Clear all syntax items
" For version 6.x: Quit when a syntax file was already loaded
if version < 600
	syntax clear
elseif exists("b:current_syntax")
	finish
endif


syn keyword binifyStatement		align bigendian littleendian type blank u8 u16 u32 s8 s16 s32 float double
syn match binifyIdentifier		"\w\+"
syn match binifyComment			"//.*$"

syn match binifyString		"\".*\""
syn match binifyInteger		"\d\+"
syn match binifyFloat		"\d*.\d\+"
syn match binifyHex			"0x\x\+"
syn match binifyBinary		"%[01]\+"
syn match binifyOctal		"0\o\+"
syn cluster binifyNumber	contains=binifyInteger,binifyHex,binifyBinary,binifyOctal,binifyFloat


" Define the default highlighting.
" For version 5.7 and earlier: only when not done already
" For version 5.8 and later: only when an item doesn't have highlighting yet
if version >= 508 || !exists("did_binify_syntax_inits")
	if version < 508
		let did_binify_syntax_inits = 1
		command -nargs=+ HiLink hi link <args>
	else
		command -nargs=+ HiLink hi def link <args>
	endif

	HiLink binifyStatement	Statement
	HiLink binifyIdentifier	Identifier
	HiLink binifyString		String
	HiLink binifyNumber		Number
	HiLink binifyInteger	binifyNumber
	HiLink binifyHex		binifyNumber
	HiLink binifyBinary		binifyNumber
	HiLink binifyOctal		binifyNumber
	HiLink binifyFloat		binifyNumber

	HiLink binifyComment	Comment

	delcommand HiLink
endif

let b:current_syntax = "binify"



