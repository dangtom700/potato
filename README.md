# MSE 312 — DC Motor Directional Control

Firmware and MATLAB tooling to drive and characterize a DC motor through a Toshiba
**TB6568KQ** H‑bridge fed by an **SN7400 NAND** board, with quadrature‑encoder
feedback on an Arduino **Mega 2560**. Covers system identification (step / "bump"
tests), first‑order model fitting, and PI/PD gain design + validation.

## Hardware

| Item | Detail |
|------|--------|
| MCU | Arduino Mega 2560 (COM5 by default) |
| Driver | TB6568KQ H‑bridge |
| Logic | SN7400 NAND board: `IN1 = PWM AND DIR1`, `IN2 = PWM AND (NOT DIR1)` |
| Drive pins | **D9 = PWM**, **D8 = DIR** into the NAND board |
| Encoder | Quadrature on **D2/D3**, 500 lines, ×4, 6.3:1 gearbox → 12600 counts / output rev |
| Indicator | LED on D11 (brightness ∝ shaft angle) |

All logic is 5 V.

## Repository layout

```
potato/
├── arduino_code/      PlatformIO firmware — src/main.cpp, include/MotorLogic.h, host tests
├── RASPlib/           MATLAB support library (must stay beside the scripts — see below)
├── startup.m          Auto-run at MATLAB launch: arduino() + addpath(RASPlib)
├── nand_debug.m       Characterize the SN7400 NAND board (D9/D8 → A0/A1/A2)
├── bump_tests.m       Automated bump tests → identify K/τ, recommend PI + PD gains
├── validate_tuning.m  Expected (sim) vs actual (motor) angle response, mod-π PD
├── lab3.m, lab3_2.m            Lab 3
├── lab4_sysID*.m, lab4_report.m   Lab 4 (encoder-logging sysID + report)
├── lab1/, lab2/       Simulink labs (.slx + generated code)
├── docs/              Datasheets, lecture/experiment PDFs, reports + figures
├── media/             Bench photos & video — NOT in git (see link below)
└── *.csv, *.mat       Data captured / emitted by the scripts
```

## MATLAB scripts

Run them from the **repo root** (MATLAB's working directory). Each script's header
`addpath` expects `RASPlib/` as a **sibling**, so keep the `.m` files and
`RASPlib/` together at the root.

- **`nand_debug.m`** — sweeps PWM/DIR into the NAND board and classifies each output
  (tracks PWM / inverted / stuck) to verify the gates. → `nand_debug.csv`
- **`bump_tests.m`** — steps the motor at each duty (10–40 %, CW & CCW = 8 tests),
  fits `ω/V = K/(τs+1)`, and prints recommended **velocity PI** and **angle PD**
  gains. → `bump_tests_raw.csv`, `bump_tests_summary.csv`
- **`validate_tuning.m`** — closes a **mod-π** angle PD loop and overlays the
  *expected* (simulated) vs *actual* (measured) step response. Reads
  `bump_tests_summary.csv`; → `validate_expected.mat`, `validate_actual.csv`

## Firmware ⇄ MATLAB (important)

Two families of scripts talk to the board **differently**, and switching between
them requires a re-flash:

| Scripts | How they connect | Effect on the board |
|---------|------------------|---------------------|
| `nand_debug.m`, `lab4_*`, `startup.m` | MATLAB `arduino()` object | **Re-flashes the MATLAB server firmware** |
| `bump_tests.m`, `validate_tuning.m` | raw `serialport` + firmware commands | **Needs the MSE312 firmware** |

Flash the MSE312 firmware with:

```
pio run -d arduino_code -t upload      # port auto-detected (COM5)
```

After running any `arduino()`-based script, re-flash before the serial-based ones.
If the COM port ever changes, find it with `serialportlist("available")`.

## Typical workflow

1. `pio run -d arduino_code -t upload` — put the MSE312 firmware on the board.
2. `bump_tests.m` — identify K/τ and get PI/PD gains.
3. `validate_tuning.m` — compare the expected vs actual tuned response.

(Run `nand_debug.m` any time to sanity-check the NAND board.)

## Current findings (latest run)

- The motor is driven **through the NAND board** (D9 PWM + D8 DIR), not D12/D13.
- `K ≈ 6.4 (rad/s)/V`. `τ ≲ 50 ms` — currently at the 20 Hz telemetry resolution
  floor; lower `TELEM_DECIM` in the firmware to resolve the real value.
- Duty is capped at **40 %**: higher inrush browns out the supply and resets the board.

## Photos & video

Large bench media (a ~5-minute drive video plus photos) is kept out of git —
GitHub blocks files over 100 MB — and lives in Google Drive:

**https://drive.google.com/drive/folders/1SCe-OaLb3nAHqmevgox-py3BdQRYuB4O?usp=drive_link**
