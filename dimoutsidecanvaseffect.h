#ifndef DIMOUTSIDECANVASEFFECT_H
#define DIMOUTSIDECANVASEFFECT_H

#include <QGraphicsEffect>
#include <QGraphicsItem>

class ResizableItem;

class DimOutsideCanvasEffect : public QGraphicsEffect
{
public:
    DimOutsideCanvasEffect(QGraphicsItem *item, QGraphicsItem *canvas, QObject *parent = nullptr)
        : QGraphicsEffect(parent), m_item(item), m_canvas(canvas)
    {
    }

protected:
    void draw(QPainter *painter) override
    {
        drawSource(painter);
    }

private:
    QGraphicsItem *m_item;
    QGraphicsItem *m_canvas;
};

#endif // DIMOUTSIDECANVASEFFECT_H
