// ─────────────────────────────────────────────────────────────────────────────
// widgets/TowerGeometryWidget.cpp
// ─────────────────────────────────────────────────────────────────────────────
#include "TowerGeometryWidget.h"
#include <QPainter>
#include <QFont>
#include <algorithm>
#include <cmath>
#include <limits>

namespace LineTool {

TowerGeometryWidget::TowerGeometryWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(160, 180);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

// ─────────────────────────────────────────────────────────────────────────────
void TowerGeometryWidget::setGeometry(const GeometryInput& g)
{
    m_geom   = g;
    m_isDual = false;
    recomputeBounds(
        {g.x1, g.x2, g.x3, g.xo1, g.xo2},
        {g.y1, g.y2, g.y3, g.yo1, g.yo2});
    update();
}

void TowerGeometryWidget::setDualGeometry(const DualCircuitInput& g)
{
    m_dual   = g;
    m_geom   = g;   // base slice for OHEWs
    m_isDual = true;
    recomputeBounds(
        {g.x1, g.x2, g.x3, g.x4, g.x5, g.x6, g.xo1, g.xo2},
        {g.y1, g.y2, g.y3, g.y4, g.y5, g.y6, g.yo1, g.yo2});
    update();
}

// ─────────────────────────────────────────────────────────────────────────────
void TowerGeometryWidget::recomputeBounds(std::initializer_list<double> xs,
                                           std::initializer_list<double> ys)
{
    m_xMin = *std::min_element(xs.begin(), xs.end()) - 3.0;
    m_xMax = *std::max_element(xs.begin(), xs.end()) + 3.0;
    m_yMin = -1.0;
    m_yMax = *std::max_element(ys.begin(), ys.end()) + 3.0;
}

// ─────────────────────────────────────────────────────────────────────────────
QPointF TowerGeometryWidget::worldToWidget(double x, double y) const
{
    int w = width()  - 2*m_margin;
    int h = height() - 2*m_margin;
    double px = m_margin + (x - m_xMin) / (m_xMax - m_xMin) * w;
    double py = m_margin + (1.0 - (y - m_yMin) / (m_yMax - m_yMin)) * h;
    return {px, py};
}

// ─────────────────────────────────────────────────────────────────────────────
void TowerGeometryWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawGround(p);

    if (m_isDual)
        drawDualTower(p);
    else
        drawSingleTower(p);
}

// ─────────────────────────────────────────────────────────────────────────────
void TowerGeometryWidget::drawGround(QPainter& p)
{
    QPointF g0 = worldToWidget(m_xMin, 0);
    QPointF g1 = worldToWidget(m_xMax, 0);
    p.setPen(QPen(QColor(100,80,60), 2));
    p.drawLine(g0, g1);
    p.setPen(QPen(QColor(120,100,80), 1));
    int n = (int)((g1.x() - g0.x()) / 8);
    for (int i = 0; i <= n; ++i) {
        double x = g0.x() + i * 8.0;
        p.drawLine(QPointF(x, g0.y()), QPointF(x-5, g0.y()+6));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void TowerGeometryWidget::drawPhase(QPainter& p, QPointF pos,
                                     const QString& label, const QColor& color)
{
    const int r = 9;
    p.setPen(QPen(color.darker(130), 2));
    p.setBrush(color.lighter(150));
    p.drawEllipse(pos, r, r);
    p.setPen(color.darker(160));
    QFont f = p.font();
    f.setBold(true);
    f.setPointSize(7);
    p.setFont(f);
    p.drawText(QRectF(pos.x()-r, pos.y()-r, 2*r, 2*r), Qt::AlignCenter, label);
}

void TowerGeometryWidget::drawOhew(QPainter& p, QPointF pos)
{
    const int r = 6;
    p.setPen(QPen(QColor(50,50,50), 1.5));
    p.setBrush(QColor(200,200,200));
    p.drawEllipse(pos, r, r);
    p.drawLine(pos + QPointF(-r+1,-r+1), pos + QPointF(r-1, r-1));
    p.drawLine(pos + QPointF( r-1,-r+1), pos + QPointF(-r+1,r-1));
}

// ─────────────────────────────────────────────────────────────────────────────
// Single-circuit tower
// ─────────────────────────────────────────────────────────────────────────────
void TowerGeometryWidget::drawSingleTower(QPainter& p)
{
    const GeometryInput& g = m_geom;

    double cx   = (g.x1 + g.x2 + g.x3) / 3.0;
    double topY = m_yMax - 1.5;

    // Mast
    p.setPen(QPen(QColor(90,90,90), 2));
    p.drawLine(worldToWidget(cx, 0), worldToWidget(cx, topY));

    // Crossarms
    p.setPen(QPen(QColor(110,110,110), 1, Qt::DashLine));
    double hts[] = {g.y1, g.y2, g.y3};
    double xs[]  = {g.x1, g.x2, g.x3};
    for (int i = 0; i < 3; ++i)
        p.drawLine(worldToWidget(cx, hts[i]), worldToWidget(xs[i], hts[i]));

    // OHEWs
    p.setPen(QPen(QColor(80,80,80), 1, Qt::DotLine));
    if (g.hasOhew1) p.drawLine(worldToWidget(cx,g.yo1), worldToWidget(g.xo1,g.yo1));
    if (g.hasOhew2) p.drawLine(worldToWidget(cx,g.yo2), worldToWidget(g.xo2,g.yo2));

    // Phase circles
    static const QColor cols[3] = {
        QColor(220, 50, 50), QColor(50,150,220), QColor(50,180,50)
    };
    drawPhase(p, worldToWidget(g.x1,g.y1), "A", cols[0]);
    drawPhase(p, worldToWidget(g.x2,g.y2), "B", cols[1]);
    drawPhase(p, worldToWidget(g.x3,g.y3), "C", cols[2]);

    if (g.hasOhew1) drawOhew(p, worldToWidget(g.xo1, g.yo1));
    if (g.hasOhew2) drawOhew(p, worldToWidget(g.xo2, g.yo2));

    // D13 annotation
    double D13 = std::hypot(g.x3-g.x1, g.y3-g.y1);
    p.setPen(QPen(QColor(160,160,160), 1, Qt::DotLine));
    p.drawLine(worldToWidget(g.x1,g.y1), worldToWidget(g.x3,g.y3));
    QFont f = p.font(); f.setPointSize(6); f.setBold(false); p.setFont(f);
    p.setPen(QColor(100,100,100));
    QPointF mp = worldToWidget((g.x1+g.x3)/2, (g.y1+g.y3)/2);
    p.drawText(mp + QPointF(3,-3), QString("D13=%1m").arg(D13,0,'f',1));
}

// ─────────────────────────────────────────────────────────────────────────────
// Dual-circuit tower
// Circuit 1: left side  — A1(red), B1(blue), C1(green)  — solid colours
// Circuit 2: right side — A2(red), B2(blue), C2(green)  — lighter / hatched
// Both hang off a central H-frame or double-mast structure.
// ─────────────────────────────────────────────────────────────────────────────
void TowerGeometryWidget::drawDualTower(QPainter& p)
{
    const DualCircuitInput& g = m_dual;

    // Two mast x positions — midpoint of each circuit
    double cx1 = (g.x1 + g.x2 + g.x3) / 3.0;
    double cx2 = (g.x4 + g.x5 + g.x6) / 3.0;
    double topY = m_yMax - 1.5;

    // Mast 1
    p.setPen(QPen(QColor(90,90,90), 2));
    p.drawLine(worldToWidget(cx1, 0), worldToWidget(cx1, topY));
    // Mast 2
    p.drawLine(worldToWidget(cx2, 0), worldToWidget(cx2, topY));

    // Horizontal crossbeam at top connecting the two masts
    p.setPen(QPen(QColor(90,90,90), 2));
    p.drawLine(worldToWidget(cx1, topY), worldToWidget(cx2, topY));

    // Circuit 1 crossarms (left)
    p.setPen(QPen(QColor(110,110,110), 1, Qt::DashLine));
    double hts1[] = {g.y1, g.y2, g.y3};
    double xs1[]  = {g.x1, g.x2, g.x3};
    for (int i = 0; i < 3; ++i)
        p.drawLine(worldToWidget(cx1, hts1[i]), worldToWidget(xs1[i], hts1[i]));

    // Circuit 2 crossarms (right)
    double hts2[] = {g.y4, g.y5, g.y6};
    double xs2[]  = {g.x4, g.x5, g.x6};
    for (int i = 0; i < 3; ++i)
        p.drawLine(worldToWidget(cx2, hts2[i]), worldToWidget(xs2[i], hts2[i]));

    // OHEWs (hang from top crossbeam midpoint)
    double cxm = (cx1 + cx2) / 2.0;
    p.setPen(QPen(QColor(80,80,80), 1, Qt::DotLine));
    if (g.hasOhew1) p.drawLine(worldToWidget(cxm,g.yo1), worldToWidget(g.xo1,g.yo1));
    if (g.hasOhew2) p.drawLine(worldToWidget(cxm,g.yo2), worldToWidget(g.xo2,g.yo2));

    // Circuit 1 phases — solid
    static const QColor cols[3] = {
        QColor(220, 50, 50), QColor(50,150,220), QColor(50,180,50)
    };
    drawPhase(p, worldToWidget(g.x1,g.y1), "A1", cols[0]);
    drawPhase(p, worldToWidget(g.x2,g.y2), "B1", cols[1]);
    drawPhase(p, worldToWidget(g.x3,g.y3), "C1", cols[2]);

    // Circuit 2 phases — lighter tint to distinguish
    static const QColor cols2[3] = {
        QColor(255,160,160), QColor(160,210,255), QColor(160,240,160)
    };
    drawPhase(p, worldToWidget(g.x4,g.y4), "A2", cols2[0]);
    drawPhase(p, worldToWidget(g.x5,g.y5), "B2", cols2[1]);
    drawPhase(p, worldToWidget(g.x6,g.y6), "C2", cols2[2]);

    if (g.hasOhew1) drawOhew(p, worldToWidget(g.xo1, g.yo1));
    if (g.hasOhew2) drawOhew(p, worldToWidget(g.xo2, g.yo2));

    // Inter-circuit distance annotation (A1-A2)
    double Da1a2 = std::hypot(g.x4-g.x1, g.y4-g.y1);
    p.setPen(QPen(QColor(180,140,180), 1, Qt::DotLine));
    p.drawLine(worldToWidget(g.x1,g.y1), worldToWidget(g.x4,g.y4));
    QFont f = p.font(); f.setPointSize(6); f.setBold(false); p.setFont(f);
    p.setPen(QColor(120,80,120));
    QPointF mp = worldToWidget((g.x1+g.x4)/2, (g.y1+g.y4)/2);
    p.drawText(mp + QPointF(3,-3), QString("Da1a2=%1m").arg(Da1a2,0,'f',1));

    // Circuit labels
    QFont fl = p.font(); fl.setPointSize(7); fl.setBold(true); p.setFont(fl);
    p.setPen(QColor(80,80,80));
    p.drawText(worldToWidget(cx1, topY + 1.0) + QPointF(-12, 14), "Cct 1");
    p.drawText(worldToWidget(cx2, topY + 1.0) + QPointF(-12, 14), "Cct 2");
}

} // namespace LineTool
