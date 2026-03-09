#!/usr/bin/env python3
"""
tools/create_sample_db.py
Creates a sample conductors.db matching the YAML schema for testing LineTool.
Run once: python3 tools/create_sample_db.py
Places conductors.db in the project root (or wherever you run from).
"""

import sqlite3
import os

DB_PATH = os.path.join(os.path.dirname(__file__), "..", "conductors.db")

DDL = """
CREATE TABLE IF NOT EXISTS conductor (
    uid                   INTEGER PRIMARY KEY,
    type                  TEXT,
    codename              TEXT,
    area_mm2              REAL,
    stranding             TEXT,
    overall_diameter_mm   REAL
);

CREATE TABLE IF NOT EXISTS mechanical (
    uid           INTEGER PRIMARY KEY,
    mass          REAL,
    breaking_load REAL,
    modulus       REAL,
    expansion     REAL,
    FOREIGN KEY (uid) REFERENCES conductor(uid)
);

CREATE TABLE IF NOT EXISTS electrical (
    uid                 INTEGER PRIMARY KEY,
    dc_resistance_20C   REAL,
    dc_resistance_75C   REAL,
    ac_resistance_50Hz  REAL,
    reactance_50Hz      REAL,
    FOREIGN KEY (uid) REFERENCES conductor(uid)
);

CREATE TABLE IF NOT EXISTS current_rating (
    uid          INTEGER,
    environment  TEXT,
    season       TEXT,
    time_of_day  TEXT,
    wind_speed   TEXT,
    amps         REAL,
    FOREIGN KEY (uid) REFERENCES conductor(uid)
);
"""

# uid, type, codename, area_mm2, stranding, diameter_mm
CONDUCTORS = [
    (1, "ACSR", "Dog",    100,  "6/1",  14.15),
    (2, "ACSR", "Lynx",   175,  "30/7", 19.53),
    (3, "ACSR", "Panther",200,  "30/7", 21.00),
    (4, "ACSR", "Zebra",  400,  "54/7", 28.62),
    (5, "AAC",  "Ant",     35,  "7",     8.43),
    (6, "AAC",  "Wasp",    70,  "19",   10.52),
    (7, "AAAC", "Hazel",  100,  "7",    13.26),
    (8, "AAAC", "Oak",    300,  "19",   22.40),
]

# uid, mass kg/km, breaking_load N, modulus GPa, expansion 1/K
MECHANICAL = [
    (1,  394,  31100, 80, 19.1e-6),
    (2,  726,  58600, 80, 18.9e-6),
    (3,  844,  66700, 80, 18.9e-6),
    (4, 1621, 125100, 80, 19.3e-6),
    (5,  107,   6100, 57, 23.0e-6),
    (6,  213,  11400, 57, 23.0e-6),
    (7,  273,  27600, 69, 23.0e-6),
    (8,  819,  76800, 69, 23.0e-6),
]

# uid, dc_20C, dc_75C, ac_50Hz, reactance_50Hz  (all Ω/km)
ELECTRICAL = [
    (1, 0.2853, 0.3474, 0.2910, 0.3936),
    (2, 0.1638, 0.1993, 0.1670, 0.3563),
    (3, 0.1438, 0.1750, 0.1467, 0.3500),
    (4, 0.0714, 0.0869, 0.0730, 0.3060),
    (5, 0.8680, 1.0560, 0.8850, 0.4140),
    (6, 0.4260, 0.5184, 0.4350, 0.3900),
    (7, 0.2870, 0.3493, 0.2930, 0.3570),
    (8, 0.0958, 0.1166, 0.0980, 0.3150),
]

# Ampacity: each conductor gets 4 sample rows
def rating_rows():
    rows = []
    for uid, _, _, area, _, _ in CONDUCTORS:
        base = 450 + area * 0.8
        rows += [
            (uid, "rural",      "summer", "noon",  "still",    base * 0.70),
            (uid, "rural",      "summer", "noon",  "wind_1ms", base * 0.85),
            (uid, "rural",      "winter", "night", "wind_2ms", base * 1.10),
            (uid, "industrial", "summer", "noon",  "still",    base * 0.65),
            (uid, "industrial", "summer", "noon",  "wind_1ms", base * 0.80),
            (uid, "industrial", "winter", "night", "wind_2ms", base * 1.05),
        ]
    return rows

def main():
    conn = sqlite3.connect(DB_PATH)
    conn.executescript(DDL)
    conn.executemany("INSERT OR REPLACE INTO conductor VALUES (?,?,?,?,?,?)", CONDUCTORS)
    conn.executemany("INSERT OR REPLACE INTO mechanical VALUES (?,?,?,?,?)", MECHANICAL)
    conn.executemany("INSERT OR REPLACE INTO electrical VALUES (?,?,?,?,?)", ELECTRICAL)
    conn.execute("DELETE FROM current_rating")
    conn.executemany("INSERT INTO current_rating VALUES (?,?,?,?,?,?)", rating_rows())
    conn.commit()
    conn.close()
    print(f"Created: {os.path.abspath(DB_PATH)}")
    print(f"  {len(CONDUCTORS)} conductors, electrical, mechanical, current_rating populated.")

if __name__ == "__main__":
    main()
