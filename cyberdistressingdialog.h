#ifndef CYBERDISTRESSINGDIALOG_H
#define CYBERDISTRESSINGDIALOG_H

#include <QDialog>

class QGridLayout;
class QLabel;

class CyberDistressingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CyberDistressingDialog(QWidget *parent = nullptr);
    ~CyberDistressingDialog();
    void setOriginalImage(const QImage& image);

private:
    void updatePreview();
    QImage applyFilters(const QImage& origin_image);
    QImage applyFiltersForPreview(const QImage& origin_image);

    QImage m_image;
    QImage m_filteredImage;
    QGridLayout* m_grid_layout;
    QLabel* m_display_label;

    int m_resolution, m_color, m_noise, m_watermark;
    bool m_scanLine;
    bool m_board;
};

#endif // CYBERDISTRESSINGDIALOG_H
