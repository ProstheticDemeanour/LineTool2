#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// widgets/LoadabilityChartWidget.h
//
// Custom-painted loadability chart — pure QPainter, no QtCharts.
// Background is always palette().color(QPalette::Window) — seamless with app.
// ─────────────────────────────────────────────────────────────────────────────
#include <QWidget>
#include <QVector>
#include <QStringList>
#include "core/LoadabilityCalc.h"

namespace LineTool {

class LoadabilityChartWidget : public QWidget
{
    Q_OBJECT
public:
    explicit LoadabilityChartWidget(QWidget* parent = nullptr);

    void setResults(const QVector<LoadabilityResult>& results);

    // Optional: pass line labels for the legend (same order as results)
    void setLabels(const QStringList& labels) { m_cards_labels = labels; update(); }

    void setThermalVisible  (bool v);
    void setStabilityVisible(bool v);
    void setPracticalVisible(bool v);
    void setStClairVisible  (bool v);
    void setSILVisible      (bool v);

    QSize sizeHint()        const override { return {700, 480}; }
    QSize minimumSizeHint() const override { return {300, 200}; }

protected:
    void paintEvent(QPaintEvent*) override;

private:
    struct VisibleSeries {
        const CurveSeries* data;
        QColor             color;
        float              width;
        Qt::PenStyle       style;
    };

    void collectVisible(QVector<VisibleSeries>& out,
                        const LoadabilityResult& r, int paletteIdx) const;

    static double niceStep(double range, int ticks);

    QVector<LoadabilityResult> m_results;
    QStringList                m_cards_labels;

    bool m_showThermal   = true;
    bool m_showStability = true;
    bool m_showPractical = true;
    bool m_showStClair   = true;
    bool m_showSIL       = true;
};

} // namespace LineTool
