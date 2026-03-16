#include "filterdialog.h"
#include "ui_filterdialog.h"
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QSpinBox>
#include <QSlider>
#include <QPainter>
#include <QColor>

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
    m_display_label->setAlignment(Qt::AlignCenter);
    m_display_label->setMinimumSize(400, 300);
    m_display_label->setScaledContents(false);
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
    m_grid_layout->addWidget(cancelButton,9,7,1,3);

    connect(cancelButton, &QPushButton::pressed, this, [this](){
        close();
    });
    connect(applyButton, &QPushButton::pressed, this, &FilterDialog::onApplyClicked);

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
            connect(pSpinBox, &QSpinBox::valueChanged, this, [this, pSpinBox](){
                if(pSpinBox->objectName() == "亮度")
                    m_brightness = pSpinBox->value();
                else if(pSpinBox->objectName() == "饱和度")
                    m_saturation = pSpinBox->value();
                else if (pSpinBox->objectName()== "对比度")
                    m_contrast = pSpinBox->value();
                else if(pSpinBox->objectName() == "色相")
                    m_hue = pSpinBox->value();
                updatePreview();
            });

            pSpinBox->setValue(100);
        }
}

FilterDialog::~FilterDialog()
{
    delete ui;
}

void FilterDialog::setOriginalImage(const QImage& image)
{
    m_originalImage = image;
    updatePreview();
}

void FilterDialog::updatePreview()
{
    if (m_originalImage.isNull()) {
        m_display_label->clear();
        m_display_label->setText("暂无图像");
        return;
    }

    m_filteredImage = applyFilters(m_originalImage);

    QPixmap pixmap = QPixmap::fromImage(m_filteredImage);

    QSize fixedSize(400, 300);

    QPixmap scaledPixmap = pixmap.scaled(
        fixedSize,
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
    );

    m_display_label->setPixmap(scaledPixmap);
}

void FilterDialog::onApplyClicked()
{
    accept();
}

QImage FilterDialog::getFilteredCopy() const
{
    return m_filteredImage;
}

QImage FilterDialog::applyFilters(const QImage& image) const
{
    if (image.isNull())
        return image;

    QImage result = image.convertToFormat(QImage::Format_ARGB32);

    // 归一化参数
    qreal brightnessFactor = m_brightness / 100.0;
    qreal contrastFactor = (m_contrast - 100) / 100.0;
    qreal saturationFactor = m_saturation / 100.0;
    qreal hueRotation = m_hue - 100;

    for (int y = 0; y < result.height(); ++y) {
        for (int x = 0; x < result.width(); ++x) {
            QColor color = result.pixelColor(x, y);

            // 应用亮度
            if (brightnessFactor != 1.0) {
                color.setHsv(
                    qBound(0, static_cast<int>(color.hue()), 359),
                    color.saturation(),
                    qBound(0, static_cast<int>(color.value() * brightnessFactor), 255),
                    color.alpha()
                );
            }

            // 应用对比度
            if (contrastFactor != 0.0) {
                int red = color.red();
                int green = color.green();
                int blue = color.blue();

                red = qBound(0, static_cast<int>((red - 128) * (1.0 + contrastFactor) + 128), 255);
                green = qBound(0, static_cast<int>((green - 128) * (1.0 + contrastFactor) + 128), 255);
                blue = qBound(0, static_cast<int>((blue - 128) * (1.0 + contrastFactor) + 128), 255);

                color.setRgb(red, green, blue, color.alpha());
            }

            // 应用饱和度
            if (saturationFactor != 1.0) {
                int hue = color.hue();
                int lightness = qGray(color.red(), color.green(), color.blue());
                int saturation = qBound(0, static_cast<int>((color.saturation()) * saturationFactor), 255);

                color.setHsv(hue, saturation, lightness, color.alpha());
            }

            // 应用色相旋转
            if (hueRotation != 0.0) {
                int hue = color.hue();
                hue = (hue + static_cast<int>(hueRotation)) % 360;
                if (hue < 0) hue += 360;
                color.setHsv(hue, color.saturation(), color.value(), color.alpha());
            }

            result.setPixelColor(x, y, color);
        }
    }

    return result;
}
