#include "mainwindow.h"
#include "menuconfig.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QFileDialog>
#include <qimagereader.h>
#include <QDebug>
#include <QPushButton>
#include <ui_mainwindow.h>
#include <QLabel>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QGraphicsPixmapItem>
#include <QGraphicsDropShadowEffect>
#include <QInputDialog>
#include "resizableitem.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
     m_grid_layout(nullptr),m_graphics_scene(nullptr)
    , m_graphics_view(nullptr), m_current_z_value(0)
    ,m_view_scale(1.0)
{
    ui->setupUi(this);

    // 图标和名称初始化
    this->setWindowIcon(QIcon(":/icons/MemeGenerator.png"));
    this->setWindowTitle("表情包生成器");

    // 加载配置文件
    loadMenuConfig();
    // 菜单栏初始化
    initMenuBar();

    // 创建网格布局
    m_grid_layout= new QGridLayout(this);
    // 设置行和列的比例
    for(int i=0;i<10;++i){
    m_grid_layout->setRowStretch(i, 10);  // 将行均分为10份
    }
    m_grid_layout->setColumnStretch(0, 20);  // 第一列占20%
    m_grid_layout->setColumnStretch(1, 60);  // 第二列占60%
    m_grid_layout->setColumnStretch(2, 20);  // 第三列占20%
    m_grid_layout->setSpacing(0);
    m_grid_layout->setContentsMargins(0, 0, 0, 0);
    setCentralWidget(new QWidget(this));
    centralWidget()->setContentsMargins(0, 0, 0, 0);
    centralWidget()->setLayout(m_grid_layout);

    // 初始化图像视图
    initGraphicsView();

    // 设置状态栏
    this->statusBar()->setStyleSheet(
        "QStatusBar {"
        "   border-top: 1px solid #B0B0B0;"  /* 顶部添加 1px 的灰色实线 */
        "   background-color: #F5F5F5;"      /* 背景色稍微改浅灰，与白色视图区分 */
        "}"
        "QStatusBar::item {"
        "   border: none;"                   /* 去掉状态栏内部每个小格子的边框*/
        "}"
        );
    ui->statusbar->showMessage("准备就绪");

    // 初始化右侧按键
    initButton();
}

void MainWindow::initMenuBar() {
    createMenuFromConfig();
}

void MainWindow::initGraphicsView()
{
    // 图像视图初始化
    m_graphics_scene = new QGraphicsScene(this);
    m_graphics_view = new QGraphicsView(this);
    m_graphics_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_graphics_view->setScene(m_graphics_scene);
    // 添加view到布局
    m_grid_layout->addWidget(m_graphics_view,0,0,10,2);
    // 设置视图属性
    m_graphics_view->setRenderHint(QPainter::Antialiasing); // 开启抗锯齿
    m_graphics_view->setRenderHint(QPainter::SmoothPixmapTransform); // 光滑像素
    m_graphics_view->setDragMode(QGraphicsView::RubberBandDrag); // 局部框选开启
    m_graphics_view->setInteractive(true); // 开启交互
    m_graphics_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse); // 设置锚点
    // 设置场景背景
    m_graphics_scene->setBackgroundBrush(QBrush(Qt::white));
}

void MainWindow::initButton()
{
    // 开启/关闭网格线按钮
    QPushButton* cross_open_btn = new QPushButton(this);
    cross_open_btn->setText("开启网格线");
    cross_open_btn->setStatusTip("开启网格线(Ctrl+Shift+C)");
    cross_open_btn->setIcon(QIcon(":/icons/crossopen.png"));
    cross_open_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    cross_open_btn->setCheckable(true);
    connect(cross_open_btn, &QPushButton::toggled, [=](bool on) {
        if (on) {
            m_graphics_scene->setBackgroundBrush(QBrush(Qt::lightGray,Qt::CrossPattern));
        } else {
            m_graphics_scene->setBackgroundBrush(QBrush(Qt::white));
        }
    });
    cross_open_btn->setShortcut(QKeySequence("Ctrl+Shift+C"));
    m_grid_layout->addWidget(cross_open_btn,0,2);

    // 插入图片按钮
    QPushButton* insert_picture_btn = new QPushButton(this);
    insert_picture_btn->setText("插入图片");
    insert_picture_btn->setStatusTip("从文件中选择图片插入(Ctrl+I)");
    insert_picture_btn->setIcon(QIcon(":/icons/insertpicture.png"));
    insert_picture_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(insert_picture_btn, &QPushButton::pressed, [=]() {
        onInsertPicture();
    });
    m_grid_layout->addWidget(insert_picture_btn,1,2);

    // 插入文本按钮
    QPushButton* insert_text_btn = new QPushButton(this);
    insert_text_btn->setText("插入文本");
    insert_text_btn->setStatusTip("插入一个文本框(Ctrl+T)");
    insert_text_btn->setIcon(QIcon(":/icons/inserttext.png"));
    insert_text_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(insert_text_btn, &QPushButton::pressed, [=]() {
        onInsertText();
    });
    m_grid_layout->addWidget(insert_text_btn,2,2);

    // 导出按钮
    QPushButton* save_file_button = new QPushButton(this);
    save_file_button->setText("导出");
    save_file_button->setStatusTip("将当前视图导出为图片(Ctrl+S)");
    save_file_button->setIcon(QIcon(":/icons/export.png"));
    save_file_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(save_file_button, &QPushButton::pressed, [=]() {
        onSaveAsFile();
    });
    // 设置按钮的文字颜色和背景色
    save_file_button->setStyleSheet("color: black; background-color: #FFFD55;");
    m_grid_layout->addWidget(save_file_button,8,2,10,2);
}

void MainWindow::loadMenuConfig() {
    // 从资源文件加载配置
    QString configPath = ":/config/MenuConfig.json";
    m_menuConfigs = MenuConfigManager::loadFromJson(configPath);

    if (m_menuConfigs.isEmpty()) {
        // 如果配置文件加载失败，使用默认配置
        setupDefaultMenuConfig();
    }
}

void MainWindow::createMenuFromConfig() {
    QMenuBar *menuBar = this->menuBar();

    for (long long i=0;i<m_menuConfigs.size();++i) {
        const MenuConfig& menuConfig = m_menuConfigs[i];
        QMenu *menu = new QMenu(menuConfig.title, this);

        for (const MenuItemConfig& itemConfig : menuConfig.items) {
            if (itemConfig.separator) {
                menu->addSeparator();
                continue;
            }

            // 创建QAction
            QAction *action = new QAction(this);

            // 设置文本
            action->setText(itemConfig.text);

            // 设置图标
            if (!itemConfig.iconPath.isEmpty()) {
                action->setIcon(QIcon(itemConfig.iconPath));
            }

            // 设置快捷键
            if (!itemConfig.standardKey.isEmpty()) {
                QKeySequence::StandardKey standardKey =
                    MenuConfigManager::stringToStandardKey(itemConfig.standardKey);
                if (standardKey != QKeySequence::UnknownKey) {
                    action->setShortcut(standardKey);
                }
            } else if (!itemConfig.shortcut.isEmpty()) {
                action->setShortcut(QKeySequence(itemConfig.shortcut));
            }

            // 设置状态提示
            action->setStatusTip(itemConfig.statusTip);

            // 设置启用状态
            action->setEnabled(itemConfig.enabled);

            // 设置可勾选
            action->setCheckable(itemConfig.checkable);

            // 添加到菜单
            menu->addAction(action);

            // 保存到映射表
            if (!itemConfig.id.isEmpty()) {
                m_actionMap[itemConfig.id] = action;

                // 连接信号槽
                if (!itemConfig.slot.isEmpty()) {
                    connectActionToSlot(itemConfig.slot, action);
                }
            }
        }

        // 添加到菜单栏
        menuBar->addMenu(menu);
    }
}

void MainWindow::connectActionToSlot(const QString& slotName, QAction* action) {
    if (slotName == "onNewFile") {
        connect(action, &QAction::triggered, this, &MainWindow::onNewFile);
    } else if (slotName == "onOpenFile") {
        connect(action, &QAction::triggered, this, &MainWindow::onOpenFile);
    } else if (slotName == "onSaveFile") {
        connect(action, &QAction::triggered, this, &MainWindow::onSaveFile);
    } else if (slotName == "onSaveAsFile") {
        connect(action, &QAction::triggered, this, &MainWindow::onSaveAsFile);
    } else if (slotName == "onExit") {
        connect(action, &QAction::triggered, this, &MainWindow::onExit);
    } else if (slotName == "onUndo") {
        connect(action, &QAction::triggered, this, &MainWindow::onUndo);
    } else if (slotName == "onRedo") {
        connect(action, &QAction::triggered, this, &MainWindow::onRedo);
    }else if (slotName == "onZoomIn") {
        connect(action, &QAction::triggered, this, &MainWindow::onZoomIn);
    }else if (slotName == "onZoomOut") {
        connect(action, &QAction::triggered, this, &MainWindow::onZoomOut);
    }else if (slotName == "onCopy") {
        connect(action, &QAction::triggered, this, &MainWindow::onCopy);
    }else if (slotName == "onPaste") {
        connect(action, &QAction::triggered, this, &MainWindow::onPaste);
    }else if (slotName == "onInsertPicture") {
        connect(action, &QAction::triggered, this, &MainWindow::onInsertPicture);
    }else if (slotName == "onInsertText") {
        connect(action, &QAction::triggered, this, &MainWindow::onInsertText);
    }else if (slotName == "onCutting") {
        connect(action, &QAction::triggered, this, &MainWindow::onCutting);
    }else if (slotName == "onFilter") {
        connect(action, &QAction::triggered, this, &MainWindow::onFilter);
    }else if (slotName == "onAbout") {
        connect(action, &QAction::triggered, this, &MainWindow::onAbout);
    }
}

QImage* MainWindow::getImageFromFile(QString title)
{
    // 创建格式过滤器
    QString filter = "图片 (";
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    for (int i = 0; i < formats.size(); ++i) {
        filter += "*." + QString(formats[i]) + " ";
    }
    filter += ");;";

    // 常用图片格式过滤器
    filter += "JPEG (*.jpg *.jpeg);;"
              "PNG (*.png);;"
              "BMP (*.bmp);;"
              "GIF (*.gif);;"
              "TIFF (*.tif *.tiff);;"
              "所有文件 (*.*)";
    QString path = QFileDialog::getOpenFileName(this,title,".",filter);
    if(path.isEmpty()||!QFile::exists(path)){
        QMessageBox::warning(this,"warning",QString("文件打开失败！"));
        return nullptr;
    }
    QImage* image = new QImage;
    if(!image->load(path))
    {
        QMessageBox::warning(this,"warning",QString("路径：%1文件打开失败！").arg(path));
        return nullptr;
    }
    QMessageBox::information(this,"图像加载成功！",QString("长：%1\n宽：%2\n深度：%3位\t\t")
                                                         .arg(image->size().rheight())
                                                         .arg(image->size().rwidth())
                                                         .arg(image->depth()));
    return image;
}

// 槽函数实现
void MainWindow::onNewFile() {
    QMessageBox::information(this, "新建文件", "新建文件功能");
}

void MainWindow::onOpenFile() {
    auto image = getImageFromFile("选择要打开的文件");
    if(!image)
        return;
    ResizableItem* pixmapItem = new ResizableItem;
    pixmapItem->setPixmap(QPixmap::fromImage(*image));
    // 添加到场景
    addItemToScene(pixmapItem);
}

// 将图片添加到场景
void MainWindow::addItemToScene(ResizableItem* item)
{
    if(!item)
        return;
    // 设置物品属性
    item->setFlag(QGraphicsItem::ItemIsMovable, true); // 可移动
    item->setFlag(QGraphicsItem::ItemIsSelectable, true); // 可选
    item->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true); // 启用位置变化通知
    item->setAcceptHoverEvents(true); // 鼠标悬停事件

    // 设置位置（在已存在item的基础上偏移）
    int offset = m_items.size() * 20;  // 每次偏移20像素
    item->setPos(offset, offset);

    // 添加到场景
    m_graphics_scene->addItem(item);

    // 保存物品信息
    Item itemInfo;
    itemInfo.pixmapItem = item;
    itemInfo.offset = QPointF(offset, offset);
    itemInfo.zValue = m_current_z_value++;

    m_items[item] = itemInfo;

    // 连接信号槽（图片双击事件）
    connect(item, &ResizableItem::itemDoubleClicked, this, [this](ResizableItem *target){
        if(target->getItemType()==ItemType::Type_Image){
            auto image = getImageFromFile("选择要更换的图片");
            target->setPixmap(QPixmap::fromImage(*image));
        }
        else{
            bool ok;
            // 弹出标准输入对话框
            QString newText = QInputDialog::getText(this, "编辑文本",
                                                    "请输入新内容:",
                                                    QLineEdit::Normal,
                                                    target->getText(), &ok);
            if (ok && !newText.isEmpty()) {
                target->setText(newText);
            }
        }
        }
    );

    // 连接删除请求信号
    connect(item, &ResizableItem::itemDeleteRequested, this, [this](ResizableItem *target){
        // 从选中集合中移除
        m_selected_items.remove(target);

        // 从物品映射中移除
        m_items.remove(target);

        // 从场景中移除
        m_graphics_scene->removeItem(target);

        // 删除对象
        delete target;
    });

    // 选中新添加的物品
    selectItem(item);
}

void MainWindow::selectItem(ResizableItem* item)
{
    deselectAll();

    item->setSelected(true);
    m_selected_items.insert(item);

    // 高亮显示选中项
    // item->setGraphicsEffect(new QGraphicsDropShadowEffect());
}

void MainWindow::deselectAll()
{
    for (ResizableItem* item : m_selected_items) {
        item->setSelected(false);
        // item->setGraphicsEffect(nullptr);
    }
    m_selected_items.clear();
}

void MainWindow::onSaveFile()
{
    save("保存");
}

void MainWindow::onSaveAsFile()
{
    save("另存为");
}

void MainWindow::save(QString title)
{
    // 临时保存背景并移除
    QBrush oldBackground = m_graphics_scene->backgroundBrush();
    m_graphics_scene->setBackgroundBrush(Qt::NoBrush);

    // 获取当前视口的可见区域（在场景坐标系中）
    QRectF visibleRect = m_graphics_view->mapToScene(
                                            m_graphics_view->viewport()->rect()).boundingRect();

    // 创建QImage（使用可见区域的大小）
    QImage image(visibleRect.size().toSize(), QImage::Format_ARGB32);
    image.fill(Qt::transparent);  // 设置透明背景

    // 创建QPainter并渲染可见区域
    QPainter painter(&image);
    m_graphics_scene->render(&painter,
                             QRectF(),  // 目标矩形（整个image）
                             visibleRect);  // 源矩形（只渲染可见区域）
    painter.end();

    // 恢复背景
    m_graphics_scene->setBackgroundBrush(oldBackground);

    // 创建格式过滤器
    QString filter = "JPEG (*.jpg *.jpeg);;";
    filter += "PNG (*.png);;"
              "BMP (*.bmp);;"
              "GIF (*.gif);;"
              "TIFF (*.tif *.tiff);;"
              "所有文件 (*.*)";

    // 保存
    QString path = QFileDialog::getSaveFileName(this, title, "/untitled", filter);
    if (!path.isEmpty()) {
        image.save(path);
    }
}
void MainWindow::onExit()
{
    this->close();
}
void MainWindow::onUndo(){};
void MainWindow::onRedo(){};
void MainWindow::onZoomIn()
{
    m_view_scale*=1.2;
    m_graphics_view->scale(1.2,1.2);
}
void MainWindow::onZoomOut()
{
    m_view_scale*=0.8;
    m_graphics_view->scale(0.8,0.8);
}
void MainWindow::onCopy(){};
void MainWindow::onPaste(){};
void MainWindow::onInsertPicture()
{
    auto image = getImageFromFile("选择要插入的图片");
    if(!image)
        return;
    ResizableItem* pixmapItem = new ResizableItem;
    pixmapItem->setPixmap(QPixmap::fromImage(*image));
    // 添加到场景
    addItemToScene(pixmapItem);
}
void MainWindow::onInsertText()
{
    ResizableItem* text = new ResizableItem;
    text->setText("双击以输入文本");
    addItemToScene(text);
}
void MainWindow::onCutting(){};
void MainWindow::onFilter(){};
void MainWindow::onAbout()
{
    QMessageBox::about(this,"关于",QString("<h3>表情包制作器 v1.0</h3>"
            "<p>开发者：@l-library</p>"
            "<p>项目地址：<a href='https://github.com/l-library/MemeGenerator'>https://github.com/l-library/MemeGenerator</a></p>"));
}

// 备用默认配置
void MainWindow::setupDefaultMenuConfig() {
    MenuConfig fileMenu;
    fileMenu.title = "文件(&F)";
    fileMenu.name = "menu_file";
    m_menuConfigs.append(fileMenu);
    fileMenu.title = "插入(&I)";
    fileMenu.name = "menu_insert";
    m_menuConfigs.append(fileMenu);
    fileMenu.title = "工具(&T)";
    fileMenu.name = "menu_tool";
    m_menuConfigs.append(fileMenu);
    fileMenu.title = "效果(&E)";
    fileMenu.name = "menu_effect";
    m_menuConfigs.append(fileMenu);
    fileMenu.title = "帮助(&H)";
    fileMenu.name = "menu_help";
    m_menuConfigs.append(fileMenu);
    // 显示错误信息
    QMessageBox::critical(this,"错误","加载菜单栏配置错误！");
}


MainWindow::~MainWindow()
{
    delete ui;
}
