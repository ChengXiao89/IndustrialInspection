#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(FIBER_END_PANE_LIB)
#  define FIBER_END_PANE_EXPORT Q_DECL_EXPORT
# else
#  define FIBER_END_PANE_EXPORT Q_DECL_IMPORT
# endif
#else
# define FIBER_END_PANE_EXPORT
#endif
