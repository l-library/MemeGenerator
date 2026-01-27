#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QPushButton"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 图标和名称初始化
    this->setWindowIcon(QIcon(":/icons/MemeGenerator.png"));
    this->setWindowTitle("表情包生成器");

    // 菜单栏初始化
    initMenuBar();

    auto button = new QPushButton("test", this);
    button->move(0, 40);

    connect(button, &QPushButton::clicked, this, [=]()
            { qDebug() << "pressed"; });
}

void MainWindow::initMenuBar()
{
    // 获取/创建菜单栏
    QMenuBar *menu_bar = this->menuBar();
    this->setMenuBar(menu_bar);

    // 创建各菜单
    if(!menu_bar) return;
    QMenu *menu_file = new QMenu("文件(&F)",menu_bar);
    QMenu *menu_insert = new QMenu("插入(&I)");
    QMenu *menu_tool = new QMenu("工具(&T)");
    QMenu *menu_effect = new QMenu("效果(&E)");
    QMenu *menu_help = new QMenu("帮助(&H)");
    menu_bar->addMenu(menu_file);
    menu_bar->addMenu(menu_insert);
    menu_bar->addMenu(menu_tool);
    menu_bar->addMenu(menu_effect);
    menu_bar->addMenu(menu_help);

    // 文件菜单项
    QAction *act_new = new QAction(QIcon(":/icons/filenew.png"), "新建文件(&N)", menu_file);
    act_new->setShortcut(QKeySequence::New);  // 快捷键：Ctrl+N
    act_new->setStatusTip("新建空白文件");
    QAction *act_open = new QAction(QIcon(":/icons/fileopen.png"), "打开文件(&O)", menu_file);
    act_open->setShortcut(QKeySequence::Open);
    act_open->setStatusTip("打开图片文件");
    QAction *act_save = new QAction(QIcon(":/icons/filesave.png"), "保存文件(&S)", menu_file);
    act_save->setShortcut(QKeySequence::Save);
    act_save->setStatusTip("保存当前文件");
    QAction *act_saveas = new QAction(QIcon(":/icons/filesaveas.png"), "另存为(&A)", menu_file);
    act_saveas->setShortcut(QKeySequence::SaveAs);
    act_saveas->setStatusTip("将当前文件另存在指定位置");
    menu_file->addAction(act_new);
    menu_file->addAction(act_open);
    menu_file->addAction(act_save);
    menu_file->addAction(act_saveas);
}


MainWindow::~MainWindow()
{
    delete ui;
}
