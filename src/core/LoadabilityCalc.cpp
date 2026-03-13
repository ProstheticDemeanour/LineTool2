// ─────────────────────────────────────────────────────────────────────────────
// core/LoadabilityCalc.cpp
// ─────────────────────────────────────────────────────────────────────────────
#ifndef _USE_MATH_DEFINES
#  define _USE_MATH_DEFINES
#endif
#include "LoadabilityCalc.h"
#include "TransmissionLine.h"
#include <complex>
#include <cmath>
#include <QVector>

namespace LineTool {

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────
QVector<double> LoadabilityCalc::arange(double start, double end, double step)
{
    QVector<double> v;
    for (double x = start; x <= end + 1e-9; x += step)
        v.append(x);
    return v;
}

QVector<double> LoadabilityCalc::linspace(double start, double end, int n)
{
    QVector<double> v;
    if (n <= 1) { v.append(start); return v; }
    double step = (end - start) / (n - 1);
    for (int i = 0; i < n; ++i)
        v.append(start + i * step);
    return v;
}

// ─────────────────────────────────────────────────────────────────────────────
// stClairFactor
// Empirical Dunlop-Gutman (1977) curve.
// Returns loadability in per-unit of SIL.
// Valid for length >= 50 km; below 50 km the thermal limit governs anyway.
// ─────────────────────────────────────────────────────────────────────────────
double LoadabilityCalc::stClairFactor(double length_km)
{
    if (length_km < 1.0) length_km = 1.0;
    return 49.605 * std::pow(length_km, -0.634);
}

// ─────────────────────────────────────────────────────────────────────────────
// compute
// ─────────────────────────────────────────────────────────────────────────────
LoadabilityResult LoadabilityCalc::compute(const LoadabilityInput& in)
{
    LoadabilityResult res;

    // ── RLGC → k, Zc ─────────────────────────────────────────────────────────
    LineRLGC rlgc;
    rlgc.r = in.r_ac;
    rlgc.l = in.L_mH_km  * 1e-3;   // H/km
    rlgc.g = 0.0;
    rlgc.c = in.C_uF_km  * 1e-6;   // F/km

    LineKZc kzc = TransmissionLine::rlgcToKZc(rlgc, in.freq);
    res.Zc_ohm      = kzc.Zc.real();
    res.beta_rad_km = kzc.k.real();   // phase constant β (rad/km)

    // ── Wavelength ────────────────────────────────────────────────────────────
    if (res.beta_rad_km > 0) {
        res.lambda_km       = 2.0 * M_PI / res.beta_rad_km;
        res.quarter_wave_km = res.lambda_km / 4.0;
    } else {
        res.quarter_wave_km = TransmissionLine::quarterWaveKm(in.freq);
        res.lambda_km       = res.quarter_wave_km * 4.0;
    }

    // ── SIL ───────────────────────────────────────────────────────────────────
    if (res.Zc_ohm > 0)
        res.SIL_MVA = TransmissionLine::sil_MVA(in.Vrated_kV, res.Zc_ohm);

    // ── Thermal limit ─────────────────────────────────────────────────────────
    res.P_thermal_MW = std::sqrt(3.0) * in.Vrated_kV * in.I_th_A * 1e-3;

    // ── x-axis: 0 to min(maxLen, λ/4) in 10 km steps ─────────────────────────
    double xMax = std::min((double)in.maxLen_km, res.quarter_wave_km * 0.999);
    QVector<double> xAll = arange(10, xMax, 10);

    // ── Precompute Kc ─────────────────────────────────────────────────────────
    // Kc = Vs_pu * Vr_pu * SIL  [MW]
    double Vs_pu = in.Vs_kV / in.Vrated_kV;
    double Vr_pu = in.Vr_kV / in.Vrated_kV;
    double Kc    = Vs_pu * Vr_pu * res.SIL_MVA;

    double delta_rad = in.delta_deg * M_PI / 180.0;

    // ── Build curves ──────────────────────────────────────────────────────────
    res.thermal.name   = in.label + " Thermal";
    res.stability.name = in.label + " Stability limit";
    res.practical.name = in.label + " Practical (δ=" +
                         QString::number(in.delta_deg,'f',0) + "°)";
    res.stclair.name   = in.label + " St. Clair";
    res.sil.name       = in.label + " SIL";

    for (double l : xAll) {
        double bl = res.beta_rad_km * l;
        double sinbl = std::sin(bl);

        // Thermal — flat
        res.thermal.x.append(l);
        res.thermal.y.append(res.P_thermal_MW);

        // Theoretical stability (delta=90°)
        if (std::abs(sinbl) > 1e-6) {
            res.stability.x.append(l);
            res.stability.y.append(Kc / sinbl);   // sin(90°)=1
        }

        // Practical loadability (delta=delta_deg)
        if (std::abs(sinbl) > 1e-6) {
            res.practical.x.append(l);
            res.practical.y.append(Kc * std::sin(delta_rad) / sinbl);
        }

        // St. Clair — only meaningful from 50 km
        if (l >= 50) {
            res.stclair.x.append(l);
            res.stclair.y.append(res.SIL_MVA * stClairFactor(l));
        }

        // SIL — flat
        res.sil.x.append(l);
        res.sil.y.append(res.SIL_MVA);
    }

    return res;
}

} // namespace LineTool
