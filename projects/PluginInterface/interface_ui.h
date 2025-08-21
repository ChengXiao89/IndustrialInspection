#pragma once
#include "plugininterface_global.h"

#include <QWidget>

class PLUGININTERFACE_EXPORT interface_plugin_ui
{
public:
    interface_plugin_ui() = default;
    virtual ~interface_plugin_ui() = default;

    enum class UIType { DockWidget, Widget, Dialog };
    virtual QWidget* widget() const = 0;
    virtual UIType widget_type() const = 0;
};

#define interface_plugin_ui_iid "com.mycompany.interface_plugin_ui"
Q_DECLARE_INTERFACE(interface_plugin_ui, interface_plugin_ui_iid)