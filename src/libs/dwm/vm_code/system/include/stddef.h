#ifndef _INC_STDDEF
#define _INC_STDDEF

#define NULL    nullptr

#ifdef __cplusplus
//namespace std { typedef nullptr nullptr_t; }
#define offsetof(s,m)   (size_t)&reinterpret_cast<const volatile char&>((((s *)0)->m))
#else
#define offsetof(s,m)   (size_t)&(((s *)0)->m)
#endif

#ifndef _SIZE_T_DEFINED
typedef unsigned int   size_t;
#define _SIZE_T_DEFINED
#endif

#ifndef _INTPTR_T_DEFINED
typedef int            intptr_t;
#define _INTPTR_T_DEFINED
#endif

#ifndef _UINTPTR_T_DEFINED
typedef unsigned int   uintptr_t;
#define _UINTPTR_T_DEFINED
#endif

#ifndef _PTRDIFF_T_DEFINED
typedef  int            ptrdiff_t;
#define _PTRDIFF_T_DEFINED
#endif

#ifndef _WCHAR_T_DEFINED
typedef unsigned char wchar_t;
#define _WCHAR_T_DEFINED
#endif

#ifndef _WCTYPE_T_DEFINED
typedef unsigned char wint_t;
typedef unsigned char wctype_t;
#define _WCTYPE_T_DEFINED
#endif

#endif