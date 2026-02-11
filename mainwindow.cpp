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
    ,m_view_scale(1.0), m_editMode(NormalMode), m_canvasItem(nullptr)
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
    m_graphics_view->setBackgroundBrush(QBrush(Qt::lightGray));
    // 设置场景背景
    // m_graphics_scene->setForegroundBrush(QBrush(Qt::white));

    // 创建默认画布
    createDefaultCanvas();
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

    // 画布编辑模式切换按钮
    QPushButton* canvasEditButton = new QPushButton(this);
    canvasEditButton->setText("画布编辑");
    canvasEditButton->setStatusTip("切换画布编辑模式 (Ctrl+E)");
    canvasEditButton->setIcon(QIcon(":/icons/canvas.png"));
    canvasEditButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    canvasEditButton->setCheckable(true);
    canvasEditButton->setChecked(false);
    connect(canvasEditButton, &QPushButton::toggled, [this](bool checked) {
        setEditMode(checked ? CanvasEditMode : NormalMode);
    });
    canvasEditButton->setShortcut(QKeySequence("Ctrl+E"));
    m_grid_layout->addWidget(canvasEditButton,1,2);

    // 插入图片按钮
    QPushButton* insert_picture_btn = new QPushButton(this);
    insert_picture_btn->setText("插入图片");
    insert_picture_btn->setStatusTip("从文件中选择图片插入(Ctrl+I)");
    insert_picture_btn->setIcon(QIcon(":/icons/insertpicture.png"));
    insert_picture_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(insert_picture_btn, &QPushButton::pressed, [=]() {
        onInsertPicture();
    });
    m_grid_layout->addWidget(insert_picture_btn,2,2);

    // 插入文本按钮
    QPushButton* insert_text_btn = new QPushButton(this);
    insert_text_btn->setText("插入文本");
    insert_text_btn->setStatusTip("插入一个文本框(Ctrl+T)");
    insert_text_btn->setIcon(QIcon(":/icons/inserttext.png"));
    insert_text_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(insert_text_btn, &QPushButton::pressed, [=]() {
        onInsertText();
    });
    m_grid_layout->addWidget(insert_text_btn,3,2);

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
    }else if (slotName == "onToggleCanvasEditMode") {
        connect(action, &QAction::triggered, this, &MainWindow::onToggleCanvasEditMode);
    }else if (slotName == "onSetCanvasSize") {
        connect(action, &QAction::triggered, this, &MainWindow::onSetCanvasSize);
    }else if (slotName == "onResetCanvas") {
        connect(action, &QAction::triggered, this, &MainWindow::onResetCanvas);
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
    // 清空现有项目
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        ResizableItem* item = it.key();
        if (item != m_canvasItem) {
            m_graphics_scene->removeItem(item);
            delete item;
        }
    }
    m_items.clear();
    m_selected_items.clear();
    m_current_z_value = 0;

    // 重置画布到默认状态
    if (m_canvasItem) {
        onResetCanvas();
    } else {
        createDefaultCanvas();
    }

    // 重置编辑模式
    setEditMode(NormalMode);

    ui->statusbar->showMessage("新建文件完成");
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

    // 如果是画布，特殊处理
    if (item->isCanvas()) {
        m_canvasItem = item;
        item->setZValue(-1000);  // 确保在底层

        // 根据当前编辑模式设置画布属性
        bool canvasEditable = (m_editMode == CanvasEditMode);
        item->setFlag(QGraphicsItem::ItemIsMovable, canvasEditable);
        item->setFlag(QGraphicsItem::ItemIsSelectable, canvasEditable);
    } else {
        // 根据当前编辑模式设置项目属性
        if (m_editMode == CanvasEditMode) {
            // 非画布项目在画布编辑模式下不可编辑
            item->setFlag(QGraphicsItem::ItemIsMovable, false);
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        } else {
            item->setFlag(QGraphicsItem::ItemIsMovable, true);
            item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        }
    }

    item->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true); // 启用位置变化通知
    item->setAcceptHoverEvents(true); // 鼠标悬停事件

    // 设置位置（在已存在item的基础上偏移）
    int offset = m_items.size() * 20;  // 每次偏移20像素
    item->setPos(offset, offset);

    // 添加到场景
    m_graphics_scene->addItem(item);

    // 如果不是画布，保存物品信息到映射表
    if (!item->isCanvas()) {
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
    } else {
        // 如果是画布，连接画布双击信号
        connect(item, &ResizableItem::itemDoubleClicked,
                this, &MainWindow::onCanvasDoubleClicked);
    }
}

void MainWindow::selectItem(ResizableItem* item)
{
    if (!item || (item->isCanvas() && m_editMode != CanvasEditMode)) {
        return;  // 画布在非编辑模式下不可选中
    }

    deselectAll();

    item->setSelected(true);
    m_selected_items.insert(item);

    // 高亮显示选中项
    // item->setGraphicsEffect(new QGraphicsDropShadowEffect());
}

void MainWindow::deselectAll()
{
    for (ResizableItem* item : m_selected_items) {
        if (item != m_canvasItem || m_editMode == CanvasEditMode) {
            item->setSelected(false);
            // item->setGraphicsEffect(nullptr);
        }
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
    // 获取导出区域
    QRectF exportRect = getCanvasExportRect();
    if (exportRect.isEmpty()) {
        QMessageBox::warning(this, "错误", "无法确定导出区域");
        return;
    }

    // 临时保存背景并移除（实现透明背景导出）
    QBrush oldBackground = m_graphics_scene->backgroundBrush();
    QBrush oldForeground = m_graphics_scene->foregroundBrush();
    m_graphics_scene->setBackgroundBrush(Qt::NoBrush);
    m_graphics_scene->setForegroundBrush(Qt::NoBrush);

    // 临时隐藏画布选择边框（如果选中）
    bool canvasWasSelected = m_canvasItem && m_canvasItem->isSelected();
    if (m_canvasItem && canvasWasSelected) {
        m_canvasItem->setSelected(false);
    }

    // 创建QImage（使用画布大小）
    QSize imageSize = exportRect.size().toSize();
    QImage image(imageSize, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    // 创建QPainter并渲染画布区域
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 渲染场景到图像，只渲染画布区域
    m_graphics_scene->render(&painter,
                             QRectF(0, 0, imageSize.width(), imageSize.height()),  // 目标矩形
                             exportRect);  // 源矩形（画布区域）
    painter.end();

    // 恢复画布选择状态
    if (m_canvasItem && canvasWasSelected) {
        m_canvasItem->setSelected(true);
    }

    // 恢复背景
    m_graphics_scene->setBackgroundBrush(oldBackground);
    m_graphics_scene->setForegroundBrush(oldForeground);

    // 创建格式过滤器
    QString filter = "PNG (*.png);;";  // PNG支持透明度
    filter += "JPEG (*.jpg *.jpeg);;"
              "BMP (*.bmp);;"
              "GIF (*.gif);;"
              "TIFF (*.tif *.tiff);;"
              "所有文件 (*.*)";

    // 保存文件
    QString path = QFileDialog::getSaveFileName(this, title, "/untitled", filter);
    if (!path.isEmpty()) {
        // 根据文件扩展名选择保存格式
        if (path.endsWith(".png", Qt::CaseInsensitive)) {
            image.save(path, "PNG");
        } else if (path.endsWith(".jpg", Qt::CaseInsensitive) ||
                   path.endsWith(".jpeg", Qt::CaseInsensitive)) {
            // JPEG不支持透明度，需要填充白色背景
            QImage jpegImage(imageSize, QImage::Format_RGB32);
            jpegImage.fill(Qt::white);
            QPainter jpegPainter(&jpegImage);
            jpegPainter.drawImage(0, 0, image);
            jpegPainter.end();
            jpegImage.save(path, "JPEG");
        } else {
            image.save(path);
        }
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

// 画布管理方法实现
void MainWindow::createDefaultCanvas() {
    if (m_canvasItem) {
        return;  // 画布已存在
    }

    m_canvasItem = new ResizableItem();
    m_canvasItem->setCanvas(QSizeF(800, 600), Qt::white);
    m_canvasItem->setPos(100, 100);  // 留出边距
    m_canvasItem->setZValue(-1000);  // 极低的z值，确保在底层
    m_canvasItem->setFlag(QGraphicsItem::ItemIsMovable, false);
    m_canvasItem->setFlag(QGraphicsItem::ItemIsSelectable, false);

    m_graphics_scene->addItem(m_canvasItem);

    // 连接画布信号
    connect(m_canvasItem, &ResizableItem::itemDoubleClicked,
            this, &MainWindow::onCanvasDoubleClicked);
}

void MainWindow::setEditMode(EditMode mode) {
    m_editMode = mode;

    // 更新画布可编辑性
    bool canvasEditable = (mode == CanvasEditMode);
    if (m_canvasItem) {
        m_canvasItem->setFlag(QGraphicsItem::ItemIsMovable, canvasEditable);
        m_canvasItem->setFlag(QGraphicsItem::ItemIsSelectable, canvasEditable);
    }

    // 更新其他项目的可编辑性
    updateItemEditability();

    // 更新UI状态
    updateEditModeUI();
}

void MainWindow::updateItemEditability() {
    for (auto it = m_items.begin(); it != m_items.end(); ++it) {
        ResizableItem* item = it.key();
        if (item != m_canvasItem) {
            bool editable = (m_editMode == NormalMode);
            item->setFlag(QGraphicsItem::ItemIsMovable, editable);
            item->setFlag(QGraphicsItem::ItemIsSelectable, editable);
        }
    }
}

QRectF MainWindow::getCanvasExportRect() const {
    if (!m_canvasItem) {
        // 如果没有画布，使用可见区域（向后兼容）
        return m_graphics_view->mapToScene(
                   m_graphics_view->viewport()->rect()).boundingRect();
    }

    // 获取画布的边界矩形（场景坐标系）
    QRectF canvasRect = m_canvasItem->boundingRect();
    QPointF canvasPos = m_canvasItem->pos();
    QRectF exportRect(canvasPos, canvasRect.size());

    // 确保导出区域有效
    if (exportRect.width() <= 0 || exportRect.height() <= 0) {
        exportRect = QRectF(0, 0, 800, 600);  // 默认大小
    }

    return exportRect;
}

void MainWindow::updateEditModeUI() {
    // 更新菜单项选中状态
    QAction* canvasEditAction = m_actionMap.value("action_canvas_edit");
    if (canvasEditAction) {
        canvasEditAction->setChecked(m_editMode == CanvasEditMode);
    }

    // 更新状态栏信息
    if (m_editMode == CanvasEditMode) {
        ui->statusbar->showMessage("画布编辑模式 - 可以调整画布大小和位置");
    } else {
        ui->statusbar->showMessage("普通编辑模式 - 可以编辑图片和文字");
    }
}

// 画布相关槽函数实现
void MainWindow::onToggleCanvasEditMode() {
    if (m_editMode == NormalMode) {
        setEditMode(CanvasEditMode);
    } else {
        setEditMode(NormalMode);
    }
}

void MainWindow::onCanvasDoubleClicked(ResizableItem* canvas) {
    if (canvas == m_canvasItem && m_editMode == CanvasEditMode) {
        // 在画布编辑模式下双击画布可以设置大小
        onSetCanvasSize();
    }
}

void MainWindow::onSetCanvasSize() {
    if (!m_canvasItem) return;

    QSizeF currentSize = m_canvasItem->getCanvasSize();
    bool ok;
    double width = QInputDialog::getDouble(this, "设置画布宽度",
                                          "宽度 (像素):",
                                          currentSize.width(),
                                          50, 5000, 1, &ok);
    if (!ok) return;

    double height = QInputDialog::getDouble(this, "设置画布高度",
                                           "高度 (像素):",
                                           currentSize.height(),
                                           50, 5000, 1, &ok);
    if (!ok) return;

    m_canvasItem->setCanvas(QSizeF(width, height), Qt::white);
}

void MainWindow::onResetCanvas() {
    if (m_canvasItem) {
        m_canvasItem->setCanvas(QSizeF(800, 600), Qt::white);
        m_canvasItem->setPos(100, 100);
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}
