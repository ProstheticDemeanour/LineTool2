# LineTool v2.0

Overhead transmission line parameter calculator built with C++ and Qt6.

Given a tower geometry and conductor selection, LineTool computes the full set of distributed-parameter line quantities used in power system analysis — inductance, capacitance, characteristic impedance, SIL, ABCD matrix, loadability, and more. Conductor electrical and mechanical data is read from a SQLite database (`conductors.db`).

---

## Building

### Prerequisites

| Platform | Requirements |
|----------|-------------|
| Windows  | Qt 6 (mingw_64), CMake ≥ 3.21, MinGW 13 |
| macOS    | Qt 6 (Homebrew or qt.io), CMake ≥ 3.21, Xcode CLT |

### Windows (MinGW)

```powershell
# Add MinGW to PATH if not already set
$env:PATH = "C:\Qt\Tools\mingw1310_64\bin;" + $env:PATH

cmake -B build -G "MinGW Makefiles" `
      -DCMAKE_BUILD_TYPE=Release `
      -DCMAKE_PREFIX_PATH=C:\Qt\6.10.2\mingw_64

cmake --build build -j4

# Place the database next to the executable
copy conductors.db build\
```

### macOS

```bash
cmake -B build -S . \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=$(brew --prefix qt)

cmake --build build -j4
cp conductors.db build/
open build/LineTool.app
```

### Database

Place `conductors.db` in the same directory as `LineTool.exe` before launching. The application looks for it at startup and shows a green ✓ or orange ✗ in the status bar accordingly. You can also load a database from a different location at runtime via **File → Open Database…**

---

## Interface

The application has two tabs. Additional tabs can be added with minimal effort — see [Adding a Tab](#adding-a-tab) below.

---

### Tab 1 — Line Parameters

The main calculation tab. Split into a left input pane and a right output pane.

#### Left Pane — Inputs

**Phase Positions**

Enter the (x, y) coordinates in metres for each of the three phase conductors relative to the tower centre. x is the horizontal offset (negative = left), y is the height above ground. The tower cross-section diagram at the bottom of the left pane updates in real time as you type.

| Field | Description |
|-------|-------------|
| Phase A x / y | Horizontal offset and height of phase A conductor (m) |
| Phase B x / y | Horizontal offset and height of phase B conductor (m) |
| Phase C x / y | Horizontal offset and height of phase C conductor (m) |

**Earth Wire Positions**

Tick the checkbox next to OHEW 1 or OHEW 2 to enable an overhead earth wire. Its position is shown in the tower diagram as a crossed circle. Earth wire positions do not currently affect the electrical calculations (they are geometry display only) but are included for completeness and future shielding angle analysis.

**Conductor**

| Field | Description |
|-------|-------------|
| Type | Dropdown populated from `conductors.db`. Selecting a conductor auto-fills the AC resistance used in calculations. |
| GMR (m) | Geometric Mean Radius of the conductor in metres. Fill this manually or extend the DB schema to carry a GMR column and auto-populate it. |
| Bundle n | Number of sub-conductors per phase (1–4). |
| Bundle sp (m) | Sub-conductor spacing in metres. Only relevant when Bundle n > 1. |

**Circuit**

| Field | Description |
|-------|-------------|
| Voltage (kV) | Nominal line-to-line voltage. Used for SIL, charging MVAR, and loadability. |
| Length (km) | Total route length. Used for total admittance, charging MVAR, and ABCD matrix. |
| Frequency (Hz) | Power frequency. 50 Hz default. |
| I thermal (A) | Thermal current limit of the conductor. Used as the operating current in the loadability calculation. |

Press **⚡ Calculate [F5]** or hit **F5** to run the calculation.

#### Tower Cross-Section Diagram

Shown at the bottom of the left pane. Updates live as coordinates change — no need to press Calculate to see the geometry. Phases are coloured red (A), blue (B), and green (C). Earth wires appear as grey crossed circles. The dashed line and label shows the D13 spacing.

#### Right Pane — Outputs

**Geometry**

| Output | Description |
|--------|-------------|
| D₁₂ | Phase A to Phase B centre-to-centre distance (m) |
| D₂₃ | Phase B to Phase C centre-to-centre distance (m) |
| D₁₃ | Phase A to Phase C centre-to-centre distance (m) |
| GMD | Geometric Mean Distance = ∛(D₁₂ × D₂₃ × D₁₃) (m) |

**Per-Unit-Length Parameters**

Computed from tower geometry and conductor GMR using the standard transposed line formulae.

| Output | Formula | Units |
|--------|---------|-------|
| Inductance | L = 0.2 · ln(GMD / GMR_bundle) | mH/km |
| Capacitance | C = 0.0556 / ln(GMD / GMR_bundle) | µF/km |
| Reactance | X = ωL | Ω/km |
| Susceptance | B = ωC | µS/km |

For bundled conductors the effective GMR is adjusted before the logarithm: bundle of 2 → √(DS · s), bundle of 3 → ∛(DS · s²), bundle of 4 → 1.091 · (DS · s³)^¼, where s is the bundle spacing.

**Characteristic Parameters**

Derived from the full complex RLGC model (series impedance z = r + jωL, shunt admittance y = g + jωC).

| Output | Description |
|--------|-------------|
| Zc | Characteristic impedance = √(z/y), real part shown (Ω) |
| β | Phase constant (propagation constant real part) (rad/km) |
| Vel. factor | Wave propagation velocity as a fraction of the speed of light |

**Power System Results**

| Output | Description |
|--------|-------------|
| SIL | Surge Impedance Loading = V² / Zc (MVA). The natural load at which the line neither absorbs nor generates reactive power. |
| λ/4 length | Quarter-wavelength of the line at the given frequency (km). A line at this length has maximum voltage rise at no-load. |
| Shunt admittance | Total shunt admittance over the full route length (mS) |
| Charging | Total line-charging reactive power at rated voltage (MVAR) |
| Loadability | Approximate maximum transmissible active power at the thermal current limit (MW), using the St. Clair / exact-model formula |

**ABCD Matrix**

The exact distributed-parameter transmission matrix for the full line length, displayed in polar form (magnitude ∠ angle°).

```
[ Vs ]   [ A  B ] [ Vr ]
[ Is ] = [ C  D ] [ Ir ]
```

A and D are dimensionless (equal by symmetry for a passive line). B is in Ω and represents the transfer impedance. C is in mS and represents the transfer admittance.

---

### Tab 2 — Conductor Database

A read-only viewer of `conductors.db`. Select which table to display using the radio buttons at the top:

- **Conductor** — master table: type, codename, cross-sectional area, stranding, overall diameter
- **Electrical** — DC resistance at 20 °C and 75 °C, AC resistance at 50 Hz, inductive reactance at 50 Hz (all Ω/km)
- **Mechanical** — mass (kg/km), breaking load (N), elastic modulus (GPa), thermal expansion coefficient (1/K)
- **Current Ratings** — ampacity table broken down by environment (rural/industrial), season, time of day, and wind speed

The filter bar above the table searches across all columns simultaneously. The table is sortable by clicking any column header.

All four tables are joined on the `uid` primary key. The conductor selected in Tab 1 is looked up by `uid` to retrieve its AC resistance at 50 Hz for use in calculations.

---

## Calculations Reference

All calculations use the **exact distributed-parameter model**, not the nominal-π approximation. This is accurate for lines of any length.

### Inductance and Capacitance

Assumes a fully transposed three-phase line. GMD is the cube root of the product of the three phase-to-phase spacings. GMR for bundled conductors is computed as described above. The shunt conductance g is assumed zero (lossless insulation), which is standard practice for overhead lines.

### ABCD Matrix

```
A = D = cos(k·l)
B     = Zc · sin(k·l)
C     = sin(k·l) / Zc
```

where k = √(z·y) is the complex propagation constant and l is the line length. This is the exact solution to the telegrapher's equations.

### SIL

```
SIL = V² / Zc   (MVA)
```

At SIL the line reactive power is self-compensating: inductive reactive power consumed equals capacitive reactive power generated.

### Loadability

Uses the power-angle relationship with the exact ABCD parameters:

```
P = (|Vs|/Vn) · (|Vr|/Vn) · SIL · sin(δ) / sin(βl)
```

where δ is the angle between sending and receiving end voltages computed from the thermal current, and βl is the electrical length of the line.

---

## Database Schema

The application expects a SQLite file named `conductors.db` with the following tables and column names exactly as shown. The primary/foreign key joining all tables is `uid`.

```
conductor      (uid, type, codename, area_mm2, stranding, overall_diameter_mm)
electrical     (uid, dc_resistance_20C, dc_resistance_75C, ac_resistance_50Hz, reactance_50Hz)
mechanical     (uid, mass, breaking_load, modulus, expansion)
current_rating (uid, environment, season, time_of_day, wind_speed, amps)
```

If your database uses different column names, update the SQL queries in `src/core/Database.cpp`.

---

## Adding a Tab

The application is structured so that adding functionality means adding a tab — nothing else in the codebase needs to change.

1. Create `src/tabs/MyTab.h` and `src/tabs/MyTab.cpp`. Inherit `QWidget`. Accept `Database&` in the constructor if you need DB access.

2. In `src/MainWindow.h`, add:
   ```cpp
   #include "tabs/MyTab.h"
   // ...
   MyTab* m_myTab = nullptr;
   ```

3. In `src/MainWindow.cpp` inside `buildUi()`, add:
   ```cpp
   m_myTab = new MyTab(m_db);
   m_tabs->addTab(m_myTab, "🗂  My Tab");
   ```

4. In `CMakeLists.txt`, add `src/tabs/MyTab.cpp` to the `TAB_SOURCES` list.

Re-run cmake configure and build. The new tab appears automatically.

---

## Project Structure

```
LineTool/
├── CMakeLists.txt
├── README.md
├── conductors.db               ← place here or in build/ beside the exe
├── resources/
│   └── winres.rc               ← Windows icon stub (disabled by default)
├── tools/
│   └── create_sample_db.py     ← generates a test DB if you don't have one
└── src/
    ├── main.cpp
    ├── MainWindow.h/.cpp       ← window shell; register new tabs here
    ├── core/
    │   ├── TransmissionLine.h/.cpp   ← all physics, no Qt dependency
    │   └── Database.h/.cpp           ← SQLite read wrapper
    ├── models/
    │   └── ConductorTableModel.h/.cpp    ← QAbstractTableModel for DB viewer
    ├── widgets/
    │   └── TowerGeometryWidget.h/.cpp    ← QPainter tower cross-section
    └── tabs/
        ├── LineParametersTab.h/.cpp      ← Tab 1
        └── ConductorDatabaseTab.h/.cpp   ← Tab 2
```

---

## Notes and Limitations

- All calculations assume a **fully transposed** three-phase line. Untransposed lines require a full 3×3 impedance matrix approach and are not currently implemented.
- The shunt conductance **g is set to zero**. This is appropriate for overhead lines in normal service but will underestimate losses on aged or contaminated insulation.
- The **GMR field must be set manually** unless your `conductors.db` carries a GMR column. A typical starting value for ACSR is 0.7–0.75 × the conductor radius. Check manufacturer datasheets.
- Loadability is an **approximation** based on the thermal current limit and the angle derived from it. It does not account for voltage regulation limits, stability margins, or network constraints.
- Results are indicative. Always verify against manufacturer data and relevant standards before use in engineering design.
