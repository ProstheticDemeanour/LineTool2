// ─────────────────────────────────────────────────────────────────────────────
// core/TransmissionLine.cpp
// ─────────────────────────────────────────────────────────────────────────────
#include "TransmissionLine.h"
#include <cmath>
#include <stdexcept>

namespace LineTool {

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────
double TransmissionLine::gmd(double D12, double D23, double D13)
{
    return std::cbrt(D12 * D23 * D13);
}

double TransmissionLine::gmrBundle(double DS, int bundleNo, double bundleSpace)
{
    if (bundleNo <= 1)  return DS;
    if (bundleNo == 2)  return std::sqrt(DS * bundleSpace);
    if (bundleNo == 3)  return std::cbrt(DS * bundleSpace * bundleSpace);
    // 4+ conductors
    return 1.091 * std::pow(DS * std::pow(bundleSpace, 3), 0.25);
}

// ─────────────────────────────────────────────────────────────────────────────
// Inductance  (mH/km)
// Formula: L = 0.2 * ln(GMD / GMR_bundle)   [mH/km]
// ─────────────────────────────────────────────────────────────────────────────
double TransmissionLine::calcInductance(double D12, double D23, double D13,
                                        double DS, int bundleNo, double bundleSpace)
{
    const double GMD = gmd(D12, D23, D13);
    const double DSL = gmrBundle(DS, bundleNo, bundleSpace);
    if (DSL <= 0 || GMD <= 0)
        throw std::invalid_argument("Distances must be positive");
    return 0.2 * std::log(GMD / DSL);   // mH/km
}

// ─────────────────────────────────────────────────────────────────────────────
// Capacitance  (µF/km)
// Formula: C = 0.0556 / ln(GMD / GMR_bundle)   [µF/km]
// ─────────────────────────────────────────────────────────────────────────────
double TransmissionLine::calcCapacitance(double D12, double D23, double D13,
                                         double DS, int bundleNo, double bundleSpace)
{
    const double GMD = gmd(D12, D23, D13);
    const double DSL = gmrBundle(DS, bundleNo, bundleSpace);
    if (DSL <= 0 || GMD <= 0)
        throw std::invalid_argument("Distances must be positive");
    return 0.0556 / std::log(GMD / DSL);   // µF/km
}

// ─────────────────────────────────────────────────────────────────────────────
// RLGC → k, Zc
// (lossless approximation: k = ω√(LC),  Zc = √(L/C))
// ─────────────────────────────────────────────────────────────────────────────
LineKZc TransmissionLine::rlgcToKZc(const LineRLGC& q, double freq)
{
    const double omega = kTwoPi * freq;
    // Convert stored units: L in H/km, C in F/km
    const double L = q.l;
    const double C = q.c;

    cdouble x = cdouble(q.r, omega * L);   // series impedance per km
    cdouble y = cdouble(q.g, omega * C);   // shunt admittance per km

    cdouble k  = std::sqrt(x * y);
    cdouble Zc = std::sqrt(x / y);

    if (Zc.real() < 0) Zc = -Zc;
    if (k.real()  < 0) k  = -k;

    return {k, Zc};
}

// ─────────────────────────────────────────────────────────────────────────────
// k, Zc → RLGC
// ─────────────────────────────────────────────────────────────────────────────
LineRLGC TransmissionLine::kZcToRlgc(const LineKZc& kzc, double freq)
{
    const double omega = kTwoPi * freq;
    cdouble x = kzc.k * kzc.Zc;
    cdouble y = kzc.k / kzc.Zc;

    LineRLGC q;
    q.r = x.real();
    q.l = x.imag() / omega;
    q.g = y.real();
    q.c = y.imag() / omega;
    return q;
}

// ─────────────────────────────────────────────────────────────────────────────
// ABCD / Hybrid matrix
// ─────────────────────────────────────────────────────────────────────────────
HybridParams TransmissionLine::hybrid(cdouble k, cdouble Zc, double d_km)
{
    cdouble kd = k * d_km;
    HybridParams p;
    p.A = std::cos(kd);
    p.B = Zc * std::sin(kd);
    p.C = std::sin(kd) / Zc;
    p.D = p.A;   // symmetric
    return p;
}

// ─────────────────────────────────────────────────────────────────────────────
// SIL
// ─────────────────────────────────────────────────────────────────────────────
double TransmissionLine::sil_MVA(double voltage_kV, double Zc_ohm)
{
    const double V = voltage_kV * 1e3;   // V
    return (V * V / Zc_ohm) * 1e-6;     // MVA
}

// ─────────────────────────────────────────────────────────────────────────────
// Quarter-wave length  (km)
// ─────────────────────────────────────────────────────────────────────────────
double TransmissionLine::quarterWaveKm(double freq)
{
    return (kSpeedOfLight / freq) / 4.0 / 1000.0;   // m → km
}

// ─────────────────────────────────────────────────────────────────────────────
// Total charging MVAR (both ends)
//   Q = V² × ω × C_total  = V² × ω × (C_µF_km × 1e-6) × length
// ─────────────────────────────────────────────────────────────────────────────
double TransmissionLine::chargingMVAR(double voltage_kV, double C_uF_km,
                                       double length_km, double freq)
{
    const double V     = voltage_kV * 1e3;
    const double C_tot = C_uF_km * 1e-6 * length_km;   // F
    return (V * V * kTwoPi * freq * C_tot) * 1e-6;     // MVAR
}

// ─────────────────────────────────────────────────────────────────────────────
// Master compute()
// ─────────────────────────────────────────────────────────────────────────────
LineResults TransmissionLine::compute(const GeometryInput& g)
{
    LineResults res;

    // ── Phase-to-phase distances from (x,y) coordinates ──────────────────────
    auto dist = [](double x1,double y1,double x2,double y2){
        return std::hypot(x2-x1, y2-y1);
    };
    double D12 = dist(g.x1,g.y1, g.x2,g.y2);
    double D23 = dist(g.x2,g.y2, g.x3,g.y3);
    double D13 = dist(g.x1,g.y1, g.x3,g.y3);

    // Protect against degenerate geometry
    if (D12 < 0.1) D12 = 0.1;
    if (D23 < 0.1) D23 = 0.1;
    if (D13 < 0.1) D13 = 0.1;

    // ── Geometry → L, C ───────────────────────────────────────────────────────
    res.inductance_mH_km  = calcInductance (D12, D23, D13, g.DS, g.bundleNo, g.bundleSpace);
    res.capacitance_uF_km = calcCapacitance(D12, D23, D13, g.DS, g.bundleNo, g.bundleSpace);

    const double omega = kTwoPi * g.freq;
    res.reactance_ohm_km  = omega * res.inductance_mH_km  * 1e-3;   // Ω/km
    res.susceptance_S_km  = omega * res.capacitance_uF_km * 1e-6;   // S/km

    // ── Build RLGC ─────────────────────────────────────────────────────────────
    LineRLGC rlgc;
    rlgc.r = g.r_ac;
    rlgc.l = res.inductance_mH_km  * 1e-3;   // H/km
    rlgc.g = 0.0;
    rlgc.c = res.capacitance_uF_km * 1e-6;   // F/km

    // ── k, Zc ─────────────────────────────────────────────────────────────────
    LineKZc kzc = rlgcToKZc(rlgc, g.freq);
    res.Zc_ohm   = kzc.Zc.real();
    res.k_rad_km = kzc.k.real();

    // Velocity factor  (v = ω/β,  β = k_real)
    if (res.k_rad_km > 0)
        res.vel_factor = (omega / res.k_rad_km) / kSpeedOfLight * 1e3;
    else
        res.vel_factor = 0;

    // ── SIL, λ/4, charging ────────────────────────────────────────────────────
    if (res.Zc_ohm > 0)
        res.SIL_MVA = sil_MVA(g.voltageKV, res.Zc_ohm);

    res.quarter_wave_km = quarterWaveKm(g.freq);
    res.admittance_S    = res.susceptance_S_km * g.lengthKm;
    res.charging_MVAR   = chargingMVAR(g.voltageKV, res.capacitance_uF_km,
                                        g.lengthKm, g.freq);

    // ── ABCD at full line length ───────────────────────────────────────────────
    HybridParams abcd = hybrid(kzc.k, kzc.Zc, g.lengthKm);
    res.A_mag = std::abs(abcd.A);  res.A_ang = std::arg(abcd.A) * 180.0 / M_PI;
    res.B_mag = std::abs(abcd.B);  res.B_ang = std::arg(abcd.B) * 180.0 / M_PI;
    res.C_mag = std::abs(abcd.C);  res.C_ang = std::arg(abcd.C) * 180.0 / M_PI;
    res.D_mag = std::abs(abcd.D);  res.D_ang = std::arg(abcd.D) * 180.0 / M_PI;

    // ── Loadability (approximate, using thermal current) ──────────────────────
    // P_load ≈ |Vs|·|Vr| / Zc · sin(δ) — simplified using current as proxy
    {
        const double Vn = g.voltageKV * 1e3 / std::sqrt(3.0);
        cdouble Vs = cdouble(Vn, 0) * abcd.A + cdouble(0, g.currentA) * abcd.B;
        cdouble Vr = cdouble(Vn, 0) * abcd.D - cdouble(0, g.currentA) * abcd.B;
        double delta = std::arg(Vs) - std::arg(Vr);   // rad
        double beta_L = res.k_rad_km * g.lengthKm;     // rad
        if (std::abs(std::sin(beta_L)) > 1e-6 && res.Zc_ohm > 0)
            res.loadability_MW = (std::abs(Vs)/Vn) * (std::abs(Vr)/Vn)
                                 * res.SIL_MVA
                                 * (std::sin(delta) / std::sin(beta_L));
        else
            res.loadability_MW = 0;
    }

    return res;
}

} // namespace LineTool
