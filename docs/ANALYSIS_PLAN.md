# Modeling & analysis plan — evaluation and next steps

_Evaluation of the current system-ID / inversion approach, and the additional analyses that build a solid foundation for tuning._

## 1. What the current scripts give you (and their limits)

| Script | Gives | Limitation to be aware of |
|---|---|---|
| `sysid_motor_speed` | Kv, deadband Vdb, tau (motor+gearbox) | measured light-loaded; the pump load changes these under pressure |
| `sysid_pressure_build` | dP/dt vs (V, P) — the real nonlinearity | needs runs at several V *and* several start pressures to fill the surface |
| `sysid_leak` | leak rate c, tau_leak; separates cooling | assumes first-order leak; orifice leak needs the sqrt(P) fit |
| `fit_plant_VtoP` | dP/dt = kv·(V−Vdb) − c·P, local `kv/(s+c)` | single global linear-ish model; gain actually rises with P |
| `projectile_sim`/`build_inversion_table` | distance → v0 → P_target, per ball | v0→P uses guessed eta & V_swept until fit to real shots |

The chain is decoupled correctly (dynamic V→P + static P→distance), and the models are deliberately simple because the PI + anti-windup is robust to model error. The weak spots are all **parameter confidence** and **coupling**, addressed below.

## 2. Model validation (do these first — cheap, high value)
1. **Hold-out cross-validation.** Fit the plant on N−1 build runs, predict the N-th, report **NRMSE / VAF** on the prediction (not the fit). Repeat leave-one-out. A model that only fits its own data is worthless for control.
2. **Simulate-vs-measured overlay.** Feed a *recorded voltage profile* into `dP/dt = kv(V−Vdb) − cP` and overlay the simulated P on the measured P for a run you did NOT fit. Eyeball + NRMSE.
3. **Energy-balance sanity check.** Compare pneumatic energy released (∫P dV or P_gauge·V_swept) to measured ball KE (from `v0 = sqrt(2·KE/m)`, back-computed from landing). The ratio *is* your launch efficiency eta — if it's absurd (>1 or ~0), the pneumatic model or geometry is wrong.

## 3. Parameter confidence & sensitivity
4. **Sensitivity of landing distance to each parameter.** Perturb Cd, eta, V_swept, P_target, h_launch, angle ±10% in `projectile_sim`/inversion and rank the effect on distance. This tells you *what to measure precisely* and *what you can guess*. (Expect P_target and eta to dominate; angle is fixed at 45°.)
5. **Confidence intervals on fits.** Report standard errors on Kv, c, kv from the regressions (or bootstrap the residuals). Wide intervals = collect more data there.
6. **Drag + efficiency from real shots (joint fit).** Once you have shot data, fit **Cd and eta together** so the inversion table matches reality, instead of trusting textbook Cd=0.47 and a guessed eta. This is the single biggest accuracy lever.

## 4. Coupling & second-order effects to quantify
7. **Motor-load ↔ pressure coupling.** Log motor speed (encoder, slow) *and* pressure during a build. Motor speed will droop as P rises (load torque up). Quantify it; if large, either gain-schedule or accept that `kv` effectively falls with P (already captured by the −cP term partly, but verify).
8. **Thermal/adiabatic effect.** From the leak-down fast transient, estimate the compression heating (how much P you "lose" to cooling if you hold). Confirms the fire-on-the-fly decision and bounds the hold-time error.
9. **Valve / firing delay.** Command fire and measure the time from signal to actual pressure drop (scope the pressure). This delay shifts your fire-on-the-fly threshold — subtract it (fire slightly early).
10. **Operating-point check for gain scheduling.** Estimate `kv/(s+c)` at low / mid / high pressure. If the gain varies >~2–3×, gain-schedule the PI on measured pressure; if less, one PI is fine.

## 5. Repeatability & competition performance
11. **Shot-to-shot variance budget.** At a few fixed P_target, fire ~10 shots each; record landing spread σ per pressure. Propagate σ through the inverse map to a **distance-error budget**. This tells you if ±0.5 cm is even reachable and where the noise comes from (pressure sensing, valve timing, ball seating).
12. **Monte Carlo of the full pipeline.** Sample the parameter uncertainties (Cd, eta, P sensing, valve delay) and run `projectile_sim` many times → predicted **landing distribution and hit-rate** vs target. Sets realistic expectations and shows which uncertainty to attack.
13. **Run-to-run convergence test.** Simulate the shot-to-shot trim (integral / fuzzy) against a drifting map and confirm it converges and doesn't oscillate.

## 6. Data collection efficiency (so you don't shoot forever)
14. **Design of experiments.** For the build surface, use a grid of ~4–5 voltages × 3 start pressures (not random). For distance calibration, 5–7 pressure levels × 5–10 shots per ball type; measure the variance, not just the mean.
15. **Live-zero / fault logging.** Log the raw pressure ADC every shot; flag `<0.88 V` (broken sensor) so bad data is auto-rejected.

## 7. Suggested order
1. Validate the plant model (cross-val + energy balance) — trust before you tune.
2. Sensitivity ranking — decide what to measure well.
3. Distance calibration + joint Cd/eta fit — make the inversion table real.
4. Variance budget + Monte Carlo — set the achievable spec.
5. Gain-schedule / valve-delay only if 1–4 say you need them.

The theme: your models are simple by design and that's fine — **spend the effort on validating parameters and quantifying variance**, because for this launcher repeatability (not model fidelity) is what wins.
