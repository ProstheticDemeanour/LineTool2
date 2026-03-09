# LineTool — Build Instructions

## Prerequisites

| Platform | Requirement |
|----------|-------------|
| macOS    | Qt 6 (or Qt 5.15+) via Homebrew or qt.io installer, CMake ≥ 3.21, Xcode CLT |
| Windows  | Qt 6 (or Qt 5.15+) via qt.io installer, CMake ≥ 3.21, Visual Studio 2022 or MinGW |

---

## 1. Create the sample database (first time only)

```bash
python3 tools/create_sample_db.py
# Outputs: conductors.db in the project root
```

Copy `conductors.db` into the same directory as your build output (see step 4).

---

## 2. Configure

### macOS

```bash
cmake -B build -S . \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH=$(brew --prefix qt)
```

### Windows (MSVC)

```powershell
cmake -B build -S . `
      -DCMAKE_BUILD_TYPE=Release `
      -DCMAKE_PREFIX_PATH="C:/Qt/6.7.0/msvc2022_64"
```

### Windows (MinGW)

```bash
cmake -B build -S . \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_PREFIX_PATH="C:/Qt/6.7.0/mingw_64" \
      -G "MinGW Makefiles"
```

---

## 3. Build

```bash
cmake --build build --config Release
```

---

## 4. Run

### macOS

```bash
# Copy DB next to app bundle
cp conductors.db build/
open build/LineTool.app
# OR
./build/LineTool.app/Contents/MacOS/LineTool
```

### Windows

```powershell
copy conductors.db build\Release\
.\build\Release\LineTool.exe
```

> **Note:** `windeployqt` / `macdeployqt` runs automatically post-build if found on PATH.

---

## Directory Structure

```
LineTool/
├── CMakeLists.txt              ← top-level build file
├── BUILD.md                    ← this file
├── conductors.db               ← SQLite database (generate with tools/)
├── tools/
│   └── create_sample_db.py     ← one-time DB population script
├── resources/
│   ├── AppIcon.icns            ← macOS icon (optional)
│   └── winres.rc               ← Windows resource (optional)
└── src/
    ├── main.cpp
    ├── MainWindow.h / .cpp     ← shell; add tabs here
    ├── core/
    │   ├── TransmissionLine.h  ← physics engine (port of transmission.py)
    │   ├── TransmissionLine.cpp
    │   ├── Database.h          ← SQLite wrapper (conductors.db schema)
    │   └── Database.cpp
    ├── models/
    │   ├── ConductorTableModel.h   ← QAbstractTableModel for DB viewer
    │   └── ConductorTableModel.cpp
    ├── widgets/
    │   ├── TowerGeometryWidget.h   ← real-time tower cross-section painter
    │   └── TowerGeometryWidget.cpp
    └── tabs/
        ├── LineParametersTab.h     ← Tab 1: geometry inputs + outputs
        ├── LineParametersTab.cpp
        ├── ConductorDatabaseTab.h  ← Tab 2: DB viewer (QTableView)
        └── ConductorDatabaseTab.cpp
```

---

## Adding a New Tab

1. Create `src/tabs/MyTab.h` + `MyTab.cpp` — inherit `QWidget`, accept `Database&`.
2. In `src/MainWindow.h`: add `#include "tabs/MyTab.h"` and a `MyTab* m_myTab;` member.
3. In `src/MainWindow.cpp` `buildUi()`:
   ```cpp
   m_myTab = new MyTab(m_db);
   m_tabs->addTab(m_myTab, "🗂  My Tab");
   ```
4. In `CMakeLists.txt`, add `src/tabs/MyTab.cpp` to `TAB_SOURCES`.

That's the entire integration surface — no other files need to change.
