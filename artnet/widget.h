#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <QTcpSocket>
#include <QSlider>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QVector>
#include <QColor>
#include <QIntValidator>
#include "ui_widget.h"

#define NUM_LEDS 64
#define PAGE_SIZE 16

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_connentButton_clicked();    // 连接按钮点击事件
    void on_breakButton_clicked();      // 断开连接按钮点击事件
    void on_sendButton_clicked();       // 发送按钮点击事件
    void on_readButton_clicked();       // 连接按钮点击事件

    void on_clearButton_clicked();
    void on_setButton_clicked();

private:

    void initializePair(QSlider* slider, QLineEdit* lineEdit);  // 初始化滑块和输入框的同步逻辑
    void initializeLedButtons();
    void updateColorArrayFromSliders();
    void updateButtonAppearance();
    void updateButtons();
    void initializePageButtons();
    void initializeSocket();

    Ui::Widget *ui;
    QUdpSocket *socketUDP;  // UDP 套接字
    QTcpSocket *socketTCP;  // TCP 套接字
    QHostAddress IP;        // IP 地址
    quint16 port;           // 端口号
    QPlainTextEdit *messagePrompt;  // 消息提示框
    QWidget *leftWidget;
    QWidget *rightWidget;
    QGridLayout *gridLayoutLed;  // LED 控件

    int currentPage;  // 当前页面
    int selectedButtonIndex;
    QVector<QPushButton*> buttons;  // 按钮数组
    QVector<QColor> colorArray;  // 颜色数组

};

#endif // WIDGET_H
