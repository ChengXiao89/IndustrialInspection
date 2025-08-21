#include "central_image_viewer.h"


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
}

void image_viewer::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!m_texture || !m_texture->isCreated())
        return;
    m_texture->bind();
    glEnable(GL_TEXTURE_2D);
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1); glVertex2f(-1, -1);
    glTexCoord2f(1, 1); glVertex2f(1, -1);
    glTexCoord2f(1, 0); glVertex2f(1, 1);
    glTexCoord2f(0, 0); glVertex2f(-1, 1);
    glEnd();
    glDisable(GL_TEXTURE_2D);
    m_texture->release();
}

void image_viewer::set_image(const QImage& image)
{
    m_image = image; // RGB888 格式
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