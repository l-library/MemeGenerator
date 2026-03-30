#include "cyberdistressingdialog.h"
#include "ui_cyberdistressingdialog.h"

CyberDistressingDialog::CyberDistressingDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CyberDistressing)
{
    ui->setupUi(this);
}

CyberDistressingDialog::~CyberDistressingDialog()
{
    delete ui;
}
