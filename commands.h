#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include <QGraphicsScene>
#include <QPointF>
#include <QSizeF>
#include <QPixmap>
#include <QString>
#include <QMap>
#include <QSet>

class ResizableItem;
class MainWindow;

class AddItemCommand : public QUndoCommand
{
public:
    AddItemCommand(MainWindow *mainWindow, ResizableItem *item, QPointF pos = {-112, -112},QUndoCommand *parent = nullptr);
    ~AddItemCommand();
    void undo() override;
    void redo() override;
    int id() const override { return 1001; }

private:
    MainWindow *m_mainWindow;
    ResizableItem *m_item;
    QPointF m_position;
    bool m_ownsItem;
};

class DeleteItemCommand : public QUndoCommand
{
public:
    DeleteItemCommand(MainWindow *mainWindow, ResizableItem *item, QUndoCommand *parent = nullptr);
    ~DeleteItemCommand();
    void undo() override;
    void redo() override;
    int id() const override { return 1002; }

private:
    MainWindow *m_mainWindow;
    ResizableItem *m_item;
    bool m_ownsItem;
    QPointF m_position;
    qreal m_zValue;
    int m_itemType;
    QPixmap m_pixmap;
    QString m_text;
    QColor m_textBackgroundColor;
    QColor m_textColor;
    QFont m_textFont;
};

class MoveItemCommand : public QUndoCommand
{
public:
    MoveItemCommand(MainWindow *mainWindow, ResizableItem *item, const QPointF &oldPos, const QPointF &newPos, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
    int id() const override { return 1003; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    MainWindow *m_mainWindow;
    ResizableItem *m_item;
    QPointF m_oldPos;
    QPointF m_newPos;
};

class ResizeItemCommand : public QUndoCommand
{
public:
    ResizeItemCommand(MainWindow *mainWindow, ResizableItem *item, const QRectF &oldRect, const QRectF &newRect, const QPointF &oldPos, const QPointF &newPos, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
    int id() const override { return 1004; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    MainWindow *m_mainWindow;
    ResizableItem *m_item;
    QRectF m_oldRect;
    QRectF m_newRect;
    QPointF m_oldPos;
    QPointF m_newPos;
};

class ZValueChangeCommand : public QUndoCommand
{
public:
    ZValueChangeCommand(MainWindow *mainWindow, ResizableItem *item, qreal oldZValue, qreal newZValue, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
    int id() const override { return 1005; }

private:
    MainWindow *m_mainWindow;
    ResizableItem *m_item;
    qreal m_oldZValue;
    qreal m_newZValue;
};

class ChangePixmapCommand : public QUndoCommand
{
public:
    ChangePixmapCommand(MainWindow *mainWindow, ResizableItem *item, const QPixmap &oldPixmap, const QPixmap &newPixmap, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
    int id() const override { return 1006; }

private:
    MainWindow *m_mainWindow;
    ResizableItem *m_item;
    QPixmap m_oldPixmap;
    QPixmap m_newPixmap;
};

class ChangeTextCommand : public QUndoCommand
{
public:
    ChangeTextCommand(MainWindow *mainWindow, ResizableItem *item, const QString &oldText, const QString &newText, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
    int id() const override { return 1007; }

private:
    MainWindow *m_mainWindow;
    ResizableItem *m_item;
    QString m_oldText;
    QString m_newText;
};

class CanvasResizeCommand : public QUndoCommand
{
public:
    CanvasResizeCommand(MainWindow *mainWindow, ResizableItem *canvas, const QSizeF &oldSize, const QSizeF &newSize, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
    int id() const override { return 1008; }

private:
    MainWindow *m_mainWindow;
    ResizableItem *m_canvas;
    QSizeF m_oldSize;
    QSizeF m_newSize;
};

class CanvasMoveCommand : public QUndoCommand
{
public:
    CanvasMoveCommand(MainWindow *mainWindow, ResizableItem *canvas, const QPointF &oldPos, const QPointF &newPos, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
    int id() const override { return 1009; }
    bool mergeWith(const QUndoCommand *other) override;

private:
    MainWindow *m_mainWindow;
    ResizableItem *m_canvas;
    QPointF m_oldPos;
    QPointF m_newPos;
};

class CompositeCommand : public QUndoCommand
{
public:
    CompositeCommand(const QString &text, QList<QUndoCommand *> commands, QUndoCommand *parent = nullptr);
    ~CompositeCommand();
    void undo() override;
    void redo() override;
    int id() const override { return 1100; }

private:
    QList<QUndoCommand *> m_commands;
};

#endif // COMMANDS_H
