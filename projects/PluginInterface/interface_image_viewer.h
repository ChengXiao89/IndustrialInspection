#pragma once

#include "interface.h"

class PLUGININTERFACE_EXPORT plugin_image_viewer : public interface_plugin
{
public:
    virtual ~plugin_image_viewer() = default;

    virtual void showImage(const QImage& image) = 0;
    virtual void setZoom(double factor) = 0;
};

#define IPluginImageViewer_iid "com.mycompany.IPluginImageViewer"
Q_DECLARE_INTERFACE(plugin_image_viewer, IPluginImageViewer_iid)