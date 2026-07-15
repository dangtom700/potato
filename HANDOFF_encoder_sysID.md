# Handoff — MSE 312 board state: encoder-only data acquisition for System ID

_Last updated: 2026-07-14_

## TL;DR
- The Arduino Mega's **digital OUTPUT drivers are dead** (destroyed by motor-reversal transients). The board still reads analog + digital **inputs** fine and runs code.
- The board is therefore repurposed as an **encoder logger only** — it no longer drives the motor.
- The **motor is spun by a separate power source + separate PWM driver** (external, not this Arduino). The Arduino just watches the encoder and records the response.
- **Request for the Code session (later):** write a new Arduino/PlatformIO script that logs encoder position/velocity (and optionally the motor drive voltage) as time-stamped CSV over serial, for offline system identification. No motor driving on this board.

## 1. Board state — what works / what doesn't
| Function | Status | Notes |
|---|---|---|
| 5 V rail | OK | powers encoder; usable as Lab-1 reference |
| Code execution / USB | OK | sketch runs, serial works |
| Analog inputs (A0–A15) | OK | usable for logging an external voltage |
| Digital **inputs** / interrupts (D2, D3) | OK | encoder reads correctly |
| Digital **outputs** (all D pins) | **DEAD** | stuck ~0.3 V on HIGH even bare; drivers destroyed |
| Motor driving from this board | **ABANDONED** | do not rely on any output pin |

Cause of death (for the record): hard CW↔CCW reversals with no bulk capacitor → back-EMF / rail-collapse / ground-bounce transients injected into the output pins' clamp diodes → output stage cooked. See `MSE312_MCU_Protection.pdf` for the fix on the replacement board.

## 2. Physical setup (current)
- **Arduino**: USB-powered, encoder duty only.
- **Encoder → Arduino** (quadrature, 500-line, X4, mounted on the motor shaft **before** the 6.3:1 gearbox):
  - Ch A (yellow) → **D2** (interrupt)
  - Ch B (blue) → **D3** (interrupt)
  - Vcc (red) → **5V**
  - GND (black) → **GND**
  - Index (green) → unused
- **Motor**: driven by a **separate PWM driver + separate power supply**. Electrically isolated from the Arduino — the only coupling is the **motor shaft** (mechanical) and the encoder attached to it. The encoder is a separate optical device powered from the Arduino, not from the motor windings.
- **Board on COM5.**

This isolation is deliberate and protective: the motor's electrical domain never touches the (already fragile) Arduino, so data collection can't kill it further.

## 3. What to collect for system identification
Goal: identify the DC-motor model — input **voltage (or PWM duty)** → output **shaft angle / angular velocity**. Typically a first-order velocity model `ω(s)/V(s) = K/(τs+1)`, or second-order if capturing position/inertia.

- **Output (from encoder):** angle and angular velocity.
  - Angle: `theta = count * 2*pi / (Q*N*G)`, with `Q=4, N=500, G=6.3` → `Q*N*G = 12600` edges per output-shaft revolution.
  - Velocity: differentiate the **continuous (unwrapped)** angle — filtered derivative `s/(cs+1)`, `c=0.01`, or `Δcount/Δt · 2π/12600`. **Wrap to [0,2π) for display only, never before the derivative.**
- **Input (from the external driver):** the commanded motor voltage / duty. Two ways to know it (pick per test):
  1. **Known step schedule (recommended, fully isolated):** command the external driver to fixed voltages on a known timeline (e.g., step to 6 V at t=0). You *know* the input, so you don't need to measure it — keeps the Arduino electrically isolated.
  2. **Measured on A0 (only if you need arbitrary inputs):** tap the motor drive voltage into **A0 through a divider** (motor up to 12 V → divide ≥3× to stay ≤5 V, add a series resistor + clamp). ⚠️ This requires tying the external driver's ground to Arduino GND at a single point — re-introducing a ground link to the motor domain. Do it only if step tests aren't enough, and keep it star-grounded and away from motor power leads.
- **Test signals:** a set of voltage **steps** at several amplitudes (both directions), and/or a slow **staircase/sweep**. Log at **100 Hz** through the full transient. Keep within the motor's ±12 V rating.

## 4. Script to build (spec for the Code session)
Target: `megaatmega2560`, PlatformIO. **No motor driving** (outputs are dead — don't even try).

- **Encoder:** X4 quadrature decode on **D2/D3** via hardware interrupts; signed `volatile long count`.
- **Sampling:** fixed **dt = 0.01 s (100 Hz)** using `micros()`.
- **Compute per sample:** continuous `theta_rad = count*2π/12600`; `omega_rad_s` = filtered derivative (or Δcount/dt). Keep continuous theta for velocity; also emit wrapped `theta_wrap` for convenience.
- **Optional analog input:** `A0` = scaled motor voltage (if using method 2 above); log raw ADC + scaled volts (apply the divider ratio in code).
- **Serial @ 115200, CSV** with a header line, e.g.:
  `# t_s,count,theta_rad,omega_rad_s[,vin_V]`
- **Commands:** `start`, `stop`, `zero` (reset count/time), `help`.
- **Companion (host side):** capture serial to a `.csv`, then fit the model in MATLAB (System Identification Toolbox / `tfest`) or Python — first-order for velocity, optionally second-order for position.

## 5. Wiring / safety constraints
- **Never connect the external driver's outputs to any Arduino pin.** The Arduino touches only the encoder (5 V logic).
- If tapping Vin on **A0**: divider must guarantee **≤5 V** at the pin (≥3× for a 12 V rail), add a series ~1 kΩ + optional 5.1 V zener/clamp; and star-ground the shared reference.
- Twist encoder **A/B with GND**, keep the encoder cable away from motor power leads to minimize EMI on the counts.
- Remember the Arduino outputs are dead — the script must not depend on driving anything.

## 6. Environment / commands (carried from prior handoff)
- PlatformIO project at `C:\Users\dangs\Desktop\project\potato`.
- `pio` not on PATH — call full path: `C:\Users\dangs\.platformio\penv\Scripts\pio.exe`.
- Board on **COM5** (close the serial monitor before upload — it locks the port).
```
# build
C:\Users\dangs\.platformio\penv\Scripts\pio.exe run -e megaatmega2560
# upload
C:\Users\dangs\.platformio\penv\Scripts\pio.exe run -e megaatmega2560 -t upload --upload-port COM5
# serial monitor
C:\Users\dangs\.platformio\penv\Scripts\pio.exe device monitor -e megaatmega2560 -b 115200
```

## 7. Next steps
- **(You)** confirm: the external PWM driver/supply used, and whether you'll drive by **known step schedule** (no A0 tap) or need **A0 voltage logging**.
- **(Code)** write the encoder-logger sketch per §4, plus a small host capture + `tfest` script.
- Run step tests → capture CSV → fit `K/(τs+1)` (and check against the theoretical motor params).
- Replace the dead Mega and rebuild the drive with the protections in `MSE312_MCU_Protection.pdf` before returning to closed-loop control.
