#include "QledWidget.h"
#include "widget.h"

QledWidget::QledWidget(QWidget *parent) :
    QWidget(parent), currentPage(0){
    mainLayout = new QVBoxLayout(this);
    gridLayout = new QGridLayout();
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    // 创建导航按钮
    QPushButton *prevButton = new QPushButton("上一页", this);
    connect(prevButton, &QPushButton::clicked, this, &QledWidget::previousPage);

    QPushButton *nextButton = new QPushButton("下一页", this);
    connect(nextButton, &QPushButton::clicked, this, &QledWidget::nextPage);

    buttonLayout->addStretch(); // 增加一些间距
    buttonLayout->addWidget(prevButton);
    buttonLayout->addWidget(nextButton);
    buttonLayout->addStretch();

    // 初始化按钮
    initializeButtons();

    mainLayout->addLayout(gridLayout);
    mainLayout->addLayout(buttonLayout);
}

void QledWidget::setColorArray(const QVector<QColor> &colors) {
    colorArray = colors;  // 设置颜色数组
    updatePage();  // 更新页面
}

void QledWidget::initializeButtons() {
    for(int i = 0; i < LED_SIZE; ++i) {
        QPushButton *button = new QPushButton(QString("LED %1").arg(i + 1), this);
        connect(button, &QPushButton::clicked, this, &QledWidget::on_buttonLed_clicked);
        buttons.push_back(button);
    }
    updatePage();
}

void QledWidget::updatePage() {
    // 清除现有的布局内容
    QLayoutItem* item;
    while ((item = gridLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // 更新当前页面的按钮
    int start = currentPage * 16;
    for (int i = 0; i < 16 && (start + i) < buttons.size(); ++i) {
        int index = start + i;
        QPushButton *button = buttons[index];
        button->setStyleSheet(QString("background-color: %1;").arg(colorArray[index].name()));
        gridLayout->addWidget(button, i / 4, i % 4);
    }
}

void QledWidget::nextPage() {
    if ((currentPage + 1) * 16 < buttons.size()) {
        currentPage++;
        updatePage();
    }
}

void QledWidget::previousPage() {
    if (currentPage > 0) {
        currentPage--;
        updatePage();
    }
}

void QledWidget::on_buttonLed_clicked() {
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button) {
        int index = buttons.indexOf(button);
        if (index != -1) {
            emit buttonClicked(index);  // 发送按钮点击信号
        }
    }
}

const QVector<QColor>& QledWidget::getColorArray() const {
    return colorArray;  // 返回颜色数组的引用
}
