#ifndef _COMPAT_H
#define _COMPAT_H

#ifdef _MSC_VER

#pragma warning(disable:4996) // _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4102) 

#define strdup _strdup

#define strncasecmp _strnicmp
#define strcasecmp _stricmp

#endif // _MSC_VER

#endif // _COMPAT_H