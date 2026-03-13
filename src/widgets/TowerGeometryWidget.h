#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// widgets/TowerGeometryWidget.h
//
// Draws a tower cross-section schematic.
// setGeometry()     → single-circuit mode  (3 phases + up to 2 OHEWs)
// setDualGeometry() → dual-circuit mode    (6 phases + up to 2 OHEWs)
// The widget redraws on every call; no Calculate needed.
// ─────────────────────────────────────────────────────────────────────────────
#include <QWidget>
#include "core/TransmissionLine.h"

namespace LineTool {

class TowerGeometryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TowerGeometryWidget(QWidget* parent = nullptr);

    void setGeometry    (const GeometryInput&      g);
    void setDualGeometry(const DualCircuitInput&   g);

    QSize sizeHint()        const override { return {280, 300}; }
    QSize minimumSizeHint() const override { return {160, 180}; }

protected:
    void paintEvent(QPaintEvent*) override;

private:
    // Common helpers
    QPointF worldToWidget(double x, double y) const;
    void drawGround (QPainter& p);
    void drawPhase  (QPainter& p, QPointF pos, const QString& label,
                     const QColor& color);
    void drawOhew   (QPainter& p, QPointF pos);

    // Single-circuit draw
    void drawSingleTower(QPainter& p);

    // Dual-circuit draw
    void drawDualTower(QPainter& p);

    // World bounds
    void recomputeBounds(std::initializer_list<double> xs,
                         std::initializer_list<double> ys);

    GeometryInput    m_geom;
    DualCircuitInput m_dual;
    bool             m_isDual = false;

    double m_xMin = -10, m_xMax = 10;
    double m_yMin =   0, m_yMax = 20;
    int    m_margin = 24;
};

} // namespace LineTool
