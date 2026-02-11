#include "resizableitem.h"
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneContextMenuEvent>
#include <QCursor>
#include <QDebug>
#include <QMenu>
#include <QColorDialog>

// 定义控制手柄的大小
const int HANDLE_SIZE = 8;
const int MIN_SIZE = 20; // 最小尺寸

ResizableItem::ResizableItem(QGraphicsItem *parent)
    : QGraphicsObject (parent), m_type(Type_Text), m_rect(0, 0, 100, 100),
      m_textBackgroundColor(Qt::transparent), m_textColor(Qt::black), m_canvasColor(Qt::white), m_currentScale(1.0), m_handleSelected(Handle_None)
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
    // 计算基于当前缩放的手柄大小，确保包围盒包含调整后的手柄
    qreal scale = m_currentScale;
    if (scale < 0.001) scale = 1.0;
    qreal handleSizeLocal = HANDLE_SIZE / scale;
    // 限制最大扩展，避免过度放大
    const qreal maxExtension = HANDLE_SIZE * 5.0;
    qreal extension = qMin(handleSizeLocal / 2.0, maxExtension);
    return m_rect.adjusted(-extension, -extension, extension, extension);
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
    } else if (m_type == Type_Canvas) {
        // 绘制画布背景
        painter->setBrush(m_canvasColor);
        painter->setPen(Qt::NoPen);
        painter->drawRect(m_rect);

        // 绘制画布边框（虚线）
        painter->setPen(QPen(Qt::gray, 1, Qt::DashLine));
        painter->setBrush(Qt::NoBrush);
        painter->drawRect(m_rect);
    } else { // Type_Text
        // 绘制背景
        painter->setBrush(m_textBackgroundColor);
        painter->setPen(Qt::NoPen);
        painter->drawRect(m_rect);

        // 绘制文字
        painter->setPen(m_textColor);
        // 根据矩形高度动态调整字体大小，使其随整体放大而放大
        QFont font = painter->font();
        int fontSize = qMax(6, static_cast<int>(m_rect.height() * 0.2));
        font.setPixelSize(fontSize);
        painter->setFont(font);
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

        // 获取当前视图的缩放因子，使手柄大小不随视图缩放而变化
        QTransform transform = painter->worldTransform();
        qreal scaleX = transform.m11();  // X方向缩放
        qreal scaleY = transform.m22();  // Y方向缩放
        // 使用平均缩放因子，处理可能的非均匀缩放
        m_currentScale = (qAbs(scaleX) + qAbs(scaleY)) / 2.0;
        if (m_currentScale < 0.001) m_currentScale = 1.0;  // 避免除零

        // 计算屏幕上的固定手柄大小（逆缩放）
        qreal handleSizeScreen = HANDLE_SIZE;
        qreal handleSizeLocal = handleSizeScreen / m_currentScale;

        // 获取所有手柄的矩形并绘制
        for (int i = 1; i <= 8; ++i) {
            QRectF handleRect = getHandleRect((HandlePosition)i);
            // 调整矩形大小，使在屏幕上显示为固定大小
            QRectF adjustedRect;
            adjustedRect.setWidth(handleSizeLocal);
            adjustedRect.setHeight(handleSizeLocal);
            adjustedRect.moveCenter(handleRect.center());
            painter->drawRect(adjustedRect);
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

        // 对于图片类型，计算初始宽高比（用于角落手柄保持比例）
        qreal aspectRatio = 0.0;
        if (m_type == Type_Image) {
            if (m_initialRect.height() > 0) {
                aspectRatio = m_initialRect.width() / m_initialRect.height();
            }
        }

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
            if (m_type == Type_Image && aspectRatio > 0 && m_initialRect.width() > 0 && m_initialRect.height() > 0) {
                // 保持图片比例：选择较大的变化量作为主导
                if (qAbs(diff.x() / m_initialRect.width()) > qAbs(diff.y() / m_initialRect.height())) {
                    // 宽度变化为主导
                    newRect.setWidth(m_initialRect.width() + diff.x());
                    newRect.setHeight(newRect.width() / aspectRatio);
                } else {
                    // 高度变化为主导
                    newRect.setHeight(m_initialRect.height() + diff.y());
                    newRect.setWidth(newRect.height() * aspectRatio);
                }
            } else {
                newRect.setWidth(m_initialRect.width() + diff.x());
                newRect.setHeight(m_initialRect.height() + diff.y());
            }
            break;
        case Handle_BottomLeft:
            if (m_type == Type_Image && aspectRatio > 0 && m_initialRect.width() > 0 && m_initialRect.height() > 0) {
                // 保持图片比例
                if (qAbs(diff.x() / m_initialRect.width()) > qAbs(diff.y() / m_initialRect.height())) {
                    // 宽度变化为主导
                    newRect.setWidth(m_initialRect.width() - diff.x());
                    newRect.setHeight(newRect.width() / aspectRatio);
                    newPos.setX(m_initialPos.x() + diff.x());
                } else {
                    // 高度变化为主导
                    newRect.setHeight(m_initialRect.height() + diff.y());
                    newRect.setWidth(newRect.height() * aspectRatio);
                    // 需要调整X位置以保持左边固定
                    newPos.setX(m_initialPos.x() + m_initialRect.width() - newRect.width());
                }
            } else {
                newRect.setWidth(m_initialRect.width() - diff.x());
                newRect.setHeight(m_initialRect.height() + diff.y());
                newPos.setX(m_initialPos.x() + diff.x());
            }
            break;
        case Handle_TopRight:
            if (m_type == Type_Image && aspectRatio > 0 && m_initialRect.width() > 0 && m_initialRect.height() > 0) {
                // 保持图片比例
                if (qAbs(diff.x() / m_initialRect.width()) > qAbs(diff.y() / m_initialRect.height())) {
                    // 宽度变化为主导
                    newRect.setWidth(m_initialRect.width() + diff.x());
                    newRect.setHeight(newRect.width() / aspectRatio);
                    // 需要调整Y位置以保持顶边固定
                    newPos.setY(m_initialPos.y() + m_initialRect.height() - newRect.height());
                } else {
                    // 高度变化为主导
                    newRect.setHeight(m_initialRect.height() - diff.y());
                    newRect.setWidth(newRect.height() * aspectRatio);
                    newPos.setY(m_initialPos.y() + diff.y());
                }
            } else {
                newRect.setWidth(m_initialRect.width() + diff.x());
                newRect.setHeight(m_initialRect.height() - diff.y());
                newPos.setY(m_initialPos.y() + diff.y());
            }
            break;
        case Handle_TopLeft:
            if (m_type == Type_Image && aspectRatio > 0 && m_initialRect.width() > 0 && m_initialRect.height() > 0) {
                // 保持图片比例
                if (qAbs(diff.x() / m_initialRect.width()) > qAbs(diff.y() / m_initialRect.height())) {
                    // 宽度变化为主导
                    newRect.setWidth(m_initialRect.width() - diff.x());
                    newRect.setHeight(newRect.width() / aspectRatio);
                    newPos.setX(m_initialPos.x() + diff.x());
                    // 需要调整Y位置以保持顶边固定
                    newPos.setY(m_initialPos.y() + m_initialRect.height() - newRect.height());
                } else {
                    // 高度变化为主导
                    newRect.setHeight(m_initialRect.height() - diff.y());
                    newRect.setWidth(newRect.height() * aspectRatio);
                    newPos.setY(m_initialPos.y() + diff.y());
                    // 需要调整X位置以保持左边固定
                    newPos.setX(m_initialPos.x() + m_initialRect.width() - newRect.width());
                }
            } else {
                newRect.setWidth(m_initialRect.width() - diff.x());
                newRect.setHeight(m_initialRect.height() - diff.y());
                newPos.setX(m_initialPos.x() + diff.x());
                newPos.setY(m_initialPos.y() + diff.y());
            }
            break;
        default: break;
        }

        // 最小尺寸限制与位置回弹
        // 宽度限制
        if (newRect.width() < MIN_SIZE) {
            newRect.setWidth(MIN_SIZE);
            if (m_type == Type_Image && aspectRatio > 0) {
                // 保持图片比例：按比例调整高度
                qreal newHeight = MIN_SIZE / aspectRatio;
                if (newHeight >= MIN_SIZE) {
                    newRect.setHeight(newHeight);
                } else {
                    // 如果按比例计算的高度仍然小于最小尺寸，则设置高度为最小尺寸
                    newRect.setHeight(MIN_SIZE);
                }
            }
            if (m_handleSelected == Handle_Left || m_handleSelected == Handle_TopLeft || m_handleSelected == Handle_BottomLeft) {
                // 新位置 = 初始位置 + (初始宽度 - 最小宽度)
                newPos.setX(m_initialPos.x() + m_initialRect.width() - MIN_SIZE);
            }
        }
        // 高度限制
        if (newRect.height() < MIN_SIZE) {
            newRect.setHeight(MIN_SIZE);
            if (m_type == Type_Image && aspectRatio > 0) {
                // 保持图片比例：按比例调整宽度
                qreal newWidth = MIN_SIZE * aspectRatio;
                if (newWidth >= MIN_SIZE) {
                    newRect.setWidth(newWidth);
                } else {
                    // 如果按比例计算的宽度仍然小于最小尺寸，则设置宽度为最小尺寸
                    newRect.setWidth(MIN_SIZE);
                }
            }
            if (m_handleSelected == Handle_Top || m_handleSelected == Handle_TopLeft || m_handleSelected == Handle_TopRight) {
                newPos.setY(m_initialPos.y() + m_initialRect.height() - MIN_SIZE);
            }
        }

        prepareGeometryChange();

        // 设置新的矩形大小 (始终保持 TopLeft 为 0,0)
        m_rect = QRectF(0, 0, newRect.width(), newRect.height());

        // 设置 Item 在场景中的新位置
        setPos(newPos);

        // 发送大小变化信号
        emit sizeChanged(this);

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
    // 使用当前缩放因子，如果未初始化则使用1.0
    qreal scale = m_currentScale;
    if (scale < 0.001) scale = 1.0;
    qreal handleSizeLocal = HANDLE_SIZE / scale;

    for (int i = 1; i <= 8; ++i) {
        QRectF handleRect = getHandleRect((HandlePosition)i);
        // 创建调整后的矩形（与绘制时一致）
        QRectF adjustedRect;
        adjustedRect.setWidth(handleSizeLocal);
        adjustedRect.setHeight(handleSizeLocal);
        adjustedRect.moveCenter(handleRect.center());

        if (adjustedRect.contains(p)) {
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

void ResizableItem::setTextBackgroundColor(const QColor &color)
{
    if (m_type == Type_Text) {
        m_textBackgroundColor = color;
        update();
    }
}

void ResizableItem::setTextColor(const QColor &color)
{
    if (m_type == Type_Text) {
        m_textColor = color;
        update();
    }
}

// 画布相关方法实现
void ResizableItem::setCanvas(const QSizeF &size, const QColor &color) {
    m_type = Type_Canvas;
    m_rect = QRectF(0, 0, size.width(), size.height());
    m_canvasColor = color;
    update();
}

QSizeF ResizableItem::getCanvasSize() const {
    return m_rect.size();
}

void ResizableItem::setCanvasColor(const QColor &color) {
    if (m_type == Type_Canvas) {
        m_canvasColor = color;
        update();
    }
}

void ResizableItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    // 如果是画布类型，不显示右键菜单（通过主窗口菜单编辑）
    if (m_type == Type_Canvas) {
        event->accept();
        return;
    }

    // 创建右键菜单
    QMenu menu;

    if (m_type == Type_Text) {
        // 文本类型菜单
        QAction *changeBgColorAction = menu.addAction("更换文字背景颜色");
        QAction *changeTextColorAction = menu.addAction("更换文字颜色");
        QAction *editTextAction = menu.addAction("更改文字");
        QAction *deleteAction = menu.addAction("删除");

        // 执行选中的动作
        QAction *selectedAction = menu.exec(event->screenPos());
        if (selectedAction == changeBgColorAction) {
            // 打开颜色对话框选择背景颜色
            QColor color = QColorDialog::getColor(
                m_textBackgroundColor,
                QApplication::activeWindow(),
                "选择背景颜色",
                QColorDialog::ShowAlphaChannel
                );
            if (color.isValid()) {
                setTextBackgroundColor(color);
            }
        } else if (selectedAction == changeTextColorAction) {
            // 打开颜色对话框选择文字颜色
            QColor color = QColorDialog::getColor(
                m_textColor,
                QApplication::activeWindow(),
                "选择文字颜色",
                QColorDialog::ShowAlphaChannel
                );
            if (color.isValid()) {
                setTextColor(color);
            }
        } else if (selectedAction == editTextAction) {
            // 触发双击信号，让主窗口处理文字编辑
            emit itemDoubleClicked(this);
        } else if (selectedAction == deleteAction) {
            // 发射删除请求信号
            emit itemDeleteRequested(this);
        }
    } else if (m_type == Type_Image) {
        // 图像类型菜单
        QAction *changeImageAction = menu.addAction("更换图像");
        QAction *deleteAction = menu.addAction("删除");

        // 执行选中的动作
        QAction *selectedAction = menu.exec(event->screenPos());
        if (selectedAction == changeImageAction) {
            // 触发双击信号，让主窗口处理图像更换
            emit itemDoubleClicked(this);
        } else if (selectedAction == deleteAction) {
            // 发射删除请求信号
            emit itemDeleteRequested(this);
        }
    }

    event->accept();
}
