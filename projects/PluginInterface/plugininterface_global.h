#pragma once

#include <QtCore/qglobal.h>

#ifdef PLUGININTERFACE_STATIC_LIB
	# define PLUGININTERFACE_EXPORT
#else
	# if defined(PLUGININTERFACE_LIB)
	#  define PLUGININTERFACE_EXPORT Q_DECL_EXPORT
	# else
	#  define PLUGININTERFACE_EXPORT Q_DECL_IMPORT
	# endif
#endif
