# Simulink setup — launcher control system

_How to build and deploy the control model on the Arduino Mega. Pair with the MATLAB scripts in `analysis/` for calibration._

## 0. Prerequisites (once)
- **MATLAB/Simulink R2023a** (matches the course; RASPlib is fragile on newer versions).
- Add-Ons → install **Simulink Support Package for Arduino Hardware** (brings the Hardware tab + Monitor & Tune). **Simulink Coder** must be present (`ver` shows it).
- Install **Stateflow** (used for the supervisor). If unavailable, see the block-only fallback at the bottom.
- Run `startup.m` as admin to load **RASPlib** (only if you use its encoder/PWM blocks; for the pressure loop you mostly don't).
- Restart MATLAB after installing so the toolstrip registers the package.

## 1. Model architecture
One model, three parts:

1. **Hardware I/O blocks** — the pins.
2. **Stateflow supervisor** — the sequence (IDLE → DETECT → COMPRESS → FIRE → VENT).
3. **Pressure-PI subsystem** — enabled only during COMPRESS.

### I/O block map
| Block (Simulink Support Package for Arduino) | Pin | Signal |
|---|---|---|
| Digital Input | IR1 pin | ball present / wooden |
| Digital Input | IR2 pin | ball present / bouncy |
| Analog Input | A2 | pressure (raw ADC) |
| Dashboard Lamp ×2 (virtual, no wiring) | — | wooden / bouncy indicator |
| Arduino PWM | D8 | motor speed |
| Digital Output | D7 | motor direction |
| Digital Output | D12 | solenoid fire |
| Encoder (RASPlib, optional) | D2/D3 | homing only, NOT in pressure loop |

### Pressure conversion (put right after the Analog Input)
`raw → V = raw*5/1023 → I_mA = V/220*1000 → PSI = (I_mA-4)/16*100`
Use a **MATLAB Function** or Gain/Bias blocks. (220 Ω sense resistor.)

## 2. Stateflow supervisor
States and transitions (times in seconds):

- **IDLE** — motor off, solenoid off, LEDs off.
  `IR1 && debounced → DETECT (type=WOOD)`, `IR2 && debounced → DETECT (type=BOUNCY)`.
- **DETECT** (entry) — latch `type`; light that LED; set `P_target = Pmap(type)`. → `COMPRESS`.
- **COMPRESS** (during) — `motor_enable=1` (enables the PI subsystem). `[P >= P_target] → FIRE`. Guards: `[P > P_max] → VENT` (abort), `after(T_timeout, sec) → VENT`.
- **FIRE** (entry) — `fire=1`. `after(0.2, sec) → VENT` (one-shot dwell).
- **VENT** — `fire=0; motor_enable=0; LED off`. `[~IR1 && ~IR2] → IDLE` (re-arm only when the ball is gone).

Debounce = require the IR input stable for ~50 ms (a small counter in the chart, or a Debounce block).

## 3. Pressure-PI subsystem (Enabled Subsystem, enable = motor_enable)
- `error = P_target − P_meas`
- **Discrete PID(z)** as PI (P, I; no D on the noisy signal). In the block:
  - Output Saturation → **Limit output** to your working range (e.g. ±6 or 0–12), **Anti-windup = clamping**.
- **Rate-limit the reference** (or rate-limit the error path) so big jumps ramp — kills the transition overshoot.
- Optional **Dead Zone** on the error if you see low-pressure hunting.
- Output → map to **0–255 duty** → Arduino PWM (D8). Direction (D7) fixed forward during COMPRESS; reverse only if you add a retract/home step.

## 4. Loading calibration parameters
Keep tuning numbers out of the diagram — load them from a script into the base workspace and reference them by name in the blocks (e.g. `P_target`, `kv`, `c`, `Pmap`).

1. Run `analysis/sysid_*` and `fit_plant_VtoP.m` → they leave `motorModel`, `plantModel`, `leakModel` in the workspace.
2. Run `analysis/build_inversion_table.m` → `InversionTables.wooden/bouncy` (distance→pressure).
3. Set `P_target` from the inversion table for the demo distance and the detected ball type (a 1-D Lookup Table indexed by type, or two constants).
4. Reference `plantModel.kv`, `.c` when you pick PI gains (start from the first-order `kv/(s+c)`).

Put these in a `params.m` you run before the model (Model Properties → Callbacks → `InitFcn: params`).

## 5. Configure & deploy
- gear → **Model Configuration Parameters** → **Hardware Implementation** = Arduino Mega 2560; Host-board connection COM set **manually**.
- **Solver**: Start 0, Stop inf, **Fixed-step**, ode/discrete, **fixed step 0.01 s** (match every block).
- Mode **External / Run on board** → **Monitor & Tune**.
- **Telemetry:** stream few signals, decimate to ~20–50 Hz, and bump the External-mode **baud to 250000/500000** to avoid the throughput crash. The pressure loop is on analog A2, so no encoder-interrupt flood.

## 6. Bring-up order (mirror the hardware stages)
1. Logic only (5 V): confirm IR digital reads and LED outputs; watch the state chart step.
2. Add 12 V: COMPRESS drives the motor; watch `P` rise on the scope; confirm it stops/fires at `P_target`.
3. Enable the solenoid: confirm the one-shot pulse and clean re-arm.

## Fallback without Stateflow
Build the sequence with **SR-latches + Compare-to-Constant + Switch**: latch ball type on IR edges → `motor_enable = wood|bouncy`; `P_target` via Switch on type; `fire = (P>=P_target)&motor_enable` through a **monostable** (one-shot) for the dwell; reset the latches when both IR clear. Works, but the one-shot + re-arm logic is fiddlier than Stateflow.
