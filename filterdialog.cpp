#include "filterdialog.h"
#include "ui_filterdialog.h"
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>

FilterDialog::FilterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FilterDialog)
    ,m_brightness(100),m_saturation(100),m_contrast(100),m_hue(100)
{
    ui->setupUi(this);
    this->setWindowTitle("设置滤镜");

    // 创建布局
    // 创建网格布局
    m_grid_layout= new QGridLayout(this);
    // 设置行和列的比例
    for(int i=0;i<10;++i){
        m_grid_layout->setRowStretch(i, 10);  // 将行均分为10份
    }
    for(int i=0;i<10;++i){
        m_grid_layout->setColumnStretch(i, 10);  // 将列均分为10份
    }
    m_grid_layout->setSpacing(0);
    m_grid_layout->setContentsMargins(0, 0, 0, 0);
    setLayout(m_grid_layout);

    // 预览区域
    m_display_label = new QLabel;
    m_display_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_display_label->setFrameShape(QFrame::Box);
    m_grid_layout->addWidget(m_display_label,0,0,5,10);

    // 应用和取消按钮
    QPushButton* applyButton = new QPushButton(this);
    applyButton->setText("应用");
    applyButton->setStatusTip("应用当前滤镜");
    applyButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_grid_layout->addWidget(applyButton,9,4,1,3);
    QPushButton* cancelButton = new QPushButton(this);
    cancelButton->setText("取消");
    cancelButton->setStatusTip("取消编辑");
    // cancelButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_grid_layout->addWidget(cancelButton,9,7,1,3);
    connect(cancelButton,&QPushButton::pressed,[=](){
        close();
    });

    // 各参数滑动条
    int nMin = 0;
    int nMax = 200;
    int nSingleStep = 1;
    const std::vector<QString> names = {"亮度","饱和度","对比度","色相"};
    auto name = names.begin();
    for(int i = 0;i < 2;++i)
        for(int j = 0;j < 2;++j)
        {
            // 名称
            m_grid_layout->addWidget(new QLabel(*name),5 + i*2,0 + 5*j,1,3);
            // 微调框
            QSpinBox *pSpinBox = new QSpinBox(this);
            pSpinBox->setMinimum(nMin);  // 最小值
            pSpinBox->setMaximum(nMax);  // 最大值
            pSpinBox->setSingleStep(nSingleStep);  // 步长
            m_grid_layout->addWidget(pSpinBox,6 + i *2,0+5*j,1,2);
            pSpinBox->setObjectName(*name);
            name++;
            // 滑动条
            QSlider *pSlider = new QSlider(this);
            pSlider->setOrientation(Qt::Horizontal);  // 水平方向
            pSlider->setMinimum(nMin);  // 最小值
            pSlider->setMaximum(nMax);  // 最大值
            pSlider->setSingleStep(nSingleStep);  // 步长
            m_grid_layout->addWidget(pSlider,6 + i*2,2+5*j,1,3);

            // 连接信号槽（相互改变）
            connect(pSpinBox, SIGNAL(valueChanged(int)), pSlider, SLOT(setValue(int)));
            connect(pSlider, SIGNAL(valueChanged(int)), pSpinBox, SLOT(setValue(int)));
            // 连接信号槽（更改实际值）
            connect(pSpinBox,&QSpinBox::valueChanged,[=](){
                if(pSpinBox->objectName() == "亮度")
                    m_brightness = pSpinBox->value();
                else if(pSpinBox->objectName() == "饱和度")
                    m_saturation = pSpinBox->value();
                else if (pSpinBox->objectName()== "对比度")
                    m_contrast = pSpinBox->value();
                else if(pSpinBox->objectName() == "色相")
                    m_hue = pSpinBox->value();;
            });

            pSpinBox->setValue(100);
        }
}

FilterDialog::~FilterDialog()
{
    delete ui;
}
