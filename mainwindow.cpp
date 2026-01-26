#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QPushButton"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QMenuBar *bar = new QMenuBar(this);
    setMenuBar(bar);
    bar->addMenu("文件");
    bar->addMenu("编辑");
    bar->addMenu("帮助");

    auto button = new QPushButton("test", this);
    button->move(0, 40);

    connect(button, &QPushButton::clicked, this, [=]()
            { qDebug() << "pressed"; });
}

MainWindow::~MainWindow()
{
    delete ui;
}
