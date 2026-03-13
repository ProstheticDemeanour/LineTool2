#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// core/LoadabilityCalc.h
//
// Loadability curve calculations.
//
// Three curves (all in MW, plotted vs line length in km):
//
//  1. Thermal limit
//     P_thermal = sqrt(3) * V_rated_kV * I_thermal_A * 1e-3   [MW]
//     Flat horizontal line — independent of length.
//
//  2. Theoretical stability limit  (lossless exact model)
//     P_stab = (|Vs| * |Vr| / Zc) * sin(delta) / sin(beta*l)   [MW]
//     delta = 90° at the theoretical limit (sin(delta)=1)
//     = Kc / sin(beta*l)    where Kc = Vs_pu * Vr_pu * SIL
//
//  3. Practical loadability  (delta = 30° per Saadat / Kundur convention)
//     P_load = Kc * sin(30°) / sin(beta*l)
//
//  4. St. Clair curve  (Dunlop-Gutman 1977 empirical)
//     P_stclair = SIL * 49.605 * length^(-0.634)   [MW]
//     Valid for length >= 50 km. Below 50 km it saturates at the thermal limit.
//
//  5. SIL — flat line at SIL_MVA across all lengths.
//
// Reference:
//   Saadat, H. (1999) Power Systems Analysis, McGraw-Hill  (Loadabil.m)
//   Dunlop, Gutman, Marchenko (1977) IEEE Trans. PAS-96, pp. 1044-1052
// ─────────────────────────────────────────────────────────────────────────────
#ifndef _USE_MATH_DEFINES
#  define _USE_MATH_DEFINES
#endif
#include <cmath>
#include <vector>
#include <QString>
#include <QVector>

namespace LineTool {

// ── One line's worth of input parameters ─────────────────────────────────────
struct LoadabilityInput {
    QString label;           // display name on the plot
    double  L_mH_km  = 0;   // inductance  mH/km
    double  C_uF_km  = 0;   // capacitance µF/km
    double  r_ac     = 0;   // AC resistance Ω/km (used for Zc — lossy)
    double  freq     = 50;  // Hz
    double  Vs_kV    = 220; // sending-end voltage kV L-L
    double  Vr_kV    = 220; // receiving-end voltage kV L-L
    double  Vrated_kV= 220; // rated voltage kV L-L
    double  I_th_A   = 500; // thermal current limit A
    double  delta_deg= 30;  // practical stability angle (°)
    int     maxLen_km= 600; // x-axis maximum (km)
};

// ── One curve series ─────────────────────────────────────────────────────────
struct CurveSeries {
    QString          name;
    QVector<double>  x;   // km
    QVector<double>  y;   // MW
};

// ── All results for one line ──────────────────────────────────────────────────
struct LoadabilityResult {
    double SIL_MVA      = 0;
    double Zc_ohm       = 0;
    double beta_rad_km  = 0;
    double lambda_km    = 0;   // full wavelength
    double quarter_wave_km = 0;
    double P_thermal_MW = 0;

    CurveSeries thermal;
    CurveSeries stability;
    CurveSeries practical;
    CurveSeries stclair;
    CurveSeries sil;
};

// ─────────────────────────────────────────────────────────────────────────────
class LoadabilityCalc
{
public:
    // Compute all five curve series for one line.
    static LoadabilityResult compute(const LoadabilityInput& in);

    // St. Clair factor only  (pu SIL)
    // = 49.605 * length_km^(-0.634)    for length >= 50 km
    static double stClairFactor(double length_km);

private:
    static QVector<double> linspace(double start, double end, int n);
    static QVector<double> arange (double start, double end, double step);
};

} // namespace LineTool
