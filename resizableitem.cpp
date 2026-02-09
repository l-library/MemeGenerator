#include "resizableitem.h"
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QDebug>

// 定义控制手柄的大小
const int HANDLE_SIZE = 8;
const int MIN_SIZE = 20; // 最小尺寸

ResizableItem::ResizableItem(QGraphicsItem *parent)
    : QGraphicsObject (parent), m_type(Type_Text), m_rect(0, 0, 100, 100), m_handleSelected(Handle_None)
{
    // 允许选中和移动
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    // 允许鼠标悬停事件（用于更改光标形状）
    setAcceptHoverEvents(true);

    m_text = "双击输入文本";
    m_pixmap = QPixmap(100, 100);
    m_pixmap.fill(Qt::lightGray);
}

void ResizableItem::setPixmap(const QPixmap &pixmap) {
    m_type = Type_Image;
    m_pixmap = pixmap;
    // 调整大小为图片大小
    if (m_rect.width() == 100 && m_rect.height() == 100) {
        m_rect.setSize(pixmap.size());
    }
    update();
}

void ResizableItem::setText(const QString &text) {
    m_type = Type_Text;
    m_text = text;
    update();
}

QRectF ResizableItem::boundingRect() const
{
    return m_rect.adjusted(-HANDLE_SIZE/2, -HANDLE_SIZE/2, HANDLE_SIZE/2, HANDLE_SIZE/2);
}

void ResizableItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // 保存当前状态
    painter->save();

    // 绘制内容
    painter->setRenderHint(QPainter::Antialiasing); // 抗锯齿
    if (m_type == Type_Image) {
        // 绘制图片（拉伸适应当前矩形）
        painter->drawPixmap(m_rect.toRect(), m_pixmap);
    } else {
        // 绘制背景
        painter->setBrush(QColor(240, 240, 240));
        painter->setPen(Qt::NoPen);
        painter->drawRect(m_rect);

        // 绘制文字
        painter->setPen(Qt::black);
        painter->drawText(m_rect, Qt::AlignCenter | Qt::TextWordWrap, m_text);
    }

    // 如果被选中，绘制边框和控制手柄
    if (isSelected()) {
        painter->setPen(QPen(Qt::blue, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(m_rect);

        // 绘制8个手柄
        painter->setPen(QPen(Qt::black, 1));
        painter->setBrush(Qt::white);

        // 获取所有手柄的矩形并绘制
        for (int i = 1; i <= 8; ++i) {
            painter->drawRect(getHandleRect((HandlePosition)i));
        }
    }

    // 回到原来的状态
    painter->restore();
}

void ResizableItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (isSelected()) {
        HandlePosition handle = getHandleAt(event->pos());
        setCursorShape(handle);
    } else {
        setCursor(Qt::ArrowCursor);
    }
    QGraphicsItem::hoverMoveEvent(event);
}

void ResizableItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && isSelected()) {
        m_handleSelected = getHandleAt(event->pos());
        if (m_handleSelected != Handle_None) {
            // 记录场景坐标
            m_resizeStartScenePos = event->scenePos();
            // 记录 Item 当前的初始状态
            m_initialRect = m_rect;
            m_initialPos = pos();
            // 拦截事件
            event->accept();
            return;
        }
    }
    QGraphicsItem::mousePressEvent(event);
}

void ResizableItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    m_handleSelected = Handle_None;
    setCursor(Qt::ArrowCursor);
    QGraphicsItem::mouseReleaseEvent(event);
}

// 双击事件实现
void ResizableItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // 发送信号
        emit itemDoubleClicked(this);

        event->accept();
    } else {
        QGraphicsObject::mouseDoubleClickEvent(event); // 注意调用的是 QGraphicsObject
    }
}

void ResizableItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_handleSelected != Handle_None) {
        // 使用场景坐标计算鼠标的总偏移量
        QPointF diff = event->scenePos() - m_resizeStartScenePos;

        QRectF newRect = m_initialRect;
        QPointF newPos = m_initialPos;

        // 根据手柄调整 newRect 的大小和 newPos 的位置
        switch (m_handleSelected) {
        case Handle_Right:
            newRect.setWidth(m_initialRect.width() + diff.x());
            break;
        case Handle_Bottom:
            newRect.setHeight(m_initialRect.height() + diff.y());
            break;
        case Handle_Left:
            newRect.setWidth(m_initialRect.width() - diff.x());
            newPos.setX(m_initialPos.x() + diff.x());
            break;
        case Handle_Top:
            newRect.setHeight(m_initialRect.height() - diff.y());
            newPos.setY(m_initialPos.y() + diff.y());
            break;
        case Handle_BottomRight:
            newRect.setWidth(m_initialRect.width() + diff.x());
            newRect.setHeight(m_initialRect.height() + diff.y());
            break;
        case Handle_BottomLeft:
            newRect.setWidth(m_initialRect.width() - diff.x());
            newRect.setHeight(m_initialRect.height() + diff.y());
            newPos.setX(m_initialPos.x() + diff.x());
            break;
        case Handle_TopRight:
            newRect.setWidth(m_initialRect.width() + diff.x());
            newRect.setHeight(m_initialRect.height() - diff.y());
            newPos.setY(m_initialPos.y() + diff.y());
            break;
        case Handle_TopLeft:
            newRect.setWidth(m_initialRect.width() - diff.x());
            newRect.setHeight(m_initialRect.height() - diff.y());
            newPos.setX(m_initialPos.x() + diff.x());
            newPos.setY(m_initialPos.y() + diff.y());
            break;
        default: break;
        }

        // 最小尺寸限制与位置回弹
        // 宽度限制
        if (newRect.width() < MIN_SIZE) {
            newRect.setWidth(MIN_SIZE);
            if (m_handleSelected == Handle_Left || m_handleSelected == Handle_TopLeft || m_handleSelected == Handle_BottomLeft) {
                // 新位置 = 初始位置 + (初始宽度 - 最小宽度)
                newPos.setX(m_initialPos.x() + m_initialRect.width() - MIN_SIZE);
            }
        }
        // 高度限制
        if (newRect.height() < MIN_SIZE) {
            newRect.setHeight(MIN_SIZE);
            if (m_handleSelected == Handle_Top || m_handleSelected == Handle_TopLeft || m_handleSelected == Handle_TopRight) {
                newPos.setY(m_initialPos.y() + m_initialRect.height() - MIN_SIZE);
            }
        }

        prepareGeometryChange();

        // 设置新的矩形大小 (始终保持 TopLeft 为 0,0)
        m_rect = QRectF(0, 0, newRect.width(), newRect.height());

        // 设置 Item 在场景中的新位置
        setPos(newPos);

    } else {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

QRectF ResizableItem::getHandleRect(HandlePosition handle) const
{
    qreal x = 0, y = 0;
    qreal w = m_rect.width();
    qreal h = m_rect.height();

    switch (handle) {
    case Handle_TopLeft:     x = 0; y = 0; break;
    case Handle_Top:         x = w/2; y = 0; break;
    case Handle_TopRight:    x = w; y = 0; break;
    case Handle_Left:        x = 0; y = h/2; break;
    case Handle_Right:       x = w; y = h/2; break;
    case Handle_BottomLeft:  x = 0; y = h; break;
    case Handle_Bottom:      x = w/2; y = h; break;
    case Handle_BottomRight: x = w; y = h; break;
    default: break;
    }
    return QRectF(x - HANDLE_SIZE/2, y - HANDLE_SIZE/2, HANDLE_SIZE, HANDLE_SIZE);
}

HandlePosition ResizableItem::getHandleAt(const QPointF &p) const {
    for (int i = 1; i <= 8; ++i) {
        if (getHandleRect((HandlePosition)i).contains(p)) {
            return (HandlePosition)i;
        }
    }
    return Handle_None;
}

void ResizableItem::setCursorShape(HandlePosition handle)
{
    switch (handle) {
    case Handle_TopLeft:
    case Handle_BottomRight: setCursor(Qt::SizeFDiagCursor); break;
    case Handle_TopRight:
    case Handle_BottomLeft:  setCursor(Qt::SizeBDiagCursor); break;
    case Handle_Top:
    case Handle_Bottom:      setCursor(Qt::SizeVerCursor); break;
    case Handle_Left:
    case Handle_Right:       setCursor(Qt::SizeHorCursor); break;
    default:                 setCursor(Qt::ArrowCursor); break;
    }
}
