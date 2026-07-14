# Handoff — MSE 312 motor-control firmware (Arduino Mega 2560 + TB6568KQ)

_Last updated: 2026-07-13_

## Project layout
- PlatformIO project at `c:\Users\dangs\Desktop\project\potato`.
- Pure control logic: `include/MotorLogic.h` — host-unit-tested with Unity (`pio test -e native`).
- Firmware: `src/main.cpp` (env `megaatmega2560`).
- Encoder: quadrature 500-line, X4-decoded on D2/D3 via native `ISR(INT4/5_vect)`, mounted on the
  motor shaft **before** a 6.3:1 gearbox.
- Board is on **COM5**.
- `pio` is **not** on PATH — call it by full path: `C:\Users\dangs\.platformio\penv\Scripts\pio.exe`.
  The User PATH entry is misspelled `...\penv\Script` (missing the "s", should be `Scripts`).
  Fix (not yet applied):
  ```powershell
  $p = [Environment]::GetEnvironmentVariable('Path','User')
  $fixed = ($p -split ';' | ForEach-Object {
    if ($_.TrimEnd('\') -ieq 'C:\Users\dangs\.platformio\penv\Script') { 'C:\Users\dangs\.platformio\penv\Scripts' } else { $_ }
  }) -join ';'
  [Environment]::SetEnvironmentVariable('Path', $fixed, 'User')
  ```

## What changed this session
1. **NAND board removed → direct drive.** D12/D13 now drive the H-bridge inputs IN1/IN2 directly.
   `applyDrive()` PWMs the active direction pin (forward = PWM on D12, D13 LOW; reverse = PWM on D13,
   D12 LOW; brake = both HIGH; coast = both LOW). Old PWM pin D9/Timer2 retired.
   `computeDrive()` pure logic and its unit tests are unchanged.
2. **Variable-gain sweep** (`sweep on|off`, `MODE_SWEEP`) — the "virtual MATLAB gain slider."
   Ramps 2→12 V in 1 V steps, currently **10 s per step**; at the top it resets to the 2 V floor and
   flips CW↔CCW. Floor is 2 V because ≤1 V sits in the motor's stiction deadband (won't turn).
   A **soft-start slew (10 V/s)** eases the applied voltage at each step to cut inrush.
3. **Telemetry:** default **OFF** now (`telem on` to stream); `count` column removed; angle wrapped to
   [0, 2π) via new `countToWrappedTheta()` (integer modulo — exact, never overflows; continuous theta
   kept internally for velocity). Header: `# t_s,theta_wrap_rad,omega_rad_s,duty,DIR1,DIR2,mode`.
   `t_s` = seconds since boot.
4. **Reset diagnostic:** boot banner prints `# last reset: 0x.. [BROWN-OUT/WATCHDOG/EXTERNAL/POWER-ON]`
   from MCUSR. Caveat: the Mega bootloader clears MCUSR so it reads `0x0`, but the banner reappearing
   at all proves a reset.
5. Added 7 wrap-angle unit tests. **All 33 tests pass**, firmware builds clean, soft-start build is
   flashed to the board.

## Wiring (current)
| Signal   | From             | To (driver) |
|----------|------------------|-------------|
| IN1      | Arduino **D12**  | pin 1       |
| IN2      | Arduino **D13**  | pin 2       |
| +Motor   |                  | pin 3       |
| Signal/Power GND | Arduino GND + supply − | pin 4 |
| −Motor   |                  | pin 5       |
| (NC)     |                  | pin 6       |
| 12 V     | supply +         | pin 7       |

- Arduino GND **is** tied to pin 4 now (fixed this session).
- Arduino powered via USB; motor via a **12 V wall adapter**.

## Open problem (main issue): board resets under motor load
- Originally reset at **5 V** with **no shared ground** — logic signals D12/D13 had no reference.
  Fixed by connecting Arduino GND to driver pin 4.
- After grounding, the driver drives properly so current is higher, and it now resets **exactly at the
  2→3 V sweep step** (boot banner reappears = confirmed reset).
- Diagnosis: **brown-out / supply transient.** Suspect the 12 V **wall adapter's** limited current
  headroom + hiccup-mode overcurrent protection (differs from a bench supply under motor inrush).
- Soft-start was added to distinguish inrush vs. steady-current sag.

## Next steps
- Run `telem on` then `sweep on`; watch the 2→3 V step:
  - **Sweeps through** → inrush; soft-start fixed it.
  - **Reset ~100 ms *after* the step** (a couple telem rows appear first) → adapter can't hold the
    steady current → add a **470–1000 µF bulk cap across pin 7 ↔ pin 4** and/or a beefier supply.
  - **Instant reset as before** → grounding/EMI, not supply sag.
- Get the wall adapter's **current rating** (label).
- Recommended hardware: bulk cap on the 12 V rail; 0.1 µF ceramic across motor terminals (pin 3 ↔ 5);
  star-ground (signal ground meets power ground only at the supply negative).
- Optional/deferred: apply the PATH typo fix; add a watchdog if it turns out to be a *hang* not a
  reset; option to reset `t_s` at sweep start.

## Build / flash / test
```
# build
C:\Users\dangs\.platformio\penv\Scripts\pio.exe run -e megaatmega2560
# upload (close the serial monitor first — it locks COM5)
C:\Users\dangs\.platformio\penv\Scripts\pio.exe run -e megaatmega2560 -t upload --upload-port COM5
# host unit tests
C:\Users\dangs\.platformio\penv\Scripts\pio.exe test -e native
# serial monitor
C:\Users\dangs\.platformio\penv\Scripts\pio.exe device monitor -e megaatmega2560
```

## Serial commands
`v <volts>` (manual, −12..12) · `sweep on|off` · `brake` · `stop` (coast) · `target <rad>` ·
`pid on|off` · `zero` · `telem on|off` · `help`
