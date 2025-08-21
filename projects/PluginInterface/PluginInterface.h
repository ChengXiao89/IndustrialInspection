#pragma once

#include "plugininterface_global.h"
#include <QWidget>

class PLUGININTERFACE_EXPORT IPluginInterface
{
public:
    virtual ~IPluginInterface() {}
    virtual QString name() const = 0;
    virtual QWidget* widget() = 0; // 插件对应的 UI 窗口
};

#define IPluginInterface_iid "com.mycompany.IPluginInterface"
Q_DECLARE_INTERFACE(IPluginInterface, IPluginInterface_iid)

//void registerPluginInterfaces();
