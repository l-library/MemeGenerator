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

class ResizableItem : public QGraphicsItem
{
public:
    enum ItemType {
        Type_Image,
        Type_Text
    };

    // 构造函数
    ResizableItem(QGraphicsItem *parent = nullptr);

    // 设置为图片模式
    void setPixmap(const QPixmap &pixmap);

    // 设置为文本模式
    void setText(const QString &text);

    // 返回包围盒
    QRectF boundingRect() const override;

    // 核心绘制逻辑
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    // 鼠标悬停
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

    // 鼠标按下
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

    // 鼠标移动：执行调整大小或移动
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    // 鼠标释放：重置状态
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    ItemType m_type;
    QRectF m_rect;
    QString m_text;
    QPixmap m_pixmap;

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
