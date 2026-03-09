// ─────────────────────────────────────────────────────────────────────────────
// widgets/TowerGeometryWidget.cpp
// ─────────────────────────────────────────────────────────────────────────────
#include "TowerGeometryWidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QFont>
#include <algorithm>
#include <cmath>

namespace LineTool {

TowerGeometryWidget::TowerGeometryWidget(QWidget* parent)
    : QWidget(parent)
{
    setMinimumSize(160, 180);
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void TowerGeometryWidget::setGeometry(const GeometryInput& g)
{
    m_geom = g;

    // Recompute world bounds with padding
    double xs[] = {g.x1, g.x2, g.x3, g.xo1, g.xo2};
    double ys[] = {g.y1, g.y2, g.y3, g.yo1, g.yo2};
    m_xMin = *std::min_element(xs, xs+5) - 3.0;
    m_xMax = *std::max_element(xs, xs+5) + 3.0;
    m_yMin = -1.0;
    m_yMax = *std::max_element(ys, ys+5) + 3.0;

    update();
}

QPointF TowerGeometryWidget::worldToWidget(double x, double y) const
{
    int w = width()  - 2*m_margin;
    int h = height() - 2*m_margin;
    double px = m_margin + (x - m_xMin) / (m_xMax - m_xMin) * w;
    double py = m_margin + (1.0 - (y - m_yMin) / (m_yMax - m_yMin)) * h;
    return {px, py};
}

void TowerGeometryWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    drawGround(p);
    drawTower(p);
    drawDistanceAnnotations(p);

    // OHEWs
    if (m_geom.hasOhew1) drawOhew(p, worldToWidget(m_geom.xo1, m_geom.yo1));
    if (m_geom.hasOhew2) drawOhew(p, worldToWidget(m_geom.xo2, m_geom.yo2));

    // Phases
    static const QColor cols[3] = {
        QColor(220,  50,  50),   // A — red
        QColor( 50, 150, 220),   // B — blue
        QColor( 50, 180,  50),   // C — green
    };
    drawPhase(p, worldToWidget(m_geom.x1, m_geom.y1), "A", cols[0]);
    drawPhase(p, worldToWidget(m_geom.x2, m_geom.y2), "B", cols[1]);
    drawPhase(p, worldToWidget(m_geom.x3, m_geom.y3), "C", cols[2]);
}

void TowerGeometryWidget::drawGround(QPainter& p)
{
    QPointF g0 = worldToWidget(m_xMin, 0);
    QPointF g1 = worldToWidget(m_xMax, 0);
    p.setPen(QPen(QColor(100,80,60), 2));
    p.drawLine(g0, g1);

    // Hatching
    p.setPen(QPen(QColor(120,100,80), 1));
    int n = (int)((g1.x() - g0.x()) / 8);
    for (int i = 0; i <= n; ++i) {
        double x = g0.x() + i * 8.0;
        p.drawLine(QPointF(x, g0.y()), QPointF(x - 5, g0.y() + 6));
    }
}

void TowerGeometryWidget::drawTower(QPainter& p)
{
    // Simple lattice tower: vertical mast + crossarms at each phase height
    QPen mast(QColor(90,90,90), 2);
    p.setPen(mast);

    double cx = (m_geom.x1 + m_geom.x2 + m_geom.x3) / 3.0;
    double topY = m_yMax - 1.5;

    QPointF base  = worldToWidget(cx, 0);
    QPointF top   = worldToWidget(cx, topY);
    p.drawLine(base, top);

    // Crossarms at each conductor height
    double heights[] = {m_geom.y1, m_geom.y2, m_geom.y3};
    double xCoords[]  = {m_geom.x1, m_geom.x2, m_geom.x3};

    p.setPen(QPen(QColor(110,110,110), 1, Qt::DashLine));
    for (int i = 0; i < 3; ++i) {
        QPointF mpt = worldToWidget(cx, heights[i]);
        QPointF ept = worldToWidget(xCoords[i], heights[i]);
        p.drawLine(mpt, ept);
    }

    // OHEW arms
    if (m_geom.hasOhew1) {
        QPointF mpt = worldToWidget(cx, m_geom.yo1);
        QPointF ept = worldToWidget(m_geom.xo1, m_geom.yo1);
        p.setPen(QPen(QColor(80,80,80), 1, Qt::DotLine));
        p.drawLine(mpt, ept);
    }
    if (m_geom.hasOhew2) {
        QPointF mpt = worldToWidget(cx, m_geom.yo2);
        QPointF ept = worldToWidget(m_geom.xo2, m_geom.yo2);
        p.setPen(QPen(QColor(80,80,80), 1, Qt::DotLine));
        p.drawLine(mpt, ept);
    }
}

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
    QRectF tr(pos.x() - r, pos.y() - r, 2*r, 2*r);
    p.drawText(tr, Qt::AlignCenter, label);
}

void TowerGeometryWidget::drawOhew(QPainter& p, QPointF pos)
{
    const int r = 6;
    p.setPen(QPen(QColor(50,50,50), 1.5));
    p.setBrush(QColor(200,200,200));
    p.drawEllipse(pos, r, r);
    p.drawLine(pos + QPointF(-r+1, -r+1), pos + QPointF(r-1,  r-1));
    p.drawLine(pos + QPointF( r-1, -r+1), pos + QPointF(-r+1, r-1));
}

void TowerGeometryWidget::drawDistanceAnnotations(QPainter& p)
{
    // Draw D12, D23, D13 as dashed lines between phases
    auto dist = [](double x1,double y1,double x2,double y2){
        return std::hypot(x2-x1, y2-y1);
    };
    double D12 = dist(m_geom.x1,m_geom.y1,m_geom.x2,m_geom.y2);
    double D13 = dist(m_geom.x1,m_geom.y1,m_geom.x3,m_geom.y3);

    p.setPen(QPen(QColor(160,160,160), 1, Qt::DotLine));
    p.drawLine(worldToWidget(m_geom.x1,m_geom.y1),
               worldToWidget(m_geom.x3,m_geom.y3));

    QFont f = p.font();
    f.setBold(false);
    f.setPointSize(6);
    p.setFont(f);
    p.setPen(QColor(100,100,100));

    // Midpoint label for D13
    double mx = (m_geom.x1+m_geom.x3)/2;
    double my = (m_geom.y1+m_geom.y3)/2;
    QPointF mp = worldToWidget(mx, my);
    p.drawText(mp + QPointF(3,-3),
               QString("D13=%1m").arg(D13, 0, 'f', 1));
    (void)D12; // suppress unused warning; D12/D23 shown in outputs panel
}

} // namespace LineTool
