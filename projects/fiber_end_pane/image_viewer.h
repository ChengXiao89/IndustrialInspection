/*************************
 * 影像显示窗口
 *************************/
#pragma once

#include <QtOpenGLWidgets/QOpenGLWidget>
#include <QOpenGLTexture>

class image_viewer : public QOpenGLWidget
{
    Q_OBJECT
public:
    explicit image_viewer(QWidget* parent = nullptr);
    ~image_viewer();
    void initialize();


    void set_image(const QImage& image);
    void draw_image();
    
protected:
    void paintGL() override;
    void resizeGL(int w, int h) override;
    void initializeGL() override;

private:
	QImage m_image;       //显示的影像数据指针，只负责显示，不负责内存管理
    QOpenGLTexture* m_texture{ nullptr };

};