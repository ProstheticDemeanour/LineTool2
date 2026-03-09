#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// widgets/TowerGeometryWidget.h
//
// Custom QWidget that renders a schematic tower cross-section in real time.
// Phase conductors are drawn as filled circles with labels (A, B, C).
// OHEW conductors are drawn as smaller crossed circles.
// The tower structure is drawn as a simple lattice outline.
// ─────────────────────────────────────────────────────────────────────────────
#include <QWidget>
#include "core/TransmissionLine.h"

namespace LineTool {

class TowerGeometryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit TowerGeometryWidget(QWidget* parent = nullptr);

    void setGeometry(const GeometryInput& g);

    QSize sizeHint() const override { return {280, 300}; }
    QSize minimumSizeHint() const override { return {160, 180}; }

protected:
    void paintEvent(QPaintEvent*) override;

private:
    GeometryInput m_geom;

    // Map world coords → widget coords
    QPointF worldToWidget(double x, double y) const;

    void drawPhase (QPainter& p, QPointF pos, const QString& label, const QColor& color);
    void drawOhew  (QPainter& p, QPointF pos);
    void drawTower (QPainter& p);
    void drawGround(QPainter& p);
    void drawDistanceAnnotations(QPainter& p);

    // World bounding box (m)
    double m_xMin = -10, m_xMax = 10;
    double m_yMin =  0,  m_yMax = 20;
    int    m_margin = 24;
};

} // namespace LineTool
