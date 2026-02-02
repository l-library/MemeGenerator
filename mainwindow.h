#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE

// 向前声明
struct MenuConfig;
class QGridLayout;
class QGraphicsView;
class QGraphicsScene;

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
    void loadMenuConfig();
    void createMenuFromConfig();
    void connectActionToSlot(const QString& actionId, QAction* action);
    void setupDefaultMenuConfig();

    QHash<QString, QAction*> m_actionMap;  // 存储actionId到QAction的映射
    QVector<MenuConfig> m_menuConfigs;     // 存储菜单配置
    QGridLayout* m_grid_layout;            // 网格布局
    QGraphicsView* m_graphics_view;
    QGraphicsScene* m_graphics_scene;
    // 当前图像状态
    double m_current_scale;
};
#endif // MAINWINDOW_H
