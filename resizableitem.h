#ifndef RESIZABLEITEM_H
#define RESIZABLEITEM_H

#include <QGraphicsItem>

// 手柄位置枚举
enum HandlePosition {
    Handle_None,
    Handle_TopLeft, Handle_Top, Handle_TopRight,
    Handle_Left, Handle_Right,
    Handle_BottomLeft, Handle_Bottom, Handle_BottomRight
};
// 物品类型枚举
enum ItemType {
    Type_Image,
    Type_Text,
    Type_Canvas  // 新增画布类型
};

class ResizableItem : public QGraphicsObject
{

    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父图形项指针
     */
    explicit ResizableItem(QGraphicsItem *parent = nullptr);

    /**
     * @brief 设置为图片模式
     * @param pixmap 图片数据
     */
    void setPixmap(const QPixmap &pixmap);

    /**
     * @brief 设置为文本模式
     * @param text 文本内容
     */
    void setText(const QString &text);

    /**
     * @brief 返回包围盒
     * @return 包围盒矩形
     */
    QRectF boundingRect() const override;

    /**
     * @brief 核心绘制逻辑
     * @param painter 绘图器
     * @param option 绘图选项
     * @param widget 部件指针
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    /**
     * @brief 获得当前物品的类型
     * @return 物品类型枚举
     */
    ItemType getItemType(){return m_type; }

    /**
     * @brief 获得当前文本内容
     * @return 文本字符串
     */
    QString getText(){return m_text; }

    /**
     * @brief 获得当前图片（仅对图片有效）
     * @return 图片数据
     */
    QPixmap getPixmap(){return m_pixmap; }

    /**
     * @brief 设置文本背景颜色（仅对文本类型有效）
     * @param color 背景颜色
     */
    void setTextBackgroundColor(const QColor &color);
    /**
     * @brief 设置文本颜色（仅对文本类型有效）
     * @param color 文本颜色
     */
    void setTextColor(const QColor &color);

    // 画布相关方法
    /**
     * @brief 设置为画布模式
     * @param size 画布大小
     * @param color 画布颜色，默认为白色
     */
    void setCanvas(const QSizeF &size, const QColor &color = Qt::white);
    /**
     * @brief 获取画布大小
     * @return 画布大小
     */
    QSizeF getCanvasSize() const;
    /**
     * @brief 设置画布颜色
     * @param color 画布颜色
     */
    void setCanvasColor(const QColor &color);
    /**
     * @brief 检查是否为画布
     * @return 如果是画布类型返回true，否则返回false
     */
    bool isCanvas() const { return m_type == Type_Canvas; }

    /**
     * @brief 获取内容矩形（不包含选择手柄扩展）
     * @return 内容矩形，局部坐标
     */
    QRectF getContentRect() const { return m_rect; }

signals:
    /**
     * @brief 双击信号
     * @param item 被双击的项指针
     */
    void itemDoubleClicked(ResizableItem *item);
    /**
     * @brief 裁剪请求信号
     * @param item 请求裁剪的项指针
     */
    void imageCropRequested(ResizableItem *item);
    /**
     * @brief 删除请求信号
     * @param item 请求删除的项指针
     */
    void itemDeleteRequested(ResizableItem *item);
    /**
     * @brief 大小变化信号
     * @param item 大小发生变化的项指针
     */
    void sizeChanged(ResizableItem *item);
    /**
     * @brief 位置变化信号
     * @param item 位置发生变化的项指针
     */
    void positionChanged(ResizableItem *item);
    /**
     * @brief 更改画布大小信号
     * @param item 请求更改画布大小的项指针
     */
    void changeCanvasSize(ResizableItem *item);

protected:
    /**
     * @brief 鼠标悬停事件处理
     * @param event 鼠标悬停事件
     */
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

    /**
     * @brief 鼠标按下事件处理
     * @param event 鼠标按下事件
     */
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 鼠标移动事件处理：执行调整大小或移动
     * @param event 鼠标移动事件
     */
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 鼠标释放事件处理：重置状态
     * @param event 鼠标释放事件
     */
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    /**
     * @brief 鼠标双击事件处理
     * @param event 鼠标双击事件
     */
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    /**
     * @brief 右键菜单事件处理
     * @param event 右键菜单事件
     */
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    ItemType m_type;
    QRectF m_rect;
    QString m_text;
    QPixmap m_pixmap;
    QColor m_textBackgroundColor;  // 文本背景颜色
    QColor m_textColor;            // 文本颜色
    QColor m_canvasColor;          // 画布背景颜色
    mutable qreal m_currentScale;  // 当前视图缩放因子，用于固定手柄大小

    HandlePosition m_handleSelected;

    QPointF m_resizeStartScenePos; // 记录鼠标按下时的场景坐标
    QRectF m_initialRect;          // 记录初始的 rect
    QPointF m_initialPos;          // 记录 Item 初始的场景位置 pos()

    /**
     * @brief 辅助函数：获取特定位置手柄的矩形区域
     * @param handle 手柄位置枚举
     * @return 手柄矩形区域
     */
    QRectF getHandleRect(HandlePosition handle) const;

    /**
     * @brief 辅助函数：判断点在哪个手柄上
     * @param p 点坐标
     * @return 手柄位置枚举
     */
    HandlePosition getHandleAt(const QPointF &p) const;

    /**
     * @brief 辅助函数：设置光标
     * @param handle 手柄位置枚举
     */
    void setCursorShape(HandlePosition handle);
};

#endif // RESIZABLEITEM_H
