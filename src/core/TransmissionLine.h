#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// core/TransmissionLine.h

#ifndef _USE_MATH_DEFINES
#  define _USE_MATH_DEFINES
#endif

#include <complex>
#include <stdexcept>
#include <cmath>

namespace LineTool {

// ── Fundamental constants ─────────────────────────────────────────────────────
constexpr double kSpeedOfLight = 299'792'458.0;
constexpr double kMu0          = 4e-7 * M_PI;
constexpr double kEps0         = 8.854187817e-12;
constexpr double kTwoPi        = 2.0 * M_PI;

using cdouble = std::complex<double>;

struct LineRLGC {
    double r = 0;
    double l = 0;
    double g = 0;
    double c = 0;
};

struct LineKZc {
    cdouble k  = 0;
    cdouble Zc = 0;
};

struct HybridParams {
    cdouble A, B, C, D;
};

struct LineResults {
    double inductance_mH_km   = 0;
    double capacitance_uF_km  = 0;
    double admittance_S       = 0;
    double reactance_ohm_km   = 0;
    double susceptance_S_km   = 0;
    double Zc_ohm             = 0;
    double k_rad_km           = 0;
    double vel_factor         = 0;
    double SIL_MVA            = 0;
    double quarter_wave_km    = 0;
    double charging_MVAR      = 0;
    double loadability_MW     = 0;
    double A_mag=0, B_mag=0, C_mag=0, D_mag=0;
    double A_ang=0, B_ang=0, C_ang=0, D_ang=0;
};

struct GeometryInput {
    double x1 = -4.0, y1 = 10.0;
    double x2 =  0.0, y2 = 12.0;
    double x3 =  4.0, y3 = 10.0;
    bool   hasOhew1 = false;
    double xo1 = -3.0, yo1 = 16.0;
    bool   hasOhew2 = false;
    double xo2 =  3.0, yo2 = 16.0;
    double DS          = 0.0117;
    int    bundleNo    = 1;
    double bundleSpace = 0.40;
    double r_ac      = 0.05;
    double freq      = 50.0;
    double lengthKm  = 100.0;
    double voltageKV = 220.0;
    double currentA  = 500.0;
};

// ─────────────────────────────────────────────────────────────────────────────
// DualCircuitInput
// Circuit 2 phases: x4/y4=A2, x5/y5=B2, x6/y6=C2
// Both circuits assumed same conductor and bundle.
//
// Equivalent positive-sequence parameters:
//   DS_b  = gmrBundle(DS, bundleNo, bundleSpace)
//   GMRa  = sqrt(DS_b * Da1a2)
//   GMRb  = sqrt(DS_b * Db1b2)
//   GMRc  = sqrt(DS_b * Dc1c2)
//   GMRl  = cbrt(GMRa * GMRb * GMRc)
//   Dab   = (Da1b1 * Da1b2 * Da2b1 * Da2b2)^(1/4)
//   Dbc   = (Db1c1 * Db1c2 * Db2c1 * Db2c2)^(1/4)
//   Dac   = (Da1c1 * Da1c2 * Da2c1 * Da2c2)^(1/4)
//   GMD   = cbrt(Dab * Dbc * Dac)
//   L     = 0.2 * ln(GMD / GMRl)   mH/km
// ─────────────────────────────────────────────────────────────────────────────
struct DualCircuitInput : public GeometryInput {
    double x4 =  4.0, y4 = 10.0;   // A2
    double x5 =  0.0, y5 = 12.0;   // B2
    double x6 = -4.0, y6 = 10.0;   // C2
};

struct DualCircuitResults {
    // Geometry intermediates
    double GMD_m     = 0;
    double GMRl_m    = 0;
    double DSb_m     = 0;
    double Da1a2_m   = 0;
    double Db1b2_m   = 0;
    double Dc1c2_m   = 0;
    // Per-unit-length
    double inductance_mH_km  = 0;
    double capacitance_uF_km = 0;
    double reactance_ohm_km  = 0;
    double susceptance_S_km  = 0;
    // Characteristic
    double Zc_ohm     = 0;
    double k_rad_km   = 0;
    double vel_factor = 0;
    // Power (per circuit; total = 2x)
    double SIL_MVA        = 0;
    double SIL_total_MVA  = 0;
    double quarter_wave_km= 0;
    double charging_MVAR  = 0;
    double loadability_MW = 0;
    // ABCD
    double A_mag=0, B_mag=0, C_mag=0, D_mag=0;
    double A_ang=0, B_ang=0, C_ang=0, D_ang=0;
};

// ─────────────────────────────────────────────────────────────────────────────
// TransmissionLine
// ─────────────────────────────────────────────────────────────────────────────
class TransmissionLine
{
public:
    static double calcInductance (double D12, double D23, double D13,
                                  double DS, int bundleNo, double bundleSpace);
    static double calcCapacitance(double D12, double D23, double D13,
                                  double DS, int bundleNo, double bundleSpace);
    static LineKZc      rlgcToKZc(const LineRLGC& rlgc, double freq);
    static LineRLGC     kZcToRlgc(const LineKZc& kzc,   double freq);
    static HybridParams hybrid   (cdouble k, cdouble Zc, double d_km);
    static double       sil_MVA  (double voltage_kV, double Zc_ohm);
    static double       quarterWaveKm(double freq);
    static double       chargingMVAR (double voltage_kV, double C_uF_km,
                                      double length_km,  double freq);
    static LineResults        compute    (const GeometryInput& g);
    static DualCircuitResults computeDual(const DualCircuitInput& g);

    // Public — used by UI for display
    static double gmrBundle(double DS, int bundleNo, double bundleSpace);

private:
    static double gmd(double D12, double D23, double D13);
};

} // namespace LineTool
