# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

**attcalc** is a Qt/C++ GUI application that calculates coaxial cable attenuation at given frequencies and lengths. It fits frequency-attenuation datasheet values to a three-coefficient model (`attenuation = k1 + k2*f + k3*sqrt(f)`) using Levenberg-Marquardt curve fitting, then applies the fitted curve at any user-specified frequency.

Homepage: https://schueler.ws/projekte/kabeldampfungsrechner/?lang=en

## Build

```bash
qmake daempfung.pro
make          # Linux/macOS
nmake         # Windows (MSVC)
mingw32-make  # Windows (MinGW)
```

Output binary: `coaxatt`

No automated test suite — functionality is validated manually through the GUI.

## Architecture

The app is structured around three main classes:

- **`Widget`** (`widget.h/cpp`, `widget.ui`) — main window; owns the cable list, filter controls, and frequency/length inputs; triggers attenuation calculations on value changes.
- **`CableDialog`** (`cabledialog.h/cpp`, `cabledialog.ui`) — modal dialog for creating/editing cables; hosts the editable frequency-attenuation table (`VTable`) and calls `Calc()` to run the curve fitting.
- **`DirMngr`** (`dirmngr.h/cpp`) — resolves paths for program data, user config, and cable library directories.

### Cable data

Cables are stored as `.cbl` files (Qt INI format via `QSettings`). Each file holds metadata (name, impedance, diameter, manufacturer, dealer, datasheet URL) plus up to 15 frequency/attenuation pairs and the fitted coefficients k1, k2, k3.

### Curve fitting

`USE_FIT 1` (set in the source) selects the Levenberg-Marquardt algorithm (`lmcurve.c` / `lmmin.c`) as the primary fitter. `Simplex.h/cpp` (Nelder-Mead) is available as a fallback. The model function evaluated during fitting is `k1 + k2*f + k3*sqrt(f)`.

### Calculation

`result_dB = length_m * (k1 + k2*freq + k3*sqrt(freq))`

Frequency is stored and calculated in MHz internally. The UI warns when the selected frequency is outside the datasheet range of the chosen cable.

## Key constants

- `CBL_ROWS 15` — maximum number of frequency/attenuation datasheet rows per cable.
- Band presets cover 160 m through 3 cm (amateur radio bands).
- Impedance and diameter filters are applied to the cable list at display time.
