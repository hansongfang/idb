#ifndef DLL_H
#define DLL_H

#include <QLibrary>

#if defined(DEF_TRIDAG_LIB)
#  define TRIDAG_LIB Q_DECL_EXPORT
#else
#  define TRIDAG_LIB Q_DECL_IMPORT
#endif


#endif // DLL_H
