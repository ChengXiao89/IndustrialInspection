#include "Camera_View.h"

plugin_camera_view::plugin_camera_view()
{

}


// IPluginInterface ½Ó¿Ú
QString plugin_camera_view::name() const
{
	return QString("Camera View Plugin");
}
QWidget* plugin_camera_view::widget()
{
	return m_dockWidget;
}

void plugin_camera_view::refreshCameraList()
{
	
}
void plugin_camera_view::openCamera(const QString& cameraId)
{
	
}