#pragma once

#include <QtCore/qglobal.h>

#ifdef BUILD_STATIC
	# define CAMERA_VIEW_EXPORT
#else
	# if defined(CAMERA_VIEW_LIB)
	#  define CAMERA_VIEW_EXPORT Q_DECL_EXPORT
	# else
	#  define CAMERA_VIEW_EXPORT Q_DECL_IMPORT
	# endif
#endif
