/************************************
 * 检测结果缩略图窗口
 ***********************************/
#pragma once
#include <QLabel>
#include <QWidget>
#include <QBoxLayout>
#include <QScrollArea>
#include <QListWidget>

 //缩略图，每个小图片对应一个 id 和绘制控件 QLabel
struct st_thumbnail
{
    int m_id{ 0 };
    QLabel* label{ nullptr };
};

class thumbnail_bar : public QWidget
{
    Q_OBJECT
public:
    explicit thumbnail_bar(QWidget* parent = nullptr);
    virtual ~thumbnail_bar() override;

    void resizeEvent(QResizeEvent* event) override;

    void initialize();

    void add_thumbnail(const QPixmap& pixmap, int id);
    void clear();
    void select_thumbnails(const QList<int>& ids);          // 外部点击列表时需要选中对应的缩略图


signals:
    void post_thumbnail_clicked(int index);

public slots:
    void on_thumbnail_selected(void* data);     //外部点击列表时需要更新选中对应的缩略图，因此要向缩略图窗口发送消息

private:

    QListWidget* m_list_widget{ nullptr };

    QVector<st_thumbnail> m_thumbnail_list;         //缩略图列表，每个缩略图对应一个 id 和 QLabel
    QList<int> m_selected_ids;                      // 当前选中的id集合
    int m_columns{ 1 };                               // 当前列数
    QSize m_thumbnail_size{ QSize(96, 96) };   // 缩略图大小
};
