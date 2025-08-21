#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(PLUGIN_CAMERA_VIEW_LIB)
#  define PLUGIN_CAMERA_VIEW_EXPORT Q_DECL_EXPORT
# else
#  define PLUGIN_CAMERA_VIEW_EXPORT Q_DECL_IMPORT
# endif
#else
# define PLUGIN_CAMERA_VIEW_EXPORT
#endif
