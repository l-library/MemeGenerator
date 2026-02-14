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
    // 构造函数
    explicit ResizableItem(QGraphicsItem *parent = nullptr);

    // 设置为图片模式
    void setPixmap(const QPixmap &pixmap);

    // 设置为文本模式
    void setText(const QString &text);

    // 返回包围盒
    QRectF boundingRect() const override;

    // 核心绘制逻辑
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // 获得当前物品的类型
    ItemType getItemType(){return m_type; }

    // 获得当前文本内容
    QString getText(){return m_text; }

    // 获得当前图片（仅对图片有效）
    QPixmap getPixmap(){return m_pixmap; }

    // 设置文本背景颜色（仅对文本类型有效）
    void setTextBackgroundColor(const QColor &color);
    // 设置文本颜色（仅对文本类型有效）
    void setTextColor(const QColor &color);

    // 画布相关方法
    // 设置为画布模式
    void setCanvas(const QSizeF &size, const QColor &color = Qt::white);
    // 获取画布大小
    QSizeF getCanvasSize() const;
    // 设置画布颜色
    void setCanvasColor(const QColor &color);
    // 检查是否为画布
    bool isCanvas() const { return m_type == Type_Canvas; }

signals:
    // 双击信号
    void itemDoubleClicked(ResizableItem *item);
    // 裁剪请求信号
    void imageCropRequested(ResizableItem *item);
    // 删除请求信号
    void itemDeleteRequested(ResizableItem *item);
    // 大小变化信号
    void sizeChanged(ResizableItem *item);
    // 位置变化信号
    void positionChanged(ResizableItem *item);
    // 更改画布大小信号
    void changeCanvasSize(ResizableItem *item);

protected:
    // 鼠标悬停
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

    // 鼠标按下
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    // 鼠标移动：执行调整大小或移动
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    // 鼠标释放：重置状态
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    // 双击事件声明
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    // 右键菜单事件
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

    // 辅助函数：获取特定位置手柄的矩形区域
    QRectF getHandleRect(HandlePosition handle) const;

    // 辅助函数：判断点在哪个手柄上
    HandlePosition getHandleAt(const QPointF &p) const;

    // 辅助函数：设置光标
    void setCursorShape(HandlePosition handle);
};

#endif // RESIZABLEITEM_H
