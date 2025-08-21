/*****************************
 * 检测结果状态
 *****************************/
#pragma once
#include <QTableWidget>

class status_table : public QTableWidget
{
    Q_OBJECT
public:
    explicit status_table(QWidget* parent = nullptr);
    ~status_table();

    void initialize();
protected:


private:
    int m_column_count{ 4 }; // 列数,外部设置. 默认为 4

};