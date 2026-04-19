#include "commands.h"
#include "mainwindow.h"
#include "resizableitem.h"
#include <QGraphicsScene>

AddItemCommand::AddItemCommand(MainWindow *mainWindow, ResizableItem *item,QPointF pos, QUndoCommand *parent)
    : QUndoCommand(parent), m_mainWindow(mainWindow), m_item(item), m_position(pos), m_ownsItem(false)
{
    setText(QObject::tr("添加项目"));
}

AddItemCommand::~AddItemCommand()
{
    if (m_ownsItem && m_item)
    {
        delete m_item;
        m_item = nullptr;
    }
}

void AddItemCommand::undo()
{
    if (!m_mainWindow || !m_item)
        return;

    m_mainWindow->removeItemFromScene(m_item);
    m_ownsItem = true;
}

void AddItemCommand::redo()
{
    if (!m_mainWindow || !m_item)
        return;

    m_mainWindow->addItemToSceneDirectly(m_item, m_position);
    m_ownsItem = false;
}

DeleteItemCommand::DeleteItemCommand(MainWindow *mainWindow, ResizableItem *item, QUndoCommand *parent)
    : QUndoCommand(parent), m_mainWindow(mainWindow), m_item(item), m_ownsItem(false)
{
    setText(QObject::tr("删除项目"));

    if (item)
    {
        m_position = item->pos();
        m_zValue = item->zValue();
        m_itemType = static_cast<int>(item->getItemType());
        m_pixmap = item->getPixmap();
        m_text = item->getText();
    }
}

DeleteItemCommand::~DeleteItemCommand()
{
    // 注意：item在redo()中已经被删除，所以这里不需要再删除
    // 但为了安全起见，如果还有item存在且我们拥有它，则删除它
    if (m_ownsItem && m_item)
    {
        delete m_item;
        m_item = nullptr;
    }
}

void DeleteItemCommand::undo()
{
    if (!m_mainWindow)
        return;

    // 如果item已经被删除（在redo中），需要重新创建
    if (!m_item)
    {
        m_item = new ResizableItem();
        ItemType type = static_cast<ItemType>(m_itemType);
        if (type == Type_Image)
        {
            m_item->setPixmap(m_pixmap);
        }
        else if (type == Type_Text)
        {
            m_item->setText(m_text);
        }
        
        m_item->setPos(m_position);
        m_item->setZValue(m_zValue);
    }

    m_mainWindow->addItemToSceneDirectly(m_item);
    m_ownsItem = false;
}

void DeleteItemCommand::redo()
{
    if (!m_mainWindow)
        return;

    if (m_item)
    {
        m_mainWindow->removeItemFromScene(m_item);
        // 立即删除item
        delete m_item;
        m_item = nullptr;
        m_ownsItem = true;
    }
}

MoveItemCommand::MoveItemCommand(MainWindow *mainWindow, ResizableItem *item, const QPointF &oldPos, const QPointF &newPos, QUndoCommand *parent)
    : QUndoCommand(parent), m_mainWindow(mainWindow), m_item(item), m_oldPos(oldPos), m_newPos(newPos)
{
    setText(QObject::tr("移动项目"));
}

void MoveItemCommand::undo()
{
    if (!m_mainWindow || !m_item)
        return;
    m_mainWindow->moveItemTo(m_item, m_oldPos);
}

void MoveItemCommand::redo()
{
    if (!m_mainWindow || !m_item)
        return;
    m_mainWindow->moveItemTo(m_item, m_newPos);
}

bool MoveItemCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id())
        return false;

    const MoveItemCommand *moveCmd = static_cast<const MoveItemCommand *>(other);
    if (moveCmd->m_item != m_item)
        return false;

    m_newPos = moveCmd->m_newPos;
    return true;
}

ResizeItemCommand::ResizeItemCommand(MainWindow *mainWindow, ResizableItem *item, const QRectF &oldRect, const QRectF &newRect, const QPointF &oldPos, const QPointF &newPos, QUndoCommand *parent)
    : QUndoCommand(parent), m_mainWindow(mainWindow), m_item(item), m_oldRect(oldRect), m_newRect(newRect), m_oldPos(oldPos), m_newPos(newPos)
{
    setText(QObject::tr("调整大小"));
}

void ResizeItemCommand::undo()
{
    if (!m_mainWindow || !m_item)
        return;
    m_mainWindow->resizeItemTo(m_item, m_oldRect, m_oldPos);
}

void ResizeItemCommand::redo()
{
    if (!m_mainWindow || !m_item)
        return;
    m_mainWindow->resizeItemTo(m_item, m_newRect, m_newPos);
}

bool ResizeItemCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id())
        return false;

    const ResizeItemCommand *resizeCmd = static_cast<const ResizeItemCommand *>(other);
    if (resizeCmd->m_item != m_item)
        return false;

    m_newRect = resizeCmd->m_newRect;
    m_newPos = resizeCmd->m_newPos;
    return true;
}

ZValueChangeCommand::ZValueChangeCommand(MainWindow *mainWindow, ResizableItem *item, qreal oldZValue, qreal newZValue, QUndoCommand *parent)
    : QUndoCommand(parent), m_mainWindow(mainWindow), m_item(item), m_oldZValue(oldZValue), m_newZValue(newZValue)
{
    setText(QObject::tr("更改图层顺序"));
}

void ZValueChangeCommand::undo()
{
    if (!m_mainWindow || !m_item)
        return;
    m_mainWindow->setItemZValue(m_item, m_oldZValue);
}

void ZValueChangeCommand::redo()
{
    if (!m_mainWindow || !m_item)
        return;
    m_mainWindow->setItemZValue(m_item, m_newZValue);
}

ChangePixmapCommand::ChangePixmapCommand(MainWindow *mainWindow, ResizableItem *item, const QPixmap &oldPixmap, const QPixmap &newPixmap, QUndoCommand *parent)
    : QUndoCommand(parent), m_mainWindow(mainWindow), m_item(item), m_oldPixmap(oldPixmap), m_newPixmap(newPixmap)
{
    setText(QObject::tr("更改图片"));
}

void ChangePixmapCommand::undo()
{
    if (!m_mainWindow || !m_item)
        return;
    m_mainWindow->setItemPixmap(m_item, m_oldPixmap);
}

void ChangePixmapCommand::redo()
{
    if (!m_mainWindow || !m_item)
        return;
    m_mainWindow->setItemPixmap(m_item, m_newPixmap);
}

ChangeTextCommand::ChangeTextCommand(MainWindow *mainWindow, ResizableItem *item, const QString &oldText, const QString &newText, QUndoCommand *parent)
    : QUndoCommand(parent), m_mainWindow(mainWindow), m_item(item), m_oldText(oldText), m_newText(newText)
{
    setText(QObject::tr("更改文本"));
}

void ChangeTextCommand::undo()
{
    if (!m_mainWindow || !m_item)
        return;
    m_mainWindow->setItemText(m_item, m_oldText);
}

void ChangeTextCommand::redo()
{
    if (!m_mainWindow || !m_item)
        return;
    m_mainWindow->setItemText(m_item, m_newText);
}

CanvasResizeCommand::CanvasResizeCommand(MainWindow *mainWindow, ResizableItem *canvas, const QSizeF &oldSize, const QSizeF &newSize, QUndoCommand *parent)
    : QUndoCommand(parent), m_mainWindow(mainWindow), m_canvas(canvas), m_oldSize(oldSize), m_newSize(newSize)
{
    setText(QObject::tr("调整画布大小"));
}

void CanvasResizeCommand::undo()
{
    if (!m_mainWindow || !m_canvas)
        return;
    m_mainWindow->setCanvasSize(m_canvas, m_oldSize);
}

void CanvasResizeCommand::redo()
{
    if (!m_mainWindow || !m_canvas)
        return;
    m_mainWindow->setCanvasSize(m_canvas, m_newSize);
}

CanvasMoveCommand::CanvasMoveCommand(MainWindow *mainWindow, ResizableItem *canvas, const QPointF &oldPos, const QPointF &newPos, QUndoCommand *parent)
    : QUndoCommand(parent), m_mainWindow(mainWindow), m_canvas(canvas), m_oldPos(oldPos), m_newPos(newPos)
{
    setText(QObject::tr("移动画布"));
}

void CanvasMoveCommand::undo()
{
    if (!m_mainWindow || !m_canvas)
        return;
    m_mainWindow->moveCanvasTo(m_canvas, m_oldPos);
}

void CanvasMoveCommand::redo()
{
    if (!m_mainWindow || !m_canvas)
        return;
    m_mainWindow->moveCanvasTo(m_canvas, m_newPos);
}

bool CanvasMoveCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id())
        return false;

    const CanvasMoveCommand *moveCmd = static_cast<const CanvasMoveCommand *>(other);
    if (moveCmd->m_canvas != m_canvas)
        return false;

    m_newPos = moveCmd->m_newPos;
    return true;
}

CompositeCommand::CompositeCommand(const QString &text, QList<QUndoCommand *> commands, QUndoCommand *parent)
    : QUndoCommand(text, parent), m_commands(commands)
{
}

CompositeCommand::~CompositeCommand()
{
    qDeleteAll(m_commands);
    m_commands.clear();
}

void CompositeCommand::undo()
{
    for (int i = m_commands.size() - 1; i >= 0; --i)
    {
        m_commands[i]->undo();
    }
}

void CompositeCommand::redo()
{
    for (QUndoCommand *cmd : m_commands)
    {
        cmd->redo();
    }
}
