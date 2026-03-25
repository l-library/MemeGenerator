#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QUndoStack>

QT_BEGIN_NAMESPACE

// 向前声明
struct MenuConfig;
class QGridLayout;
class QGraphicsView;
class QGraphicsScene;
class ResizableItem;
class QLabel;

namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    friend class AddItemCommand;
    friend class DeleteItemCommand;
    friend class MoveItemCommand;
    friend class ResizeItemCommand;
    friend class ZValueChangeCommand;
    friend class ChangePixmapCommand;
    friend class ChangeTextCommand;
    friend class CanvasResizeCommand;
    friend class CanvasMoveCommand;
    friend class CompositeCommand;

public:
    /**
     * @brief 主窗口构造函数
     * @param parent 父窗口指针，如果为nullptr则表示顶层窗口
     */
    MainWindow(QWidget *parent = nullptr);
    /**
     * @brief 主窗口析构函数
     */
    ~MainWindow();

    /**
     * @brief 场景中的物品数据结构（图片/文本）
     */
    struct Item {
        ResizableItem* pixmapItem;
        QPointF offset;  // 相对于原始位置的偏移
        qreal zValue;    // z轴值，用于图层管理
    };

protected:
    void wheelEvent(QWheelEvent *event) override;   // 重写滚轮事件

private slots:
    // 槽函数
    /**
     * @brief 新建文件槽函数
     */
    void onNewFile();
    /**
     * @brief 打开文件槽函数
     */
    void onOpenFile();
    /**
     * @brief 保存文件槽函数
     */
    void onSaveFile();
    /**
     * @brief 另存为文件槽函数
     */
    void onSaveAsFile();
    /**
     * @brief 退出应用程序槽函数
     */
    void onExit();
    /**
     * @brief 撤销操作槽函数
     */
    void onUndo();
    /**
     * @brief 重做操作槽函数
     */
    void onRedo();
    /**
     * @brief 关于对话框槽函数
     */
    void onAbout();
    /**
     * @brief 放大视图槽函数
     */
    void onZoomIn();
    /**
     * @brief 缩小视图槽函数
     */
    void onZoomOut();
    /**
     * @brief 复制选中项槽函数
     */
    void onCopy();
    /**
     * @brief 粘贴槽函数
     */
    void onPaste();
    /**
     * @brief 删除选中项槽函数
     */
    void onDeleteSelected();
    /**
     * @brief 插入图片槽函数
     */
    void onInsertPicture();
    /**
     * @brief 插入文本槽函数
     */
    void onInsertText();
    /**
     * @brief 裁剪图片槽函数
     */
    void onCutting();
    /**
     * @brief 应用滤镜槽函数
     */
    void onFilter();

    // 画布相关槽函数
    /**
     * @brief 切换画布编辑模式槽函数
     */
    void onToggleCanvasEditMode();
    /**
     * @brief 画布双击事件槽函数
     * @param canvas 被双击的画布项指针
     */
    void onCanvasDoubleClicked(ResizableItem* canvas);
    /**
     * @brief 设置画布大小槽函数
     */
    void onSetCanvasSize();
    /**
     * @brief 重置画布槽函数
     */
    void onResetCanvas();
    /**
     * @brief 更新画布大小标签槽函数
     */
    void onUpdateCanvasSizeLabel();

private:
    // 编辑模式枚举
    enum EditMode {
        NormalMode,      // 普通编辑模式
        CanvasEditMode   // 画布编辑模式
    };

private:
    Ui::MainWindow *ui;
    /**
     * @brief 初始化菜单栏
     */
    void initMenuBar();
    /**
     * @brief 初始化图形视图
     */
    void initGraphicsView();
    /**
     * @brief 初始化按钮
     */
    void initButton();
    /**
     * @brief 保存文件
     * @param title 对话框标题
     */
    void save(QString title);
    /**
     * @brief 加载菜单配置
     */
    void loadMenuConfig();
    /**
     * @brief 根据配置创建菜单
     */
    void createMenuFromConfig();
    /**
     * @brief 连接菜单动作到槽函数
     * @param actionId 动作ID
     * @param action 动作指针
     */
    void connectActionToSlot(const QString& actionId, QAction* action);
    /**
     * @brief 设置默认菜单配置
     */
    void setupDefaultMenuConfig();
    /**
     * @brief 添加项目到场景
     * @param item 可调整大小的项目指针
     */
    void addItemToScene(ResizableItem* item);
    /**
     * @brief 选择项目
     * @param item 要选择的项目指针
     */
    void selectItem(ResizableItem* item);
    /**
     * @brief 取消所有选择
     */
    void deselectAll();
    /**
     * @brief 清空场景
     */
    void clearScene();
    /**
     * @brief 从文件获取图片
     * @param title 对话框标题
     * @return 图片对象，如果加载失败返回空图片
     */
    QImage getImageFromFile(QString title);

    /**
     * @brief 获取整个场景的图像用于滤镜处理
     * @return 场景图像，如果失败返回空图像
     */
    QImage getSceneImage();

    // 画布管理相关方法
    /**
     * @brief 创建默认画布
     */
    void createDefaultCanvas();
    /**
     * @brief 创建画布
     * @param size 画布大小
     * @param pos_x X坐标
     * @param pox_y Y坐标（疑似拼写错误，应为pos_y）
     * @param color 画布颜色
     */
    void createCanvas(QSizeF size, qreal pos_x, qreal pox_y, Qt::GlobalColor color);
    /**
     * @brief 设置编辑模式
     * @param mode 编辑模式枚举
     */
    void setEditMode(EditMode mode);
    /**
     * @brief 更新项目可编辑性
     */
    void updateItemEditability();
    /**
     * @brief 获取画布导出区域
     * @return 画布矩形区域
     */
    QRectF getCanvasExportRect() const;
    /**
     * @brief 更新编辑模式UI
     */
    void updateEditModeUI();

    /**
     * @brief 检查是否需要保存修改，如果需要则提示用户
     * @return true表示可以继续操作（用户选择"否"或保存成功），false表示用户取消操作
     */
    bool maybeSave();

    /**
     * @brief 标记场景已被修改
     */
    void markModified();

    /**
     * @brief 重置修改状态（例如保存或加载文件后）
     */
    void resetModified();

    /**
     * @brief 重写关闭事件，检查是否需要保存修改
     * @param event 关闭事件
     */
    void closeEvent(QCloseEvent *event) override;

    // 撤销/重做系统辅助方法（供命令类调用）
    /**
     * @brief 直接添加项目到场景（不创建命令）
     */
    void addItemToSceneDirectly(ResizableItem* item);
    /**
     * @brief 从场景移除项目（不创建命令）
     */
    void removeItemFromScene(ResizableItem* item);
    /**
     * @brief 移动项目到指定位置（不创建命令）
     */
    void moveItemTo(ResizableItem* item, const QPointF& pos);
    /**
     * @brief 调整项目大小（不创建命令）
     */
    void resizeItemTo(ResizableItem* item, const QRectF& rect, const QPointF& pos);
    /**
     * @brief 设置项目Z值（不创建命令）
     */
    void setItemZValue(ResizableItem* item, qreal zValue);
    /**
     * @brief 设置项目图片（不创建命令）
     */
    void setItemPixmap(ResizableItem* item, const QPixmap& pixmap);
    /**
     * @brief 设置项目文本（不创建命令）
     */
    void setItemText(ResizableItem* item, const QString& text);
    /**
     * @brief 设置画布大小（不创建命令）
     */
    void setCanvasSize(ResizableItem* canvas, const QSizeF& size);
    /**
     * @brief 移动画布到指定位置（不创建命令）
     */
    void moveCanvasTo(ResizableItem* canvas, const QPointF& pos);
    /**
     * @brief 推送命令到撤销栈
     */
    void pushCommand(QUndoCommand* command);
    /**
     * @brief 检查是否正在执行撤销/重做操作
     */
    bool isUndoRedoing() const { return m_isUndoRedoing; }
    /**
     * @brief 设置撤销/重做状态
     */
    void setUndoRedoing(bool value) { m_isUndoRedoing = value; }
    /**
     * @brief 记录项目移动前的位置
     */
    void recordItemMoveStart(ResizableItem* item);
    /**
     * @brief 完成项目移动并创建命令
     */
    void finishItemMove(ResizableItem* item);

    QHash<QString, QAction*> m_actionMap;  // 存储actionId到QAction的映射
    QVector<MenuConfig> m_menuConfigs;     // 存储菜单配置
    QLabel* m_canvasSizeLabel;             // 状态栏显示当前画布大小
    // 图形界面
    QGridLayout* m_grid_layout;            // 网格布局
    QGraphicsScene* m_graphics_scene;      // 视图
    QGraphicsView* m_graphics_view;        // 场景
    // 图片管理
    QMap<ResizableItem*, Item> m_items;             // 存储所有对象
    QSet<ResizableItem*> m_selected_items;          // 当前选中的对象
    qreal m_current_z_value;                        // 当前z值计数器
    // 当前视图的整体
    double m_view_scale;
    // 画布管理
    EditMode m_editMode;          // 当前编辑模式
    ResizableItem* m_canvasItem;  // 指向画布对象的指针
    QPointF m_canvasOffset;       // 画布的位置偏移
    // 修改状态管理
    bool m_isModified;            // 场景是否被修改
    // 撤销/重做系统
    QUndoStack* m_undoStack;      // 撤销栈
    bool m_isUndoRedoing;         // 是否正在执行撤销/重做操作
    QMap<ResizableItem*, QPointF> m_itemMoveStartPos;  // 项目移动前的位置
};
#endif // MAINWINDOW_H
