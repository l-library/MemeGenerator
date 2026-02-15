#ifndef DIMOUTSIDECANVASEFFECT_H
#define DIMOUTSIDECANVASEFFECT_H

#include <QGraphicsEffect>
#include <QPainter>
#include "resizableitem.h"


class DimOutsideCanvasEffect : public QGraphicsEffect
{
public:
    DimOutsideCanvasEffect(ResizableItem* targetItem, ResizableItem* canvas,QObject* parent = nullptr)
        :QGraphicsEffect(parent),m_canvasItem(canvas),m_targetItem(targetItem)
    {
        // 连接重绘信号
        connect(canvas,&ResizableItem::sizeChanged,this,&DimOutsideCanvasEffect::update);
        connect(canvas,&ResizableItem::positionChanged,this,&DimOutsideCanvasEffect::update);
    }
protected:
    void draw(QPainter* painter) override
    {
        if (!m_targetItem || !m_canvasItem) {
            drawSource(painter);
            return;
        }

        // 绘制原始内容
        drawSource(painter);

        // 获取目标项的内容矩形（局部坐标）
        QRectF targetRect = m_targetItem->getContentRect();
        QPainterPath targetShape;
        targetShape.addRect(targetRect);

        // 获取画布在目标项局部坐标中的内容矩形
        QRectF canvasRect = m_targetItem->mapRectFromItem(m_canvasItem, m_canvasItem->getContentRect());
        QPainterPath canvasPath;
        canvasPath.addRect(canvasRect);

        // 计算画布外部区域
        QPainterPath exterior = targetShape.subtracted(canvasPath);

        // 填充半透明黑色
        painter->fillPath(exterior, QColor(0, 0, 0, 100));
    }
private:
    ResizableItem* m_canvasItem;
    ResizableItem* m_targetItem;
};

#endif // DIMOUTSIDECANVASEFFECT_H
