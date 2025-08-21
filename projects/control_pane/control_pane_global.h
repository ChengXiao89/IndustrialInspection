#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(CONTROL_PANE_LIB)
#  define CONTROL_PANE_EXPORT Q_DECL_EXPORT
# else
#  define CONTROL_PANE_EXPORT Q_DECL_IMPORT
# endif
#else
# define CONTROL_PANE_EXPORT
#endif
