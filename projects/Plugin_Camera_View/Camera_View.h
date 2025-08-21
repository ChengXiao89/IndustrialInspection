#pragma once

#include <QObject>
#include <QDockWidget>
#include <QListWidget>
#include "../PluginInterface/interface_camera_view.h"

class plugin_camera_view : public QObject, public interface_camera_view
{
    Q_OBJECT
        Q_PLUGIN_METADATA(IID InterfaceCamera_iid)
        Q_INTERFACES(IPluginCamera)

public:
    plugin_camera_view();

    // IPluginInterface 接口
    QString name() const override;
    QWidget* widget() override;

    // IPluginCamera 接口
    void refreshCameraList() override;
    void openCamera(const QString& cameraId) override;

private:
    QDockWidget* m_dockWidget;
    QListWidget* m_listWidget;
};
