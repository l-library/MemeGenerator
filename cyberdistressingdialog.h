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
    QImage getFilteredCopy();

private:
    void updatePreview();
    void applyFilters();

    void applyScanLine();
    void undoScanLine();

    QImage m_image;
    QImage m_filteredImage;
    QImage m_baseImage;
    QGridLayout* m_grid_layout;
    QLabel* m_display_label;

    int m_resolution, m_color, m_noise, m_watermark;
    bool m_scanLine;
    bool m_board;

    const int m_OutputSpacing = 2;
    const int m_ScanLineHeight = 1;
};

#endif // CYBERDISTRESSINGDIALOG_H
