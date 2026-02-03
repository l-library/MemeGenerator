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
    m_grid_layout->setRowStretch(0, 20);  // 第一行占20%
    m_grid_layout->setRowStretch(1, 60);  // 第二行占60%
    m_grid_layout->setRowStretch(2, 20);  // 第三行占20%
    m_grid_layout->setColumnStretch(0, 10);  // 第一列占20%
    m_grid_layout->setColumnStretch(1, 60);  // 第二列占60%
    m_grid_layout->setColumnStretch(2, 10);  // 第三列占20%

    setCentralWidget(new QWidget(this));
    centralWidget()->setLayout(m_grid_layout);

    // 初始化图像视图
    initGraphicsView();
}

void MainWindow::initMenuBar() {
    createMenuFromConfig();
}

void MainWindow::initGraphicsView()
{
    // 图像视图初始化
    m_graphics_scene = new QGraphicsScene(this);
    m_graphics_view = new QGraphicsView(this);
    m_graphics_view->setScene(m_graphics_scene);
    // 添加view到布局
    m_grid_layout->addWidget(m_graphics_view,1,1);
    // 设置视图属性
    m_graphics_view->setRenderHint(QPainter::Antialiasing); // 开启抗锯齿
    m_graphics_view->setRenderHint(QPainter::SmoothPixmapTransform); // 光滑像素
    // m_graphics_view->setDragMode(QGraphicsView::RubberBandDrag); // 局部框选开启
    m_graphics_view->setInteractive(true); // 开启交互
    m_graphics_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse); // 设置锚点
    // 设置场景背景
    m_graphics_scene->setBackgroundBrush(QBrush(Qt::lightGray));
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

// 槽函数实现
void MainWindow::onNewFile() {
    QMessageBox::information(this, "新建文件", "新建文件功能");
}

void MainWindow::onOpenFile() {
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
    QString path = QFileDialog::getOpenFileName(this,"选择打开的文件",".",filter);
    if(path.isEmpty()||!QFile::exists(path)){
        QMessageBox::warning(this,"warning",QString("文件打开失败！"));
        return;
    }
    QImage image;
    if(!image.load(path))
    {
        QMessageBox::warning(this,"warning",QString("路径：%1文件打开失败！").arg(path));
        return;
    }
    QMessageBox::information(this,"图像加载成功！",QString("长：%1\n宽：%2\n深度：%3位\t\t")
                                                        .arg(image.size().rheight())
                                                        .arg(image.size().rwidth())
                                                        .arg(image.depth()));

    // 添加到场景
    addImageToScene(image, path);
}

// 将图片添加到场景
void MainWindow::addImageToScene(const QImage& image, const QString& path)
{
    // 创建图片项
    QGraphicsPixmapItem* pixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(image));

    // 设置图片属性
    pixmapItem->setFlag(QGraphicsItem::ItemIsMovable, true); // 可移动
    pixmapItem->setFlag(QGraphicsItem::ItemIsSelectable, true); // 可选
    pixmapItem->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true); // 启用位置变化通知
    pixmapItem->setAcceptHoverEvents(true); // 鼠标悬停事件

    // 设置图片位置（在已存在图片基础上偏移）
    int offset = m_image_items.size() * 20;  // 每次偏移20像素
    pixmapItem->setPos(offset, offset);

    // 添加到场景
    m_graphics_scene->addItem(pixmapItem);

    // 保存图片信息
    ImageItem imgInfo;
    imgInfo.pixmapItem = pixmapItem;
    imgInfo.path = path;
    imgInfo.offset = QPointF(offset, offset);
    imgInfo.zValue = m_current_z_value++;

    m_image_items[pixmapItem] = imgInfo;

    // 选中新添加的图片
    selectImageItem(pixmapItem);
}

void MainWindow::selectImageItem(QGraphicsPixmapItem* item)
{
    deselectAll();

    item->setSelected(true);
    m_selected_items.insert(item);

    // 高亮显示选中项
    item->setGraphicsEffect(new QGraphicsDropShadowEffect());
}

void MainWindow::deselectAll()
{
    for (QGraphicsPixmapItem* item : m_selected_items) {
        item->setSelected(false);
        item->setGraphicsEffect(nullptr);
    }
    m_selected_items.clear();
}

void MainWindow::onSaveFile(){};
void MainWindow::onSaveAsFile(){};
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
void MainWindow::onInsertPicture(){};
void MainWindow::onInsertText(){};
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
