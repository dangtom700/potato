// ============================================================================
//  MotorLogic.h  -- pure, hardware-free control logic
// ----------------------------------------------------------------------------
//  This header contains ONLY math/logic with no Arduino dependencies so it can
//  be compiled and unit-tested on the host (PlatformIO `native` env / plain
//  g++). The firmware (src/main.cpp) includes it and wires it to real pins.
//
//  It replicates the MSE 312 "slider gain + brake-at-zero" Simulink model that
//  drives a Toshiba TB6568KQ H-bridge. This is pure logic: it maps a command
//  voltage to (duty, DIR1, DIR2). How those reach the pins is the firmware's
//  job -- the SN7400 NAND board has been removed and src/main.cpp now PWMs the
//  active direction line on D12/D13 directly (see applyDrive there). The
//  mapping below is unchanged by that rewiring.
// ============================================================================
#pragma once

#include <stdint.h>
#include <math.h>   // lroundf, fabsf

// ---------------------------------------------------------------------------
// Model / physical constants
// ---------------------------------------------------------------------------
static const float V_MAX   = 12.0f;   // motor rating; command is clamped to +/-12 V
static const float V_BRAKE = 0.05f;   // deadband: |v| <= this  ->  short brake
static const int   PWM_MAX = 255;     // 8-bit analogWrite() range

// Encoder + gearbox (encoder is on the MOTOR shaft, before the 6.3:1 gearbox)
static const long  ENC_N  = 500;      // encoder lines (counts/rev, single channel)
static const int   ENC_Q  = 4;        // X4 quadrature decoding
static const float GEAR_G = 6.3f;     // gearbox ratio (motor : output)

// Counts of the encoder per revolution of the OUTPUT shaft:
//     Q * N * G = 4 * 500 * 6.3 = 12600
static const float COUNTS_PER_OUTPUT_REV   = ENC_Q * ENC_N * GEAR_G;
// Same value as an exact integer, for integer-modulo angle wrapping (below).
static const long  COUNTS_PER_OUTPUT_REV_L = 12600L;

static const float TWO_PI_F = 6.28318530717958647692f;

// ---------------------------------------------------------------------------
// Drive mapping:  commanded volts v  ->  (PWM duty, DIR1, DIR2)
// ---------------------------------------------------------------------------
struct DriveCmd {
  int  duty;   // 0..255  -> D9 (PWM)
  bool dir1;   //          -> D12 (DIR1)
  bool dir2;   //          -> D13 (DIR2)
};

// Clamp the command to the motor's +/-12 V rating.
inline float clampVolts(float v) {
  if (v >  V_MAX) return  V_MAX;
  if (v < -V_MAX) return -V_MAX;
  return v;
}

// Map a signed command voltage to the three MCU outputs, exactly like the
// Simulink model:
//
//   v >  +0.05 : forward -> DIR1=H, DIR2=L, duty = round(|v|/12 * 255)
//   v <  -0.05 : reverse -> DIR1=L, DIR2=H, duty = round(|v|/12 * 255)
//   |v| <= 0.05: BRAKE   -> DIR1=H, DIR2=H, duty = 255
//
// Brake case: both DIR bits HIGH selects short brake (H-bridge IN1=IN2=H), which
// holds position; coast is intentionally never produced by this map. duty is set
// to 255 here for backward compatibility with the old NAND wiring, but the
// direct-drive firmware ignores it and just holds both inputs HIGH for brake.
inline DriveCmd computeDrive(float v) {
  v = clampVolts(v);
  DriveCmd c;

  if (v > V_BRAKE) {                 // ---- forward ----
    c.dir1 = true;
    c.dir2 = false;
    c.duty = (int)lroundf((fabsf(v) / V_MAX) * (float)PWM_MAX);
  } else if (v < -V_BRAKE) {         // ---- reverse ----
    c.dir1 = false;
    c.dir2 = true;
    c.duty = (int)lroundf((fabsf(v) / V_MAX) * (float)PWM_MAX);
  } else {                           // ---- brake (|v| <= 0.05) ----
    c.dir1 = true;
    c.dir2 = true;
    c.duty = PWM_MAX;                // full PWM => H,H => short brake, holds
  }
  return c;
}

// ---------------------------------------------------------------------------
// Encoder count -> output-shaft angle (radians), CONTINUOUS (unwrapped).
//     theta = count * 2*pi / (Q * N * G)    with Q*N*G = 12600
// Grows without bound as revolutions accumulate; used for velocity (which needs
// a continuous signal). For a bounded readout use countToWrappedTheta().
// ---------------------------------------------------------------------------
inline float countToTheta(long count) {
  return (float)count * TWO_PI_F / COUNTS_PER_OUTPUT_REV;
}

// ---------------------------------------------------------------------------
// Encoder count -> output-shaft angle WRAPPED to [0, 2*pi) (radians).
// Takes the integer modulo of the count FIRST, so the result is exact and
// bounded no matter how large `count` gets -- the displayed angle resets every
// revolution and never overflows or loses float precision.
// ---------------------------------------------------------------------------
inline float countToWrappedTheta(long count) {
  long m = count % COUNTS_PER_OUTPUT_REV_L;   // [-(rev-1) .. +(rev-1)]
  if (m < 0) m += COUNTS_PER_OUTPUT_REV_L;    // fold to [0 .. rev-1]
  return (float)m * TWO_PI_F / (float)COUNTS_PER_OUTPUT_REV_L;
}

// ---------------------------------------------------------------------------
// Wrapped shaft angle -> 8-bit LED brightness (PWM duty).
// Linearly rescales an angle in [0, 2*pi) to [0, 255], i.e. 0..360 deg -> 0..255,
// so one full output-shaft revolution sweeps the LED from off to full bright --
// the encoder shaft used as a "dial" in place of a potentiometer.
//     pwm = round(theta / (2*pi) * 255)
// Expects theta from countToWrappedTheta(); the result is clamped so a value at
// the 2*pi seam (or any float slop past it) can never produce 256 or a negative
// duty.
// ---------------------------------------------------------------------------
inline int angleToPwm(float thetaWrapped) {
  int pwm = (int)lroundf(thetaWrapped / TWO_PI_F * (float)PWM_MAX);
  if (pwm < 0)       pwm = 0;
  if (pwm > PWM_MAX) pwm = PWM_MAX;
  return pwm;
}

// ---------------------------------------------------------------------------
// X4 quadrature decode (pure). state = (A<<1)|B.
//   Returns +1 / -1 for a valid single-step transition, 0 for no-change or an
//   illegal (missed-edge) transition. Index = (prevState<<2)|newState.
// ---------------------------------------------------------------------------
inline int quadDelta(uint8_t prevState, uint8_t newState) {
  static const int8_t table[16] = {
     0, +1, -1,  0,
    -1,  0,  0, +1,
    +1,  0,  0, -1,
     0, -1, +1,  0
  };
  return table[((prevState << 2) | newState) & 0x0F];
}

// ---------------------------------------------------------------------------
// Filtered derivative for angular velocity.
//
// Continuous transfer function (a "dirty derivative"):  H(s) = s / (c*s + 1)
// i.e. differentiate theta, then low-pass with time constant c.
//
// Backward-Euler discretization (s -> (1 - z^-1)/dt) gives the recursion:
//     omega[k] = ( c*omega[k-1] + (theta[k] - theta[k-1]) ) / (c + dt)
// With c = 0.01 and dt = 0.01 this is  omega = 0.5*omega_prev + 50*dtheta.
// ---------------------------------------------------------------------------
struct VelocityFilter {
  float c;            // filter time constant (s)
  float prevTheta;
  float omega;
  bool  primed;

  VelocityFilter() : c(0.01f), prevTheta(0.0f), omega(0.0f), primed(false) {}

  void reset() { prevTheta = 0.0f; omega = 0.0f; primed = false; }

  float update(float theta, float dt) {
    if (!primed) {              // first sample: seed history, no velocity yet
      prevTheta = theta;
      omega     = 0.0f;
      primed    = true;
      return 0.0f;
    }
    float dtheta = theta - prevTheta;
    omega = (c * omega + dtheta) / (c + dt);
    prevTheta = theta;
    return omega;
  }
};

// ---------------------------------------------------------------------------
// Position PID (skeleton) -- output is a command voltage in [-V_MAX, +V_MAX].
// Disabled by default in the firmware; enable with `pid on`.
// Uses a fixed dt = 0.01 s and a simple output clamp with integrator
// anti-windup (integrator is held whenever the output saturates).
// ---------------------------------------------------------------------------
struct PID {
  float kp, ki, kd;
  float dt;
  float outMin, outMax;
  float integ;
  float prevErr;
  bool  primed;

  PID() : kp(0.0f), ki(0.0f), kd(0.0f), dt(0.01f),
          outMin(-V_MAX), outMax(V_MAX),
          integ(0.0f), prevErr(0.0f), primed(false) {}

  void reset() { integ = 0.0f; prevErr = 0.0f; primed = false; }

  float update(float setpoint, float measured) {
    float err   = setpoint - measured;
    float deriv = primed ? (err - prevErr) / dt : 0.0f;

    float integCand = integ + err * dt;
    float u = kp * err + ki * integCand + kd * deriv;

    // Clamp output; only commit the integrator if we are NOT saturating in the
    // same direction (basic clamping anti-windup).
    if (u > outMax) {
      u = outMax;
      if (err <= 0.0f) integ = integCand;   // allow integrator to unwind
    } else if (u < outMin) {
      u = outMin;
      if (err >= 0.0f) integ = integCand;
    } else {
      integ = integCand;
    }

    prevErr = err;
    primed  = true;
    return u;
  }
};
