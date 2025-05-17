#include "widget.h"

Widget::Widget(QWidget *parent):
    QWidget(parent),
    ui(new Ui::Widget) {

    ui->setupUi(this);

    // 设置窗口大小
    this->setFixedSize(640, 480);

    leftWidget = ui->leftWidget;
    leftWidget->setStyleSheet("background-color: #f0f0f0;");

    rightWidget = ui->rightWidget;
    rightWidget->setStyleSheet("background-color: #e0e0e0;");

    gridLayoutLed =ui->gridLayoutLed;

    // 设置消息框
    messagePrompt= ui->messagePrompt;
    messagePrompt->setStyleSheet("background-color: #d0d0d0;");
    messagePrompt->setReadOnly(true);

    initializePair(ui->sliderR, ui->lineEditR);
    initializePair(ui->sliderG, ui->lineEditG);
    initializePair(ui->sliderB, ui->lineEditB);

    initializeLedButtons();
    initializePageButtons();
    initializeSocket();

}

Widget::~Widget() {
    delete ui;

    // 释放 UDP 和 TCP 套接字
    if (socketUDP) {
        socketUDP->close();
        delete socketUDP;
    }
    if (socketTCP) {
        socketTCP->close();
        delete socketTCP;
    }
}

void Widget::on_connentButton_clicked() {
    IP = QHostAddress(ui->ipLineEdit->text());
    port = ui->portLineEdit->text().toShort();
    // 连接成功
    if (socketTCP->state() != QAbstractSocket::ConnectedState){
        socketTCP->connectToHost(IP,80);
    }else {
        messagePrompt->appendPlainText("已连接");
    }
}

void Widget::on_breakButton_clicked() {
    socketTCP->disconnectFromHost();
}

void Widget::on_readButton_clicked() {

}

void Widget::on_sendButton_clicked() {
    QByteArray data;
    // 直接添加Art-Net包头
    const char artNetHeader[] = { 'A', 'r', 't', '-', 'N', 'e', 't', 0x00 };
    data.append(artNetHeader, sizeof(artNetHeader));

    // 使用QDataStream来追加其他类型的变量
    QByteArray dynamicPart;
    QDataStream out(&dynamicPart, QIODevice::WriteOnly);
    out.setByteOrder(QDataStream::BigEndian);
    // Art-Net协议头
    out << quint16(0x50);  // OpCode: OpOutput (0x5000)

    quint8 ProtVerHi = 0, ProtVerLo = 14;// ProtVerHi and ProtVerLo: Protocol Version 14
    quint8 sequence = 0;     // 序列号，对于简单的应用可以固定为0
    quint8 physical = 0;     // 物理端口，通常为0
    quint8 subUni = 0;       // 子网和Universe ID
    quint8 net = 0;          // 网络地址，默认为0
    quint16 dmxLength = NUM_LEDS * 3;  // DMX数据长度，每个颜色占3个字节（RGB）

    out << ProtVerHi << ProtVerLo << sequence << physical << subUni << net << dmxLength;

    // 将colorArray中的颜色数据写入Art-Net数据包
    for (const QColor& color : colorArray) {
        out << static_cast<quint8>(color.red());
        out << static_cast<quint8>(color.green());
        out << static_cast<quint8>(color.blue());
    }
    data.append(dynamicPart);
    qDebug() << "Final data packet:" << data;

    // 使用QUdpSocket发送数据包
    if (socketUDP->writeDatagram(data, IP, 6454) == -1) {
        messagePrompt->appendPlainText("发送失败");
    } else {
        messagePrompt->appendPlainText("已发送Art-Net信号");
    }
}

void Widget::on_clearButton_clicked() {
    // 重置颜色数组
    for (QColor& color : colorArray) {
        color = QColor(0, 0, 0);  // 将所有颜色重置为黑色
    }

    // 重置滑块和输入框的值
    ui->sliderR->setValue(0);
    ui->sliderG->setValue(0);
    ui->sliderB->setValue(0);

    // 更新按钮外观
    for (QPushButton* button : buttons) {
        if (button) {
            button->setStyleSheet("background-color: black;");
        }
    }

    // 重置当前页面
    currentPage = 0;
    updateButtons();

    // 提示用户
    messagePrompt->appendPlainText("已设置所有按钮颜色");
}

void Widget::on_setButton_clicked()
{
    // 重置颜色
    for (int selectedButtons= 0; selectedButtons < colorArray.size(); selectedButtons++) {
        QColor color = QColor(rand()%256,rand()%256,rand()%256);  // 重置所有颜色
        colorArray[selectedButtons] = color;  // 更新颜色数组
        buttons[selectedButtons]->setStyleSheet(QString("background-color: %1;").arg(color.name()));  // 设置按钮背景颜色

    }

    // 重置当前页面
    currentPage = 0;
    updateButtons();

    // 提示用户
    messagePrompt->appendPlainText("已设置所有按钮颜色");
}


void Widget::initializeSocket(){
    socketUDP = new QUdpSocket;
    socketTCP = new QTcpSocket;

    // 连接信号
    connect(socketTCP, &QTcpSocket::connected, this, [this](){
        messagePrompt->appendPlainText(IP.toString() + " 连接");
    });
    connect(socketTCP, &QTcpSocket::disconnected, this, [this](){
        messagePrompt->appendPlainText("连接断开");
    });
}

void Widget::initializePair(QSlider* slider, QLineEdit* lineEdit) {
    // 设置输入框的验证器，只允许整数输入
    lineEdit->setValidator(new QIntValidator(slider->minimum(), slider->maximum(), this));

    // 初始值同步
    lineEdit->setText(QString::number(slider->value()));

    // 当滑动条的值变化时，更新文本框和颜色数组
    connect(slider, &QSlider::valueChanged, [=](int value) {
        bool isBlocked = lineEdit->blockSignals(true);  // 阻止信号以避免循环调用
        lineEdit->setText(QString::number(value));
        lineEdit->blockSignals(isBlocked);  // 恢复信号

        updateColorArrayFromSliders();  // 更新颜色数组
        updateButtonAppearance();      // 更新按钮外观
    });

    // 当文本框的内容变化时，更新滑动条和颜色数组
    connect(lineEdit, &QLineEdit::textChanged, [=](const QString &text) {
        bool ok;
        int value = text.toInt(&ok);
        if (ok && value >= slider->minimum() && value <= slider->maximum()) {
            bool isBlocked = slider->blockSignals(true);  // 阻止信号以避免循环调用
            slider->setValue(value);
            slider->blockSignals(isBlocked);  // 恢复信号

            updateColorArrayFromSliders();  // 更新颜色数组
            updateButtonAppearance();      // 更新按钮外观
        }
    });
}

void Widget::updateColorArrayFromSliders() {
    if (selectedButtonIndex >= 0 && selectedButtonIndex < colorArray.size()) {
        QColor color(
            ui->sliderR->value(),  // 红色分量
            ui->sliderG->value(),  // 绿色分量
            ui->sliderB->value()   // 蓝色分量
        );
        colorArray[selectedButtonIndex] = color;  // 更新颜色数组
    }
}

void Widget::updateButtonAppearance() {
    if (selectedButtonIndex >= 0 && selectedButtonIndex < buttons.size()) {
        QPushButton* button = buttons[selectedButtonIndex];
        QColor color = colorArray[selectedButtonIndex];
        button->setStyleSheet(QString("background-color: %1;").arg(color.name()));  // 设置按钮背景颜色
    }
}

void Widget::initializeLedButtons() {
    // 初始化颜色数组
    colorArray.resize(NUM_LEDS);  // 设置数组大小为 NUM_LEDS

    // 添加 64 个按钮到网格布局中
    for (int i = 0; i < NUM_LEDS; ++i) {
        QPushButton *button = new QPushButton(QString("Button %1").arg(i), this);  // 设置父对象
        buttons.push_back(button);  // 将按钮添加到按钮数组
        colorArray[i] = QColor(0, 0, 0);  // 初始颜色为黑色
        button->setStyleSheet("background-color: black;");  // 设置按钮背景颜色为黑色

        // 绑定按钮点击事件
        connect(button, &QPushButton::clicked, this, [this, i]() {
            selectedButtonIndex = i;  // 记录选中的按钮索引
            if (selectedButtonIndex >= 0 && selectedButtonIndex < colorArray.size()) {
                // 更新滑块和输入框的值
                QColor color = colorArray[selectedButtonIndex];
                ui->sliderR->setValue(color.red());
                ui->sliderG->setValue(color.green());
                ui->sliderB->setValue(color.blue());
            }
            qDebug() << "选中按钮：" << i;
        });
    }
    // 初始化当前页面
    currentPage = 0;
    updateButtons();
}

void Widget::updateButtons() {
    // 隐藏所有按钮
    for (QPushButton* button : buttons) {
        if (button) button->setVisible(false);  // 隐藏按钮
    }

    // 计算当前页面的按钮范围
    int start = currentPage * PAGE_SIZE;
    int end = qMin(start + PAGE_SIZE, NUM_LEDS);

    // 显示当前页按钮
    for (int i = start; i < end; ++i) {
        if (i >= 0 && i < buttons.size() && buttons[i] != nullptr) {  // 检查索引和指针有效性
            int row = (i - start) / 4;
            int column = (i - start) % 4;
            if (gridLayoutLed) {
                gridLayoutLed->addWidget(buttons[i], row, column);  // 将按钮添加到布局
                buttons[i]->setVisible(true);  // 显示按钮
                qDebug() << "显示按钮：" << buttons[i]->text() << "到位置 (" << row << "," << column << ")";
            }
        } else {
            qWarning() << "无效的按钮索引或空指针：" << i;
        }
    }
}

void Widget::initializePageButtons() {
    connect(ui->pushButtonFront, &QPushButton::clicked, this, [this]() {
        if (currentPage > 0) {
            currentPage--;  // 切换到上一页
            updateButtons();  // 更新按钮显示
            messagePrompt->appendPlainText(QString("当前页码：%1").arg(currentPage + 1));
        }
    });

    connect(ui->pushButtonAfter, &QPushButton::clicked, this, [this]() {
        if (currentPage < (NUM_LEDS / PAGE_SIZE) - 1) {
            currentPage++;  // 切换到下一页
            updateButtons();  // 更新按钮显示
            messagePrompt->appendPlainText(QString("当前页码：%1").arg(currentPage + 1));
        }
    });
}

