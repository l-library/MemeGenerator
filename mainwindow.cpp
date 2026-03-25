#include "mainwindow.h"
#include "menuconfig.h"
#include "commands.h"
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
#include <QClipboard>
#include <QMimeData>
#include "resizableitem.h"
#include "imagecropperdialog.h"
#include "DimOutsideCanvasEffect.h"
#include "filterdialog.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
      m_grid_layout(nullptr), m_graphics_scene(nullptr), m_graphics_view(nullptr), m_current_z_value(0), m_view_scale(1.0), m_editMode(NormalMode), m_canvasItem(nullptr), m_canvasOffset(0, 0), m_isModified(false), m_undoStack(new QUndoStack(this)), m_isUndoRedoing(false)
{
    ui->setupUi(this);

    // 默认窗口最大化
    this->setWindowState(Qt::WindowMaximized);

    // 图标和名称初始化
    this->setWindowIcon(QIcon(":/icons/MemeGenerator.png"));
    this->setWindowTitle("表情包生成器");

    // 加载配置文件
    loadMenuConfig();
    // 菜单栏初始化
    initMenuBar();

    // 创建网格布局
    m_grid_layout = new QGridLayout(this);
    // 设置行和列的比例
    for (int i = 0; i < 10; ++i)
    {
        m_grid_layout->setRowStretch(i, 10); // 将行均分为10份
    }
    m_grid_layout->setColumnStretch(0, 20); // 第一列占20%
    m_grid_layout->setColumnStretch(1, 60); // 第二列占60%
    m_grid_layout->setColumnStretch(2, 20); // 第三列占20%
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
        "   border-top: 1px solid #B0B0B0;" /* 顶部添加 1px 的灰色实线 */
        "   background-color: #F5F5F5;"     /* 背景色稍微改浅灰，与白色视图区分 */
        "}");
    statusBar()->showMessage("准备就绪");
    // 画布大小状态显示
    QSizeF size = m_canvasItem->getCanvasSize();
    QString message = QString("当前画布大小: %1 * %2  ").arg(size.width()).arg(size.height());
    m_canvasSizeLabel = new QLabel(message, this);
    // 将初始化的标签添加到底部状态栏上
    statusBar()->addPermanentWidget(m_canvasSizeLabel);

    // 初始化右侧按键
    initButton();

    // 初始窗口修改状态为false
    setWindowModified(false);
}

void MainWindow::initMenuBar()
{
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
    m_grid_layout->addWidget(m_graphics_view, 0, 0, 10, 2);
    // 设置视图属性
    m_graphics_view->setRenderHint(QPainter::Antialiasing);                    // 开启抗锯齿
    m_graphics_view->setRenderHint(QPainter::SmoothPixmapTransform);           // 光滑像素
    m_graphics_view->setDragMode(QGraphicsView::RubberBandDrag);               // 局部框选开启
    m_graphics_view->setInteractive(true);                                     // 开启交互
    m_graphics_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse); // 设置锚点
    m_graphics_view->setCacheMode(QGraphicsView::CacheNone);                   // 禁用缓存避免拖拽痕迹
    m_graphics_view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate); // 拖拽时全量重绘
    m_graphics_view->setBackgroundBrush(QBrush(Qt::lightGray));
    // 设置场景背景
    // m_graphics_scene->setForegroundBrush(QBrush(Qt::white));

    // 创建默认画布
    createDefaultCanvas();
}

void MainWindow::initButton()
{
    // 放大缩小按钮
    auto zoom_in_out_layout = new QGridLayout(this);
    zoom_in_out_layout->setColumnStretch(0,50);
    zoom_in_out_layout->setColumnStretch(1,50);
    m_grid_layout->addLayout(zoom_in_out_layout, 0, 2);
    QPushButton *zoomIn = new QPushButton(this);
    zoomIn->setText("放大");
    zoomIn->setStatusTip("放大当前场景");
    zoomIn->setIcon(QIcon(":/icons/zoomin.png"));
    zoomIn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(zoomIn, &QPushButton::pressed, [=]()
            { onZoomIn(); });
    QPushButton *zoomOut = new QPushButton(this);
    zoomOut->setText("缩小");
    zoomOut->setStatusTip("缩小当前场景");
    zoomOut->setIcon(QIcon(":/icons/zoomout.png"));
    zoomOut->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(zoomOut, &QPushButton::pressed, [=]()
            { onZoomOut(); });
    zoom_in_out_layout->addWidget(zoomOut,0,0);
    zoom_in_out_layout->addWidget(zoomIn,0,1);

    // 画布编辑模式切换按钮
    QPushButton *canvasEditButton = new QPushButton(this);
    canvasEditButton->setText("画布编辑");
    canvasEditButton->setStatusTip("切换画布编辑模式 (Ctrl+E)");
    canvasEditButton->setIcon(QIcon(":/icons/canvas.png"));
    canvasEditButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    canvasEditButton->setCheckable(true);
    canvasEditButton->setChecked(false);
    connect(canvasEditButton, &QPushButton::toggled, [this](bool checked)
            { setEditMode(checked ? CanvasEditMode : NormalMode); });
    canvasEditButton->setShortcut(QKeySequence("Ctrl+E"));
    m_grid_layout->addWidget(canvasEditButton, 1, 2);

    // 打开文件按钮
    QPushButton *file_open_btn = new QPushButton(this);
    file_open_btn->setText("打开文件");
    file_open_btn->setStatusTip("选择一个图片文件打开，自动调整画布(Ctrl+O)");
    file_open_btn->setIcon(QIcon(":/icons/fileopen.png"));
    file_open_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(file_open_btn, &QPushButton::pressed, [=]()
            { onOpenFile(); });
    m_grid_layout->addWidget(file_open_btn, 2, 2);

    // 插入图片按钮
    QPushButton *insert_picture_btn = new QPushButton(this);
    insert_picture_btn->setText("插入图片");
    insert_picture_btn->setStatusTip("从文件中选择图片插入(Ctrl+I)");
    insert_picture_btn->setIcon(QIcon(":/icons/insertpicture.png"));
    insert_picture_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(insert_picture_btn, &QPushButton::pressed, [=]()
            { onInsertPicture(); });
    m_grid_layout->addWidget(insert_picture_btn, 3, 2);

    // 插入文本按钮
    QPushButton *insert_text_btn = new QPushButton(this);
    insert_text_btn->setText("插入文本");
    insert_text_btn->setStatusTip("插入一个文本框(Ctrl+T)");
    insert_text_btn->setIcon(QIcon(":/icons/inserttext.png"));
    insert_text_btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(insert_text_btn, &QPushButton::pressed, [=]()
            { onInsertText(); });
    m_grid_layout->addWidget(insert_text_btn, 4, 2);

    // 添加滤镜按钮
    QPushButton *insert_filter = new QPushButton(this);
    insert_filter->setText("添加滤镜");
    insert_filter->setStatusTip("为场景添加一个加入滤镜的图层");
    insert_filter->setIcon(QIcon(":/icons/filtereffect.png"));
    insert_filter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(insert_filter, &QPushButton::pressed, [=]()
            { onFilter(); });
    m_grid_layout->addWidget(insert_filter, 5, 2);

    // 导出按钮
    QPushButton *save_file_button = new QPushButton(this);
    save_file_button->setText("导出");
    save_file_button->setStatusTip("将当前视图导出为图片(Ctrl+S)");
    save_file_button->setIcon(QIcon(":/icons/export.png"));
    save_file_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(save_file_button, &QPushButton::pressed, [=]()
            { onSaveAsFile(); });
    // 设置按钮的文字颜色和背景色
    save_file_button->setStyleSheet("color: black; background-color: #FFFD55;");
    m_grid_layout->addWidget(save_file_button, 8, 2, 10, 2);
}

void MainWindow::loadMenuConfig()
{
    // 从资源文件加载配置
    QString configPath = ":/config/MenuConfig.json";
    m_menuConfigs = MenuConfigManager::loadFromJson(configPath);

    if (m_menuConfigs.isEmpty())
    {
        // 如果配置文件加载失败，使用默认配置
        setupDefaultMenuConfig();
    }
}

void MainWindow::createMenuFromConfig()
{
    QMenuBar *menuBar = this->menuBar();

    for (long long i = 0; i < m_menuConfigs.size(); ++i)
    {
        const MenuConfig &menuConfig = m_menuConfigs[i];
        QMenu *menu = new QMenu(menuConfig.title, this);

        for (const MenuItemConfig &itemConfig : menuConfig.items)
        {
            if (itemConfig.separator)
            {
                menu->addSeparator();
                continue;
            }

            // 创建QAction
            QAction *action = new QAction(this);

            // 设置文本
            action->setText(itemConfig.text);

            // 设置图标
            if (!itemConfig.iconPath.isEmpty())
            {
                action->setIcon(QIcon(itemConfig.iconPath));
            }

            // 设置快捷键
            if (!itemConfig.standardKey.isEmpty())
            {
                QKeySequence::StandardKey standardKey =
                    MenuConfigManager::stringToStandardKey(itemConfig.standardKey);
                if (standardKey != QKeySequence::UnknownKey)
                {
                    action->setShortcut(standardKey);
                }
            }
            else if (!itemConfig.shortcut.isEmpty())
            {
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
            if (!itemConfig.id.isEmpty())
            {
                m_actionMap[itemConfig.id] = action;

                // 连接信号槽
                if (!itemConfig.slot.isEmpty())
                {
                    connectActionToSlot(itemConfig.slot, action);
                }
            }
        }

        // 添加到菜单栏
        menuBar->addMenu(menu);
    }
}

void MainWindow::connectActionToSlot(const QString &slotName, QAction *action)
{
    if (slotName == "onNewFile")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onNewFile);
    }
    else if (slotName == "onOpenFile")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onOpenFile);
    }
    else if (slotName == "onSaveFile")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onSaveFile);
    }
    else if (slotName == "onSaveAsFile")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onSaveAsFile);
    }
    else if (slotName == "onExit")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onExit);
    }
    else if (slotName == "onUndo")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onUndo);
    }
    else if (slotName == "onRedo")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onRedo);
    }
    else if (slotName == "onZoomIn")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onZoomIn);
    }
    else if (slotName == "onZoomOut")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onZoomOut);
    }
    else if (slotName == "onCopy")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onCopy);
    }
    else if (slotName == "onPaste")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onPaste);
    }
    else if (slotName == "onInsertPicture")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onInsertPicture);
    }
    else if (slotName == "onInsertText")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onInsertText);
    }
    else if (slotName == "onCutting")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onCutting);
    }
    else if (slotName == "onFilter")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onFilter);
    }
    else if (slotName == "onAbout")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onAbout);
    }
    else if (slotName == "onToggleCanvasEditMode")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onToggleCanvasEditMode);
    }
    else if (slotName == "onSetCanvasSize")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onSetCanvasSize);
    }
    else if (slotName == "onResetCanvas")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onResetCanvas);
    }
    else if (slotName == "onDeleteSelected")
    {
        connect(action, &QAction::triggered, this, &MainWindow::onDeleteSelected);
    }
}

QImage MainWindow::getImageFromFile(QString title)
{
    // 创建格式过滤器
    QString filter = "图片 (";
    QList<QByteArray> formats = QImageReader::supportedImageFormats();
    for (int i = 0; i < formats.size(); ++i)
    {
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
    QString path = QFileDialog::getOpenFileName(this, title, ".", filter);
    if (path.isEmpty() || !QFile::exists(path))
    {
        QMessageBox::warning(this, "warning", QString("文件打开失败！"));
        return QImage();
    }
    QImage image;
    if (!image.load(path))
    {
        QMessageBox::warning(this, "warning", QString("路径：%1文件打开失败！").arg(path));
        return QImage();
    }
    QMessageBox::information(this, "图像加载成功！", QString("长：%1\n宽：%2\n深度：%3位\t\t").arg(image.size().rheight()).arg(image.size().rwidth()).arg(image.depth()));
    return image;
}

QImage MainWindow::getSceneImage()
{
    QRectF exportRect = getCanvasExportRect();
    if (exportRect.isEmpty())
    {
        QMessageBox::warning(this, "错误", "无法确定导出区域");
        return QImage();
    }

    QBrush oldBackground = m_graphics_scene->backgroundBrush();
    QBrush oldForeground = m_graphics_scene->foregroundBrush();
    m_graphics_scene->setBackgroundBrush(Qt::NoBrush);
    m_graphics_scene->setForegroundBrush(Qt::NoBrush);

    bool canvasWasSelected = m_canvasItem && m_canvasItem->isSelected();
    if (m_canvasItem && canvasWasSelected)
    {
        m_canvasItem->setSelected(false);
    }

    QSize imageSize = exportRect.size().toSize();
    QImage image(imageSize, QImage::Format_ARGB32);
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    m_graphics_scene->render(&painter,
                             QRectF(0, 0, imageSize.width(), imageSize.height()),
                             exportRect);
    painter.end();

    if (m_canvasItem && canvasWasSelected)
    {
        m_canvasItem->setSelected(true);
    }

    m_graphics_scene->setBackgroundBrush(oldBackground);
    m_graphics_scene->setForegroundBrush(oldForeground);

    return image;
}
// 槽函数实现
void MainWindow::onNewFile()
{
    // 检查是否需要保存当前修改
    if (!maybeSave())
        return;

    // 清空现有项目
    clearScene();

    // 重置画布到默认状态
    if (m_canvasItem)
    {
        onResetCanvas();
    }
    else
    {
        createDefaultCanvas();
    }

    // 重置编辑模式
    setEditMode(NormalMode);

    ui->statusbar->showMessage("新建文件完成");
    // resetModified();
}

void MainWindow::onOpenFile()
{
    // 检查是否需要保存当前修改
    if (!maybeSave())
        return;

    QImage image = getImageFromFile("选择要打开的文件");
    if (image.isNull())
        return;
    ResizableItem *pixmapItem = new ResizableItem;
    pixmapItem->setPixmap(QPixmap::fromImage(image));

    // 清理场景
    clearScene();

    // 设置画布大小
    // 删除旧画布
    m_selected_items.remove(m_canvasItem);      // 从选中集合中移除
    m_items.remove(m_canvasItem);               // 从物品映射中移除
    m_graphics_scene->removeItem(m_canvasItem); // 从场景中移除
    delete m_canvasItem;                        // 删除对象
    m_canvasItem = nullptr;
    // 创建新画布
    createCanvas(QSizeF(image.rect().width(), image.rect().height()), 0, 0, Qt::white);

    // 添加到场景
    addItemToScene(pixmapItem);

    // 更新画布大小信息
    onUpdateCanvasSizeLabel();
    // resetModified();
}

// 将图片添加到场景
void MainWindow::addItemToScene(ResizableItem *item)
{
    if (!item)
        return;

    if (item->isCanvas())
    {
        addItemToSceneDirectly(item);
    }
    else
    {
        pushCommand(new AddItemCommand(this, item));
    }
}

void MainWindow::selectItem(ResizableItem *item)
{
    if (!item || (item->isCanvas() && m_editMode != CanvasEditMode))
    {
        return; // 画布在非编辑模式下不可选中
    }

    deselectAll();

    item->setSelected(true);
    m_selected_items.insert(item);
}

void MainWindow::deselectAll()
{
    for (ResizableItem *item : m_selected_items)
    {
        if (item != m_canvasItem || m_editMode == CanvasEditMode)
        {
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
    if (exportRect.isEmpty())
    {
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
    if (m_canvasItem && canvasWasSelected)
    {
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
                             QRectF(0, 0, imageSize.width(), imageSize.height()), // 目标矩形
                             exportRect);                                         // 源矩形（画布区域）
    painter.end();

    // 恢复画布选择状态
    if (m_canvasItem && canvasWasSelected)
    {
        m_canvasItem->setSelected(true);
    }

    // 恢复背景
    m_graphics_scene->setBackgroundBrush(oldBackground);
    m_graphics_scene->setForegroundBrush(oldForeground);

    // 创建格式过滤器
    QString filter = "PNG (*.png);;"; // PNG支持透明度
    filter += "JPEG (*.jpg *.jpeg);;"
              "BMP (*.bmp);;"
              "GIF (*.gif);;"
              "TIFF (*.tif *.tiff);;"
              "所有文件 (*.*)";

    // 保存文件
    QString path = QFileDialog::getSaveFileName(this, title, "/untitled", filter);
    if (!path.isEmpty())
    {
        // 根据文件扩展名选择保存格式
        if (path.endsWith(".png", Qt::CaseInsensitive))
        {
            image.save(path, "PNG");
        }
        else if (path.endsWith(".jpg", Qt::CaseInsensitive) ||
                 path.endsWith(".jpeg", Qt::CaseInsensitive))
        {
            // JPEG不支持透明度，需要填充白色背景
            QImage jpegImage(imageSize, QImage::Format_RGB32);
            jpegImage.fill(Qt::white);
            QPainter jpegPainter(&jpegImage);
            jpegPainter.drawImage(0, 0, image);
            jpegPainter.end();
            jpegImage.save(path, "JPEG");
        }
        else
        {
            image.save(path);
        }
        // 保存成功，重置修改状态
        resetModified();
    }
}
void MainWindow::onExit()
{
    this->close();
}
void MainWindow::onUndo()
{
    if (m_undoStack && m_undoStack->canUndo())
    {
        m_undoStack->undo();
    }
}

void MainWindow::onRedo()
{
    if (m_undoStack && m_undoStack->canRedo())
    {
        m_undoStack->redo();
    }
}

void MainWindow::onZoomIn()
{
    m_view_scale *= 1.2;
    m_graphics_view->scale(1.2, 1.2);
}

void MainWindow::onZoomOut()
{
    m_view_scale *= 0.8;
    m_graphics_view->scale(0.8, 0.8);
}

void MainWindow::onCopy()
{
    QClipboard *clipboard = QApplication::clipboard(); // 获取单例
    if (m_selected_items.size() == 0)
        return;
    if (m_selected_items.size() > 1)
    {
        QMessageBox::warning(this, "警告", "只能选择一个对象复制");
        return;
    }
    ResizableItem *item = *m_selected_items.begin();
    if (item->getItemType() == ItemType::Type_Canvas)
        return;
    else if (item->getItemType() == ItemType::Type_Image)
    {
        clipboard->setPixmap(item->getPixmap());
        this->statusBar()->showMessage("复制成功！");
    }
    else if (item->getItemType() == ItemType::Type_Text)
    {
        clipboard->setText(item->getText());
        this->statusBar()->showMessage("复制成功！");
    }
    else
        return;
}

void MainWindow::onPaste()
{
    QClipboard *clipboard = QApplication::clipboard();
    const QMimeData *mime_data = clipboard->mimeData();
    if (mime_data->hasImage())
    {
        ResizableItem *item = new ResizableItem;
        QImage image = clipboard->image();
        item->setPixmap(QPixmap::fromImage(image));
        addItemToScene(item);
        this->statusBar()->showMessage("粘贴成功！");
    }
    else if (mime_data->hasText())
    {
        ResizableItem *item = new ResizableItem;
        QString text = clipboard->text();
        item->setText(text);
        addItemToScene(item);
        this->statusBar()->showMessage("粘贴成功！");
    }
    else
    {
        QMessageBox::warning(this, "警告", "不支持的粘贴类型！");
    }
}

void MainWindow::onDeleteSelected()
{
    // 获取场景中所有选中的项
    QList<QGraphicsItem*> selectedItems = m_graphics_scene->selectedItems();
    
    if (selectedItems.isEmpty())
    {
        this->statusBar()->showMessage("没有选中的项目可删除");
        return;
    }
    
    // 遍历所有选中的项，删除非画布项
    for (QGraphicsItem* graphicsItem : selectedItems)
    {
        ResizableItem* item = dynamic_cast<ResizableItem*>(graphicsItem);
        if (item && item != m_canvasItem)
        {
            // 使用删除命令来支持撤销/重做
            pushCommand(new DeleteItemCommand(this, item));
        }
    }
    
    this->statusBar()->showMessage(QString("删除了 %1 个项目").arg(selectedItems.size()));
}

void MainWindow::onInsertPicture()
{
    QImage image = getImageFromFile("选择要插入的图片");
    if (image.isNull())
        return;
    ResizableItem *pixmapItem = new ResizableItem;
    pixmapItem->setPixmap(QPixmap::fromImage(image));
    // 添加到场景
    addItemToScene(pixmapItem);
}
void MainWindow::onInsertText()
{
    ResizableItem *text = new ResizableItem;
    text->setText("双击以输入文本");
    addItemToScene(text);
}

void MainWindow::onCutting()
{
    if (m_selected_items.size() != 1)
    {
        QMessageBox::warning(this, "错误", "只能裁剪一个选中的图片");
        return;
    }
    if ((*m_selected_items.begin())->getItemType() != ItemType::Type_Image)
    {
        QMessageBox::warning(this, "错误", "无法裁剪非图片");
        return;
    }
    ResizableItem *target = *m_selected_items.begin();
    auto image = ImageCropperDialog::getCroppedImage(target->getPixmap(), 1024, 576, CropperShape::RECT);
    if (!image.isNull())
    {
        QPixmap oldPixmap = target->getPixmap();
        QPixmap newPixmap = image;
        pushCommand(new ChangePixmapCommand(this, target, oldPixmap, newPixmap));
    }
}

void MainWindow::onFilter()
{
    QImage sceneImage = getSceneImage();
    if (sceneImage.isNull())
    {
        return;
    }

    FilterDialog filterDialog(this);
    filterDialog.setOriginalImage(sceneImage);

    if (filterDialog.exec() == QDialog::Accepted)
    {
        QImage filteredImage = filterDialog.getFilteredCopy();
        if (!filteredImage.isNull())
        {
            ResizableItem *resultItem = new ResizableItem;
            resultItem->setPixmap(QPixmap::fromImage(filteredImage));
            resultItem->setPos(m_canvasItem->pos());
            addItemToScene(resultItem);

            QMessageBox::information(this, "滤镜应用", "滤镜已应用到新创建的图层中");
        }
    }
};
void MainWindow::onAbout()
{
    QMessageBox::about(this, "关于", QString("<h3>表情包制作器 v0.1</h3>"
                                             "<p>开发者：@l-library</p>"
                                             "<p>项目地址：<a href='https://github.com/l-library/MemeGenerator'>https://github.com/l-library/MemeGenerator</a></p>"));
}

void MainWindow::clearScene()
{
    // 移除并删除所有非画布项目
    for (ResizableItem *item : m_items.keys())
    {
        m_graphics_scene->removeItem(item);
        delete item;
    }
    m_items.clear();
    m_selected_items.clear();
    m_current_z_value = 0; // Z值重置，新插入项将从0开始
}

// 备用默认配置
void MainWindow::setupDefaultMenuConfig()
{
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
    QMessageBox::critical(this, "错误", "加载菜单栏配置错误！");
}

// 画布管理方法实现
void MainWindow::createDefaultCanvas()
{
    createCanvas(QSizeF(800, 600), 0, 0, Qt::white);
}

void MainWindow::createCanvas(QSizeF size, qreal pos_x, qreal pos_y, Qt::GlobalColor color)
{
    if (m_canvasItem)
    {
        return;
    }

    m_canvasItem = new ResizableItem();
    m_canvasItem->setCanvas(size, color);
    m_canvasItem->setPos(pos_x, pos_y);
    m_canvasOffset = QPointF(pos_x, pos_y);
    m_canvasItem->setZValue(-1000);
    m_canvasItem->setFlag(QGraphicsItem::ItemIsMovable, false);
    m_canvasItem->setFlag(QGraphicsItem::ItemIsSelectable, false);

    m_graphics_scene->addItem(m_canvasItem);

    connect(m_canvasItem, &ResizableItem::itemDoubleClicked,
            this, &MainWindow::onCanvasDoubleClicked);
    connect(m_canvasItem, &ResizableItem::sizeChanged,
            this, [this](ResizableItem *)
            {
                if (m_isUndoRedoing) return;
                onUpdateCanvasSizeLabel();
                markModified(); });
    connect(m_canvasItem, &ResizableItem::positionChanged,
            this, [this](ResizableItem *)
            {
                if (m_isUndoRedoing) return;
                m_canvasOffset = m_canvasItem->pos();
                markModified(); });
    connect(m_canvasItem, &ResizableItem::changeCanvasSize,
            this, [this]
            { onSetCanvasSize(); });
}

void MainWindow::setEditMode(EditMode mode)
{
    m_editMode = mode;

    // 更新画布可编辑性
    bool canvasEditable = (mode == CanvasEditMode);
    if (m_canvasItem)
    {
        m_canvasItem->setFlag(QGraphicsItem::ItemIsMovable, canvasEditable);
        m_canvasItem->setFlag(QGraphicsItem::ItemIsSelectable, canvasEditable);
    }

    // 更新其他项目的可编辑性
    updateItemEditability();

    // 更新UI状态
    updateEditModeUI();
}

void MainWindow::updateItemEditability()
{
    for (auto it = m_items.begin(); it != m_items.end(); ++it)
    {
        ResizableItem *item = it.key();
        if (item != m_canvasItem)
        {
            bool editable = (m_editMode == NormalMode);
            item->setFlag(QGraphicsItem::ItemIsMovable, editable);
            item->setFlag(QGraphicsItem::ItemIsSelectable, editable);
        }
    }
}

QRectF MainWindow::getCanvasExportRect() const
{
    if (!m_canvasItem)
    {
        // 如果没有画布，使用可见区域（向后兼容）
        return m_graphics_view->mapToScene(
                                  m_graphics_view->viewport()->rect())
            .boundingRect();
    }

    // 获取画布实际内容矩形（不包含手柄、边框等）
    QSizeF canvasSize = m_canvasItem->getCanvasSize(); // 内容尺寸
    QPointF canvasPos = m_canvasItem->pos();           // 左上角位置
    QRectF exportRect(canvasPos, canvasSize);

    // 防止无效尺寸
    if (exportRect.width() <= 0 || exportRect.height() <= 0)
    {
        exportRect = QRectF(0, 0, 800, 600);
    }
    return exportRect;
}

void MainWindow::updateEditModeUI()
{
    // 更新菜单项选中状态
    QAction *canvasEditAction = m_actionMap.value("action_canvas_edit");
    if (canvasEditAction)
    {
        canvasEditAction->setChecked(m_editMode == CanvasEditMode);
    }

    // 更新状态栏信息
    if (m_editMode == CanvasEditMode)
    {
        ui->statusbar->showMessage("画布编辑模式 - 可以调整画布大小和位置");
    }
    else
    {
        ui->statusbar->showMessage("普通编辑模式 - 可以编辑图片和文字");
    }
}

// 画布相关槽函数实现
void MainWindow::onToggleCanvasEditMode()
{
    if (m_editMode == NormalMode)
    {
        setEditMode(CanvasEditMode);
    }
    else
    {
        setEditMode(NormalMode);
    }
}

void MainWindow::onCanvasDoubleClicked(ResizableItem *canvas)
{
    if (canvas == m_canvasItem && m_editMode == CanvasEditMode)
    {
        // 在画布编辑模式下双击画布可以设置大小
        onSetCanvasSize();
    }
}

void MainWindow::onSetCanvasSize()
{
    if (!m_canvasItem)
        return;

    QSizeF currentSize = m_canvasItem->getCanvasSize();
    bool ok;
    double width = QInputDialog::getDouble(this, "设置画布宽度",
                                           "宽度 (像素):",
                                           currentSize.width(),
                                           50, 5000, 1, &ok);
    if (!ok)
        return;

    double height = QInputDialog::getDouble(this, "设置画布高度",
                                            "高度 (像素):",
                                            currentSize.height(),
                                            50, 5000, 1, &ok);
    if (!ok)
        return;

    QSizeF newSize(width, height);
    pushCommand(new CanvasResizeCommand(this, m_canvasItem, currentSize, newSize));
}

void MainWindow::onResetCanvas()
{
    if (m_canvasItem)
    {
        QSizeF oldSize = m_canvasItem->getCanvasSize();
        QPointF oldPos = m_canvasItem->pos();
        QSizeF newSize(800, 600);
        QPointF newPos(100, 100);

        QList<QUndoCommand *> commands;
        commands.append(new CanvasResizeCommand(this, m_canvasItem, oldSize, newSize));
        commands.append(new CanvasMoveCommand(this, m_canvasItem, oldPos, newPos));
        pushCommand(new CompositeCommand(tr("重置画布"), commands));
    }
}

void MainWindow::onUpdateCanvasSizeLabel()
{
    QSizeF size = m_canvasItem->getCanvasSize();
    QString message = QString("当前画布大小: %1 * %2  ").arg(size.width()).arg(size.height());
    m_canvasSizeLabel->setText(message);
}

bool MainWindow::maybeSave()
{
    if (!m_isModified)
        return true;

    QMessageBox::StandardButton ret = QMessageBox::warning(
        this, "表情包生成器",
        tr("场景已被修改。\n"
           "是否保存修改？"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (ret == QMessageBox::Yes)
    {
        // 调用保存文件
        onSaveFile();
        // 检查保存是否成功（如果用户取消保存对话框，m_isModified应该仍然为true）
        return !m_isModified; // 保存成功则返回true，取消则返回false
    }
    else if (ret == QMessageBox::No)
    {
        return true; // 不保存，继续操作
    }
    else
    {
        return false; // 取消操作
    }
}

void MainWindow::markModified()
{
    m_isModified = true;
    // 使用Qt内置的窗口修改标志
    setWindowModified(true);
    // 在标题栏展示
    setWindowTitle("表情包制作器 *");
}

void MainWindow::resetModified()
{
    m_isModified = false;
    // 使用Qt内置的窗口修改标志
    setWindowModified(false);
    setWindowTitle("表情包制作器");
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
}

void MainWindow::addItemToSceneDirectly(ResizableItem *item)
{
    if (!item)
        return;

    bool isNewItem = !item->data(0).isValid();
    if (isNewItem)
    {
        item->setData(0, true);
    }

    if (item->isCanvas())
    {
        m_canvasItem = item;
        item->setZValue(-1000);

        bool canvasEditable = (m_editMode == CanvasEditMode);
        item->setFlag(QGraphicsItem::ItemIsMovable, canvasEditable);
        item->setFlag(QGraphicsItem::ItemIsSelectable, canvasEditable);
    }
    else
    {
        if (m_editMode == CanvasEditMode)
        {
            item->setFlag(QGraphicsItem::ItemIsMovable, false);
            item->setFlag(QGraphicsItem::ItemIsSelectable, false);
        }
        else
        {
            item->setFlag(QGraphicsItem::ItemIsMovable, true);
            item->setFlag(QGraphicsItem::ItemIsSelectable, true);
        }
    }

    item->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    item->setAcceptHoverEvents(true);

    QPointF offset = m_canvasOffset + QPointF(m_items.size() * 20, m_items.size() * 20);
    item->setPos(offset);

    m_graphics_scene->addItem(item);

    if (!item->isCanvas())
    {
        Item itemInfo;
        itemInfo.pixmapItem = item;
        itemInfo.offset = offset;
        itemInfo.zValue = m_current_z_value++;
        m_items[item] = itemInfo;

        if (isNewItem)
        {
            connect(item, &ResizableItem::moveUpSignal, this, [this](ResizableItem *target)
                    {
                if (!m_items.contains(target)) return;
                qreal maxZValue = 0;
                for (const auto &itemInfo : m_items) {
                    if (itemInfo.zValue > maxZValue) {
                        maxZValue = itemInfo.zValue;
                    }
                }
                qreal oldZValue = m_items[target].zValue;
                qreal newZValue = maxZValue + 1;
                m_items[target].zValue = newZValue;
                target->setZValue(newZValue);
                pushCommand(new ZValueChangeCommand(this, target, oldZValue, newZValue)); });
            connect(item, &ResizableItem::moveDownSignal, this, [this](ResizableItem *target)
                    {
                if (!m_items.contains(target)) return;
                qreal minZValue = m_items[target].zValue;
                bool first = true;
                for (const auto &itemInfo : m_items) {
                    if (first) {
                        minZValue = itemInfo.zValue;
                        first = false;
                    } else if (itemInfo.zValue < minZValue) {
                        minZValue = itemInfo.zValue;
                    }
                }
                qreal oldZValue = m_items[target].zValue;
                qreal newZValue = minZValue - 1;
                m_items[target].zValue = newZValue;
                target->setZValue(newZValue);
                pushCommand(new ZValueChangeCommand(this, target, oldZValue, newZValue)); });

            connect(item, &ResizableItem::itemDoubleClicked, this, [this](ResizableItem *target)
                    {
                if(target->getItemType()==ItemType::Type_Image){
                    QImage image = getImageFromFile("选择要更换的图片");
                    if(image.isNull()) return;
                    QPixmap oldPixmap = target->getPixmap();
                    QPixmap newPixmap = QPixmap::fromImage(image);
                    pushCommand(new ChangePixmapCommand(this, target, oldPixmap, newPixmap));
                } else {
                    bool ok;
                    QString newText = QInputDialog::getText(this, "编辑文本",
                                                            "请输入新内容:",
                                                            QLineEdit::Normal,
                                                            target->getText(), &ok);
                    if (ok && !newText.isEmpty()) {
                        QString oldText = target->getText();
                        pushCommand(new ChangeTextCommand(this, target, oldText, newText));
                    }
                } });

            connect(item, &ResizableItem::imageCropRequested, this, [this](ResizableItem *target)
                    {
                auto image = ImageCropperDialog::getCroppedImage(target->getPixmap(),1024,576,CropperShape::RECT);
                if(!image.isNull()) {
                    QPixmap oldPixmap = target->getPixmap();
                    QPixmap newPixmap = image;
                    pushCommand(new ChangePixmapCommand(this, target, oldPixmap, newPixmap));
                } });

            connect(item, &ResizableItem::itemDeleteRequested, this, [this](ResizableItem *target)
                    {
                // 创建删除命令，命令会处理item的删除和恢复
                pushCommand(new DeleteItemCommand(this, target));
            });

            connect(item, &ResizableItem::sizeChanged, this, [this](ResizableItem *)
                    {
                if (m_isUndoRedoing) return;
                markModified(); });
            connect(item, &ResizableItem::positionChanged, this, [this](ResizableItem *)
                    {
                if (m_isUndoRedoing) return;
                markModified(); });
            connect(item, &ResizableItem::moveStarted, this, [this](ResizableItem *target)
                    {
                if (m_isUndoRedoing) return;
                recordItemMoveStart(target); });
            connect(item, &ResizableItem::moveFinished, this, [this](ResizableItem *target)
                    {
                if (m_isUndoRedoing) return;
                finishItemMove(target); });
        }

        selectItem(item);
        markModified();
    }
    else
    {
        if (isNewItem)
        {
            connect(item, &ResizableItem::itemDoubleClicked,
                    this, &MainWindow::onCanvasDoubleClicked);
            connect(item, &ResizableItem::sizeChanged,
                    this, [this](ResizableItem *)
                    {
                    if (m_isUndoRedoing) return;
                    onUpdateCanvasSizeLabel();
                    markModified(); });
            connect(item, &ResizableItem::positionChanged,
                    this, [this](ResizableItem *)
                    {
                    if (m_isUndoRedoing) return;
                    m_canvasOffset = m_canvasItem->pos();
                    markModified(); });
            connect(item, &ResizableItem::changeCanvasSize,
                    this, [this]
                    { onSetCanvasSize(); });
        }
    }

    DimOutsideCanvasEffect *effect = new DimOutsideCanvasEffect(item, m_canvasItem);
    item->setGraphicsEffect(effect);
}

void MainWindow::removeItemFromScene(ResizableItem *item)
{
    if (!item)
        return;

    m_selected_items.remove(item);
    m_items.remove(item);
    m_graphics_scene->removeItem(item);
    markModified();
}

void MainWindow::moveItemTo(ResizableItem *item, const QPointF &pos)
{
    if (!item)
        return;

    m_isUndoRedoing = true;
    item->setPos(pos);
    m_isUndoRedoing = false;
    markModified();
}

void MainWindow::resizeItemTo(ResizableItem *item, const QRectF &rect, const QPointF &pos)
{
    if (!item)
        return;

    m_isUndoRedoing = true;
    item->setContentRect(rect);
    item->setPos(pos);
    m_isUndoRedoing = false;
    markModified();
}

void MainWindow::setItemZValue(ResizableItem *item, qreal zValue)
{
    if (!item)
        return;

    if (m_items.contains(item))
    {
        m_items[item].zValue = zValue;
    }
    item->setZValue(zValue);
    markModified();
}

void MainWindow::setItemPixmap(ResizableItem *item, const QPixmap &pixmap)
{
    if (!item)
        return;

    m_isUndoRedoing = true;
    item->setPixmap(pixmap);
    m_isUndoRedoing = false;
    markModified();
}

void MainWindow::setItemText(ResizableItem *item, const QString &text)
{
    if (!item)
        return;

    m_isUndoRedoing = true;
    item->setText(text);
    m_isUndoRedoing = false;
    markModified();
}

void MainWindow::setCanvasSize(ResizableItem *canvas, const QSizeF &size)
{
    if (!canvas || canvas != m_canvasItem)
        return;

    m_isUndoRedoing = true;
    canvas->setCanvas(size, canvas->getCanvasColor());
    onUpdateCanvasSizeLabel();
    m_isUndoRedoing = false;
    markModified();
}

void MainWindow::moveCanvasTo(ResizableItem *canvas, const QPointF &pos)
{
    if (!canvas || canvas != m_canvasItem)
        return;

    m_isUndoRedoing = true;
    canvas->setPos(pos);
    m_canvasOffset = pos;
    m_isUndoRedoing = false;
    markModified();
}

void MainWindow::pushCommand(QUndoCommand *command)
{
    if (m_undoStack)
    {
        m_undoStack->push(command);
    }
    else
    {
        delete command;
    }
}

void MainWindow::recordItemMoveStart(ResizableItem *item)
{
    if (!item || item->isCanvas())
        return;
    m_itemMoveStartPos[item] = item->pos();
}

void MainWindow::finishItemMove(ResizableItem *item)
{
    if (!item || item->isCanvas())
        return;
    if (!m_itemMoveStartPos.contains(item))
        return;

    QPointF oldPos = m_itemMoveStartPos[item];
    QPointF newPos = item->pos();

    m_itemMoveStartPos.remove(item);

    if (oldPos != newPos)
    {
        pushCommand(new MoveItemCommand(this, item, oldPos, newPos));
    }
}

void MainWindow::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        int delta = event->angleDelta().y(); // 正值表示向上滚动（放大），负值表示向下滚动（缩小）

        if (delta > 0) {
            onZoomIn();   // 放大
        } else if (delta < 0) {
            onZoomOut();  // 缩小
        }

        event->accept();  // 事件已处理，不再传递
    }
    else {
    // 未按下 Ctrl，交给基类处理
    QMainWindow::wheelEvent(event);
}
}

MainWindow::~MainWindow()
{
    // 清理所有动态分配的ResizableItem对象
    clearScene(); // 删除所有非画布项目

    // 删除画布项目
    if (m_canvasItem)
    {
        // 从场景中移除画布（如果还在场景中）
        if (m_graphics_scene && m_graphics_scene->items().contains(m_canvasItem))
        {
            m_graphics_scene->removeItem(m_canvasItem);
        }
        delete m_canvasItem;
        m_canvasItem = nullptr;
    }

    delete ui;
}
