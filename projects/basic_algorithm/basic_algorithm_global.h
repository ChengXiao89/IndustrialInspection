#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(BASIC_ALGORITHM_LIB)
#  define BASIC_ALGORITHM_EXPORT Q_DECL_EXPORT
# else
#  define BASIC_ALGORITHM_EXPORT Q_DECL_IMPORT
# endif
#else
# define BASIC_ALGORITHM_EXPORT
#endif
