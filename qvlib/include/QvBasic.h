#ifndef  _QV_BASIC_
#define  _QV_BASIC_

#include <sys/types.h>
#include <libc.h>
#include <stdio.h>

#ifndef FALSE
#   define FALSE	0
#   define TRUE		1
#endif

typedef int QvBool;

// This uses the preprocessor to quote a string
#if defined(__STDC__) || defined(__ANSI_CPP__)		/* ANSI C */
#  define QV__QUOTE(str)	#str
#else							/* Non-ANSI C */
#  define QV__QUOTE(str)	"str"
#endif

// This uses the preprocessor to concatenate two strings
#if defined(__STDC__) || defined(__ANSI_CPP__)		/* ANSI C */
#   define QV__CONCAT(str1, str2)	str1##str2
#else							/* Non-ANSI C */
#   define QV__CONCAT(str1, str2)	str1/**/str2
#endif

#endif /* _QV_BASIC_ */
