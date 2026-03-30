#ifndef CYBERDISTRESSINGDIALOG_H
#define CYBERDISTRESSINGDIALOG_H

#include <QDialog>

namespace Ui {
class CyberDistressing;
}

class CyberDistressingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CyberDistressingDialog(QWidget *parent = nullptr);
    ~CyberDistressingDialog();

private:
    Ui::CyberDistressing *ui;
};

#endif // CYBERDISTRESSINGDIALOG_H
