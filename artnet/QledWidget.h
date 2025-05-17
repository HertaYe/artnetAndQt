#ifndef QLEDWIDGET_H
#define QLEDWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVector>
#include <QColor>


class QledWidget : public QWidget {
    Q_OBJECT

public:
    explicit QledWidget(QWidget *parent = nullptr);
    void setColorArray(const QVector<QColor> &colors);  // 设置颜色数组
    const QVector<QColor>& getColorArray() const;       // 获取颜色数组
    void updatePage();  // 更新当前页面的按钮

signals:
    void buttonClicked(int index);  // 按钮点击信号

private slots:
    void nextPage();  // 下一页
    void previousPage();  // 上一页
    void on_buttonLed_clicked();  // 按钮点击事件

private:
    void initializeButtons();  // 初始化按钮

    QVector<QPushButton*> buttons;  // 按钮数组
    QVector<QColor> colorArray;  // 颜色数组
    QGridLayout *gridLayout;  // 网格布局
    QVBoxLayout *mainLayout;  // 主布局
    int currentPage;  // 当前页码
};

#endif // QLEDWIDGET_H
