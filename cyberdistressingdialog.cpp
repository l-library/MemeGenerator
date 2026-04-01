#include "cyberdistressingdialog.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QPainter>

CyberDistressingDialog::CyberDistressingDialog(QWidget *parent)
    : QDialog(parent), m_image(), m_resolution(0), m_color(0), m_noise(0), m_watermark(0), m_scanLine(false)
{
    this->setWindowTitle("赛博做旧");
    this->setWindowIcon(QIcon(":/icons/distress.png"));
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
    m_display_label->setAlignment(Qt::AlignCenter);
    m_display_label->setMinimumSize(400, 300);
    m_display_label->setScaledContents(false);
    m_grid_layout->addWidget(m_display_label,0,0,4,10);

    // 扫描线效果复选框
    QCheckBox* scanLineCheckBox = new QCheckBox(this);
    scanLineCheckBox->setText("扫描线");
    scanLineCheckBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_grid_layout->addWidget(scanLineCheckBox,4,0,1,2);
    connect(scanLineCheckBox, &QCheckBox::toggled, this, [this](bool checked){
        m_scanLine = checked;
        updatePreview();
    });

    QCheckBox* boardCheckBox = new QCheckBox(this);
    boardCheckBox->setText("边框");
    boardCheckBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_grid_layout->addWidget(boardCheckBox,4,2,1,2);
    connect(boardCheckBox, &QCheckBox::toggled, this, [this](bool checked){
        m_board = checked;
        updatePreview();
    });

    // 应用和取消按钮
    QPushButton* applyButton = new QPushButton(this);
    applyButton->setText("应用");
    applyButton->setStatusTip("应用当前滤镜");
    applyButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_grid_layout->addWidget(applyButton,9,4,1,3);
    QPushButton* cancelButton = new QPushButton(this);
    cancelButton->setText("取消");
    cancelButton->setStatusTip("取消编辑");
    m_grid_layout->addWidget(cancelButton,9,7,1,3);

    connect(cancelButton, &QPushButton::pressed, this, [this](){
        close();
    });
    connect(applyButton,&QPushButton::pressed,this,[this](){
        accept();
    });

    // 各参数滑动条
    int nMin = 0;
    int nMax = 100;
    int nSingleStep = 1;
    const std::vector<QString> names = {"分辨率降低","色彩失真","噪点数量","水印厚度"};
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
            connect(pSpinBox, &QSpinBox::valueChanged, this, [this, pSpinBox](){
                if(pSpinBox->objectName() == "分辨率降低")
                    m_resolution = pSpinBox->value();
                else if(pSpinBox->objectName() == "色彩失真")
                    m_color = pSpinBox->value();
                else if (pSpinBox->objectName()== "噪点数量")
                    m_noise = pSpinBox->value();
                else if(pSpinBox->objectName() == "水印厚度")
                    m_watermark = pSpinBox->value();
                updatePreview();
            });

            pSpinBox->setValue(0);
        }
}

void CyberDistressingDialog::setOriginalImage(const QImage& image)
{
    m_image = image;
    updatePreview();
}

void CyberDistressingDialog::updatePreview()
{
    if (m_image.isNull()) {
        m_display_label->clear();
        m_display_label->setText("暂无图像");
        return;
    }

    m_filteredImage = applyFiltersForPreview(m_image);

    QPixmap pixmap = QPixmap::fromImage(m_filteredImage);

    QSize fixedSize(400, 300);

    QPixmap scaledPixmap = pixmap.scaled(
        fixedSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );

    m_display_label->setPixmap(scaledPixmap);
}

QImage CyberDistressingDialog::applyFilters(const QImage& origin_image)
{
    if (origin_image.isNull()) {
        return origin_image;
    }

    QImage result = origin_image.copy();

    if (m_scanLine) {
        QPainter painter(&result);
        painter.setPen(Qt::NoPen);

        const int outputSpacing = 2;
        const int scanLineHeight = 1;

        int height = result.height();
        for (int y = 0; y < height; y += outputSpacing) {
            painter.setOpacity(0.15);
            painter.setBrush(QColor(0, 0, 0));
            painter.drawRect(0, y, result.width(), scanLineHeight);
        }
        painter.end();
    }

    return result;
}

QImage CyberDistressingDialog::applyFiltersForPreview(const QImage& origin_image)
{
    if (origin_image.isNull()) {
        return origin_image;
    }

    QImage result = origin_image.copy();

    if (m_scanLine) {
        QPainter painter(&result);
        painter.setPen(Qt::NoPen);

        const int scanLineSpacingOnScreen = 3;
        QSize fixedSize(400, 300);
        QSize scaledSize = fixedSize.boundedTo(origin_image.size());
        scaledSize.scale(fixedSize, Qt::KeepAspectRatio);

        double scaleX = static_cast<double>(origin_image.width()) / scaledSize.width();
        double scaleY = static_cast<double>(origin_image.height()) / scaledSize.height();
        double scale = qMax(scaleX, scaleY);

        int spacing = qMax(1, static_cast<int>(scanLineSpacingOnScreen * scale));

        int height = result.height();
        for (int y = 0; y < height; y += spacing) {
            painter.setOpacity(0.15);
            painter.setBrush(QColor(0, 0, 0));
            painter.drawRect(0, y, result.width(), qMax(1, spacing / 2));
        }
        painter.end();
    }

    return result;
}

CyberDistressingDialog::~CyberDistressingDialog()
{

}
