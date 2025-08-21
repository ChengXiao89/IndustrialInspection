#pragma once
#include "interface.h"

class PLUGININTERFACE_EXPORT inference_control_pane : public interface_plugin
{
public:
    inference_control_pane() = default;
    virtual ~inference_control_pane() = default;

};

#define IPluginControlPane_iid "com.mycompany.IPluginControlPane"
Q_DECLARE_INTERFACE(inference_control_pane, IPluginControlPane_iid)

