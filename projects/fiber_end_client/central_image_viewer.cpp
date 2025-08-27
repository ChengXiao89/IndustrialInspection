#include "central_image_viewer.h"
#include <QSize>


image_viewer::image_viewer(QWidget* parent)
    : QOpenGLWidget(parent)
{

}

image_viewer::~image_viewer()
{
    makeCurrent();
    if (m_texture != nullptr)
    {
        delete m_texture;
        m_texture = nullptr;
    }
    doneCurrent();
}

void image_viewer::initialize()
{
    setMinimumSize(480, 180);
}

void image_viewer::initializeGL()
{
    //initializeOpenGLFunctions();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void image_viewer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    QSize size = this->size();
    double aspect_image = static_cast<double>(m_image.width()) / m_image.height();
    double aspect_window = static_cast<double>(size.width()) / size.height();
    if (aspect_window <= aspect_image)
    {
        m_x[0] = -1.0;  m_y[0] = -aspect_window / aspect_image;
        m_x[1] = 1.0;   m_y[1] = -aspect_window / aspect_image;
        m_x[2] = 1.0;   m_y[2] = aspect_window / aspect_image;
        m_x[3] = -1.0;  m_y[3] = aspect_window / aspect_image;
    }
    else
    {
        m_x[0] = -aspect_image / aspect_window;  m_y[0] = -1.0;
        m_x[1] = aspect_image / aspect_window;   m_y[1] = -1.0;
        m_x[2] = aspect_image / aspect_window;   m_y[2] = 1.0;
        m_x[3] = -aspect_image / aspect_window;  m_y[3] = 1.0;
    }
}

void image_viewer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_texture || !m_texture->isCreated())
        return;
    m_texture->bind();
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(m_x[0], m_y[0]);
    glTexCoord2f(1, 1); glVertex2f(m_x[1], m_y[1]);
    glTexCoord2f(1, 0); glVertex2f(m_x[2], m_y[2]);
    glTexCoord2f(0, 0); glVertex2f(m_x[3], m_y[3]);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    m_texture->release();
}

void image_viewer::set_image(const QImage& image)
{
    m_image = image; // RGB888 格式
    QSize size = this->size();
    double aspect_image = static_cast<double>(m_image.width()) / m_image.height();
    double aspect_window = static_cast<double>(size.width()) / size.height();
    if(aspect_window <= aspect_image)
    {
        m_x[0] = -1.0;  m_y[0] = -aspect_window / aspect_image;
        m_x[1] = 1.0;   m_y[1] = -aspect_window / aspect_image;
        m_x[2] = 1.0;   m_y[2] = aspect_window / aspect_image;
        m_x[3] = -1.0;  m_y[3] = aspect_window / aspect_image;
    }
    else
    {
        m_x[0] = -aspect_image / aspect_window;  m_y[0] = -1.0;
        m_x[1] = aspect_image / aspect_window;   m_y[1] = -1.0;
        m_x[2] = aspect_image / aspect_window;   m_y[2] = 1.0;
        m_x[3] = -aspect_image / aspect_window;  m_y[3] = 1.0;
    }

    if (0)
    {
        m_image.save("D:/Temp/123.jpg");        //测试保存到图像
    }
    draw_image();
}


void image_viewer::draw_image()
{
    makeCurrent();
    if (m_texture)
    {
        delete m_texture;
        m_texture = nullptr;
    }
    m_texture = new QOpenGLTexture(m_image);
    doneCurrent();
    update();  // 触发重绘
}