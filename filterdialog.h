#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>
#include <QImage>

// 提前声明
class QGridLayout;
class QLabel;

namespace Ui {
class FilterDialog;
}

class FilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilterDialog(QWidget *parent = nullptr);
    ~FilterDialog();

    void setOriginalImage(const QImage& image);
    QImage getFilteredCopy() const;

private slots:
    void updatePreview();
    void onApplyClicked();

private:
    Ui::FilterDialog *ui;
    QGridLayout* m_grid_layout; // 对话框布局
    QLabel* m_display_label;    // 效果预览

    // 各参数
    int m_brightness, m_saturation, m_contrast, m_hue;

    QImage m_originalImage;  // 原始图像
    QImage m_filteredImage;  // 滤镜处理后的图像

    QImage applyFilters(const QImage& image) const;
};

#endif // FILTERDIALOG_H
