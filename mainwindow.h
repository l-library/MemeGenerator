#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE

// 向前声明
struct MenuConfig;
class QGridLayout;
class QGraphicsView;
class QGraphicsScene;
class QGraphicsPixmapItem;

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    // 引入QT信号和槽机制
    Q_OBJECT

public:
    // 构造函数中的parent是指父窗口
    // 如果parent是0，表示窗口是一个顶层的窗口
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 图片项数据结构
    struct ImageItem {
        QGraphicsPixmapItem* pixmapItem;
        QString path;
        QPointF offset;  // 相对于原始位置的偏移
        qreal zValue;    // z轴值，用于图层管理
    };

private slots:
    // 槽函数
    void onNewFile();
    void onOpenFile();
    void onSaveFile();
    void onSaveAsFile();
    void onExit();
    void onUndo();
    void onRedo();
    void onAbout();
    void onZoomIn();
    void onZoomOut();
    void onCopy();
    void onPaste();
    void onInsertPicture();
    void onInsertText();
    void onCutting();
    void onFilter();

private:
    Ui::MainWindow *ui;
    void initMenuBar();
    void initGraphicsView();
    void loadMenuConfig();
    void createMenuFromConfig();
    void connectActionToSlot(const QString& actionId, QAction* action);
    void setupDefaultMenuConfig();
    void addImageToScene(const QImage& image, const QString& path);
    void selectImageItem(QGraphicsPixmapItem* item);
    void deselectAll();

    QHash<QString, QAction*> m_actionMap;  // 存储actionId到QAction的映射
    QVector<MenuConfig> m_menuConfigs;     // 存储菜单配置
    // 图形界面
    QGridLayout* m_grid_layout;            // 网格布局
    QGraphicsScene* m_graphics_scene;      // 视图
    QGraphicsView* m_graphics_view;        // 场景
    // 图片管理
    QMap<QGraphicsPixmapItem*, ImageItem> m_image_items;  // 存储所有图片项
    QSet<QGraphicsPixmapItem*> m_selected_items;          // 当前选中的图片项
    qreal m_current_z_value;                              // 当前z值计数器
    // 当前视图的整体
    double m_view_scale;
};
#endif // MAINWINDOW_H
