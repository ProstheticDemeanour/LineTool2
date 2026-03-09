#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// core/TransmissionLine.h

// Must be defined before any <cmath> include on MSVC/MinGW to get M_PI etc.
#ifndef _USE_MATH_DEFINES
#  define _USE_MATH_DEFINES
#endif
//
// C++ port of transmission.py — all per-unit-length calculations plus
// derived system quantities (SIL, admittance, loadability, etc.)
// ─────────────────────────────────────────────────────────────────────────────

#include <complex>
#include <stdexcept>
#include <cmath>

namespace LineTool {

// ── Fundamental constants ─────────────────────────────────────────────────────
constexpr double kSpeedOfLight = 299'792'458.0;   // m/s
constexpr double kMu0          = 4e-7 * M_PI;      // H/m
constexpr double kEps0         = 8.854187817e-12;  // F/m
constexpr double kTwoPi        = 2.0 * M_PI;

using cdouble = std::complex<double>;

// ─────────────────────────────────────────────────────────────────────────────
// Struct: LineRLGC  — distributed per-unit-length parameters
// ─────────────────────────────────────────────────────────────────────────────
struct LineRLGC {
    double r = 0;   // Ω/km   — AC resistance
    double l = 0;   // H/km   — inductance
    double g = 0;   // S/km   — conductance (usually ~0)
    double c = 0;   // F/km   — capacitance
};

// ─────────────────────────────────────────────────────────────────────────────
// Struct: LineKZc — propagation constant & characteristic impedance
// ─────────────────────────────────────────────────────────────────────────────
struct LineKZc {
    cdouble k  = 0;   // propagation constant (rad/km)
    cdouble Zc = 0;   // characteristic impedance (Ω)
};

// ─────────────────────────────────────────────────────────────────────────────
// Struct: HybridParams — ABCD transmission matrix
// ─────────────────────────────────────────────────────────────────────────────
struct HybridParams {
    cdouble A, B, C, D;
};

// ─────────────────────────────────────────────────────────────────────────────
// Struct: LineResults — all computed outputs shown in the UI
// ─────────────────────────────────────────────────────────────────────────────
struct LineResults {
    // Geometry-derived
    double inductance_mH_km    = 0;   // mH/km
    double capacitance_uF_km   = 0;   // µF/km
    double admittance_S        = 0;   // total shunt admittance (S) over length
    double reactance_ohm_km    = 0;   // Ω/km  (ωL)
    double susceptance_S_km    = 0;   // S/km  (ωC)

    // Characteristic
    double Zc_ohm              = 0;   // Ω
    double k_rad_km            = 0;   // rad/km
    double vel_factor          = 0;   // fraction of c

    // Power system
    double SIL_MVA             = 0;   // MW (lossless approx.)
    double quarter_wave_km     = 0;   // km
    double charging_MVAR       = 0;   // MVAR total charging (both ends)

    // Loadability (at thermal limit I_max)
    double loadability_MW      = 0;

    // ABCD (magnitudes for display)
    double A_mag = 0, B_mag = 0, C_mag = 0, D_mag = 0;
    double A_ang = 0, B_ang = 0, C_ang = 0, D_ang = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// Struct: GeometryInput — tower geometry + conductor spec
// ─────────────────────────────────────────────────────────────────────────────
struct GeometryInput {
    // Phase conductor positions (x, y) in metres
    double x1 = -4.0, y1 = 10.0;
    double x2 =  0.0, y2 = 12.0;
    double x3 =  4.0, y3 = 10.0;

    // OHEW (earth wire) positions — up to 2
    bool   hasOhew1 = false;
    double xo1 = -3.0, yo1 = 16.0;
    bool   hasOhew2 = false;
    double xo2 =  3.0, yo2 = 16.0;

    // Conductor geometry
    double DS          = 0.0117;  // GMR (m) — from conductor DB
    int    bundleNo    = 1;       // conductors per phase
    double bundleSpace = 0.40;    // bundle spacing (m)

    // Circuit
    double r_ac        = 0.05;    // Ω/km  — from conductor DB
    double freq        = 50.0;    // Hz
    double lengthKm    = 100.0;   // km
    double voltageKV   = 220.0;   // kV (line-to-line)
    double currentA    = 500.0;   // A   (for loadability)
};

// ─────────────────────────────────────────────────────────────────────────────
// TransmissionLine — pure static computation class
// ─────────────────────────────────────────────────────────────────────────────
class TransmissionLine
{
public:
    // ── Geometry → L, C ───────────────────────────────────────────────────────
    static double calcInductance(double D12, double D23, double D13,
                                 double DS, int bundleNo, double bundleSpace);

    static double calcCapacitance(double D12, double D23, double D13,
                                  double DS, int bundleNo, double bundleSpace);

    // ── L,C,R,freq → k, Zc ───────────────────────────────────────────────────
    static LineKZc rlgcToKZc(const LineRLGC& rlgc, double freq);

    // ── k, Zc, freq → RLGC ───────────────────────────────────────────────────
    static LineRLGC kZcToRlgc(const LineKZc& kzc, double freq);

    // ── ABCD at distance d ────────────────────────────────────────────────────
    static HybridParams hybrid(cdouble k, cdouble Zc, double d_km);

    // ── SIL ──────────────────────────────────────────────────────────────────
    static double sil_MVA(double voltage_kV, double Zc_ohm);

    // ── Quarter-wave length ───────────────────────────────────────────────────
    static double quarterWaveKm(double freq);

    // ── Charging power ────────────────────────────────────────────────────────
    static double chargingMVAR(double voltage_kV, double C_uF_km,
                                double length_km, double freq);

    // ── Top-level: geometry → all results ─────────────────────────────────────
    static LineResults compute(const GeometryInput& g);

private:
    static double gmd(double D12, double D23, double D13);
    static double gmrBundle(double DS, int bundleNo, double bundleSpace);
};

} // namespace LineTool
