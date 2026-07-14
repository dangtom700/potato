// ============================================================================
//  MSE 312 -- Directional control firmware for Arduino Mega 2560
// ----------------------------------------------------------------------------
//  Drives a Toshiba TB6568KQ H-bridge. The SN7400 NAND board (which used to
//  compute IN1 = PWM AND DIR1, IN2 = PWM AND DIR2 from a separate PWM pin) has
//  been REMOVED: D12 and D13 now drive the two H-bridge inputs (IN1, IN2)
//  DIRECTLY. Speed control is done by PWM'ing whichever direction pin is active
//  (the other is held LOW); D9 is no longer used.
//    forward : D12 = PWM(duty), D13 = LOW    (CW)
//    reverse : D12 = LOW,       D13 = PWM(duty) (CCW)
//    brake   : D12 = HIGH,      D13 = HIGH   (short brake, holds position)
//    coast   : D12 = LOW,       D13 = LOW    (free spin)
//
//  The v->(duty,DIR1,DIR2) mapping is unchanged (Simulink "slider gain +
//  brake-at-zero"); only how those bits reach the pins changed. See applyDrive.
//
//  Variable-gain sweep (the automatic "virtual MATLAB slider", `sweep on`):
//  hold each integer volt for 10 s from a 2 V floor, step +1 V each interval,
//  and at the top of 12 V reset to 2 V and reverse direction (CW<->CCW). Floor is
//  2 V because ~1 V and below sit in the motor's stiction deadband (no motion).
//
//  Encoder: quadrature, 500 lines, X4 decode on D2/D3 (hardware interrupts),
//  mounted on the motor shaft (before the 6.3:1 gearbox).
//
//  All pure math/logic lives in include/MotorLogic.h (host unit tested).
// ============================================================================
#include <Arduino.h>
#include "MotorLogic.h"

// ---------------------------------------------------------------------------
// Pin map (fixed by the already-soldered board -- do not change)
// ---------------------------------------------------------------------------
// D9 (old PWM pin) is now unused -- the NAND board is gone and D12/D13 drive
// the H-bridge inputs directly, PWM'ing the active direction line themselves.
// D12 = OC1B (Timer1), D13 = OC0A (Timer0); analogWrite() on each works without
// disturbing millis()/micros().
static const uint8_t PIN_DIR1  = 12;   // D12 -> IN1 (forward/CW line, PWM here)
static const uint8_t PIN_DIR2  = 13;   // D13 -> IN2 (reverse/CCW line, PWM here)
static const uint8_t PIN_ENC_A = 2;    // D2  -> encoder A (INT4)  yellow
static const uint8_t PIN_ENC_B = 3;    // D3  -> encoder B (INT5)  blue
// D11 = OC1A, the SAME timer (Timer1) as the motor's D12 = OC1B. analogWrite on
// each is an independent duty on a shared ~490 Hz base, so dimming the LED does
// not disturb the motor PWM (and neither touches Timer0 / millis()). LED anode
// -> D11 through a series resistor, cathode -> GND.
static const uint8_t PIN_LED   = 11;   // D11 -> indicator LED, dimmed by shaft angle

// ---------------------------------------------------------------------------
// Loop / telemetry cadence
// ---------------------------------------------------------------------------
static const unsigned long DT_US    = 10000UL;  // 100 Hz control step (10 ms)
static const float         DT_S     = 0.01f;    // fixed-step dt (matches Simulink)
static const uint8_t       TELEM_DECIM = 5;     // stream telemetry every Nth step -> 20 Hz

// ---------------------------------------------------------------------------
// PID position-control gains (skeleton -- disabled by default).
// Tune on the bench; command a target angle in radians with `target <rad>`.
// ---------------------------------------------------------------------------
static const float PID_KP = 3.0f;
static const float PID_KI = 1.0f;
static const float PID_KD = 0.0f;

// ===========================================================================
//  Encoder -- X4 quadrature decode via interrupts on D2 and D3
// ===========================================================================
volatile long    g_encoderCount = 0;
volatile uint8_t g_encState     = 0;   // last (A<<1)|B

// D2 = PE4, D3 = PE5 on the Mega 2560 (Port E). Reading the port directly keeps
// the ISR short (both channels sampled in one instruction) so we don't miss
// edges at speed.  state = (A<<1)|B.
static inline uint8_t readEncoderState() {
  uint8_t p = PINE;
  uint8_t a = (p >> 4) & 0x01;   // PE4 = D2 = channel A
  uint8_t b = (p >> 5) & 0x01;   // PE5 = D3 = channel B
  return (uint8_t)((a << 1) | b);
}

// Decode one edge and accumulate +/-1. Kept `inline` so it folds directly into
// each ISR vector below (no function-call context save) -- essential at full
// speed, where the encoder fires ~150 k edges/s (4536 rpm motor x 2000/rev).
// quadDelta() is the pure, unit-tested lookup.
static inline void encoderUpdate() {
  uint8_t s = readEncoderState();
  g_encoderCount += quadDelta(g_encState, s);
  g_encState = s;
}

// Native external-interrupt vectors for INT4 (D2) and INT5 (D3). These are far
// leaner than attachInterrupt(), which dispatches through a function-pointer
// table and saves the full register context on every edge -- too slow to keep
// up at full motor speed (serial would starve). Configured for "any edge"
// (CHANGE) in setup() via EICRB/EIMSK.
ISR(INT4_vect) { encoderUpdate(); }
ISR(INT5_vect) { encoderUpdate(); }

// Snapshot the volatile count atomically.
static long readCount() {
  noInterrupts();
  long c = g_encoderCount;
  interrupts();
  return c;
}

// ===========================================================================
//  Applying a command to the pins  (direct drive -- NAND board removed)
// ===========================================================================
// Last-applied outputs (for telemetry).
int  g_lastDuty = 0;
bool g_lastDir1 = false;
bool g_lastDir2 = false;

// Map a DriveCmd onto the two H-bridge inputs now wired directly to D12/D13.
// One direction pin carries the PWM (speed); the other is held LOW. Brake pulls
// both HIGH, coast both LOW -- matching the TB6568KQ IN1/IN2 truth table:
//   forward (dir1)      : D12 = PWM(duty), D13 = LOW           -> CW
//   reverse (dir2)      : D12 = LOW,       D13 = PWM(duty)     -> CCW
//   brake  (dir1&&dir2) : D12 = HIGH,      D13 = HIGH          -> short brake
//   coast  (neither)    : D12 = LOW,       D13 = LOW           -> free spin
// (computeDrive() still emits duty=255 for the brake case; it is unused here
// since brake is produced by holding both inputs HIGH, not by the PWM level.)
// analogWrite() special-cases 0 and 255 to clean digitalWrite LOW/HIGH, so full
// speed and off give glitch-free static levels.
static void applyDrive(const DriveCmd& c) {
  if (c.dir1 && c.dir2) {              // ---- short brake (holds position) ----
    digitalWrite(PIN_DIR1, HIGH);
    digitalWrite(PIN_DIR2, HIGH);
  } else if (c.dir1) {                 // ---- forward / CW ----
    analogWrite(PIN_DIR1, c.duty);
    digitalWrite(PIN_DIR2, LOW);
  } else if (c.dir2) {                 // ---- reverse / CCW ----
    digitalWrite(PIN_DIR1, LOW);
    analogWrite(PIN_DIR2, c.duty);
  } else {                             // ---- coast / free spin ----
    digitalWrite(PIN_DIR1, LOW);
    digitalWrite(PIN_DIR2, LOW);
  }
  g_lastDuty = c.duty;
  g_lastDir1 = c.dir1;
  g_lastDir2 = c.dir2;
}

// Coast / Hi-Z: both inputs LOW -> free spin.
// NOTE: coast is NOT part of the Simulink model (which never coasts); it is a
// bench convenience exposed via the `stop` command and matches the TB6568KQ
// datasheet "stop/coast" row.
static void applyCoast() {
  digitalWrite(PIN_DIR1, LOW);
  digitalWrite(PIN_DIR2, LOW);
  g_lastDuty = 0;
  g_lastDir1 = false;
  g_lastDir2 = false;
}

// ===========================================================================
//  Control state
// ===========================================================================
enum Mode { MODE_MANUAL, MODE_COAST, MODE_PID, MODE_SWEEP };
Mode  g_mode        = MODE_MANUAL;   // default: manual "v" command
float g_commandV    = 0.0f;          // manual command volts (0 => brake at boot)
float g_targetTheta = 0.0f;          // PID target (rad, output shaft)
bool  g_telemetry   = false;         // OFF by default (keeps the monitor clean
                                     // for typing commands); enable with `telem on`

VelocityFilter g_velFilter;          // s/(c*s+1), c = 0.01
PID            g_pid;

// ---------------------------------------------------------------------------
// Variable-gain sweep -- the automatic stand-in for dragging the Simulink /
// MATLAB gain slider by hand. Ramps the command 2 -> 12 V in 1 V steps, holding
// each level for 10 s; when it reaches the top of 12 V it resets to the
// 2 V floor and flips the spin direction (CW <-> CCW). Enable with `sweep on`.
// The floor is 2 V (not 0) because the motor's stiction deadband means ~1 V and
// below don't actually turn the shaft -- so every step here produces real motion.
// ---------------------------------------------------------------------------
static const unsigned long SWEEP_STEP_MS = 10000UL; // hold each level 10 s
static const int           SWEEP_V_BOT   = 2;       // ramp floor (V); above deadband
static const int           SWEEP_V_TOP   = 12;      // ramp ceiling (V), then reset

// Soft-start: instead of jumping the applied voltage at each step (which slams a
// current inrush into the supply/ground and can brown-out the board), slew it
// toward the target at this rate. 10 V/s => a 1 V step eases in over ~100 ms, and
// a direction reversal (+12 -> -2) coasts smoothly through 0. Raise for a snappier
// ramp, lower to be gentler on the supply.
static const float         SWEEP_SLEW_VPS = 10.0f;  // volts/second

int           g_sweepVolts    = SWEEP_V_BOT; // target level (BOT..TOP), volts
bool          g_sweepCW       = true;  // true = forward/CW (D12), false = CCW (D13)
unsigned long g_sweepNextMs   = 0;     // millis() at which we next step +1 V
float         g_sweepAppliedV = 0.0f;  // slew-limited voltage actually applied

// (Re)start the sweep at the 2 V floor, CW, with the first +1 V step 10 s out.
// Applied voltage starts at 0 so the very first move eases up from rest.
static void sweepStart() {
  g_sweepVolts    = SWEEP_V_BOT;
  g_sweepCW       = true;
  g_sweepNextMs   = millis() + SWEEP_STEP_MS;
  g_sweepAppliedV = 0.0f;
  g_mode          = MODE_SWEEP;
}

// Move the applied voltage one control step toward `target`, capped by the slew
// rate. Returns the new applied voltage to hand to computeDrive().
static float sweepSlew(float target) {
  float maxStep = SWEEP_SLEW_VPS * DT_S;          // max change per 10 ms step
  if (g_sweepAppliedV < target) {
    g_sweepAppliedV += maxStep;
    if (g_sweepAppliedV > target) g_sweepAppliedV = target;
  } else if (g_sweepAppliedV > target) {
    g_sweepAppliedV -= maxStep;
    if (g_sweepAppliedV < target) g_sweepAppliedV = target;
  }
  return g_sweepAppliedV;
}

// Advance the ramp once every 10 s. At the top of 12 V, reset to the 2 V floor
// and reverse direction -- this reset is what emulates the operator sliding the
// virtual gain back down and swapping CW/CCW.
static void sweepUpdate() {
  if ((long)(millis() - g_sweepNextMs) >= 0) {
    g_sweepNextMs += SWEEP_STEP_MS;
    if (++g_sweepVolts > SWEEP_V_TOP) {
      g_sweepVolts = SWEEP_V_BOT;
      g_sweepCW    = !g_sweepCW;      // reset flips CW <-> CCW
    }
    Serial.print(F("# sweep: "));
    Serial.print(g_sweepVolts);
    Serial.print(F(" V "));
    Serial.println(g_sweepCW ? F("CW") : F("CCW"));
  }
}

// Current sweep command as a signed voltage (sign = direction) for computeDrive.
static float sweepVolts() {
  return (g_sweepCW ? 1.0f : -1.0f) * (float)g_sweepVolts;
}

// ===========================================================================
//  Serial command parser
//    v <float>   set manual command volts (-12..12); switches to MANUAL
//    brake       short brake (== v 0)
//    stop        coast / Hi-Z (free spin)
//    target <r>  PID target angle in radians (output shaft)
//    pid on|off  enable/disable position PID
//    zero        reset encoder count to 0
//    telem on|off toggle telemetry streaming
//    help        list commands
// ===========================================================================
static void printHelp() {
  Serial.println(F("# commands:"));
  Serial.println(F("#   v <volts>     manual drive, -12..12 (|v|<=0.05 = brake)"));
  Serial.println(F("#   sweep on|off  auto gain ramp: +1 V/10s, reset+reverse at 12 V"));
  Serial.println(F("#   brake         short brake (holds position)"));
  Serial.println(F("#   stop          coast / Hi-Z (free spin)"));
  Serial.println(F("#   target <rad>  PID target angle (output shaft)"));
  Serial.println(F("#   pid on|off    enable/disable position PID"));
  Serial.println(F("#   zero          reset encoder count to 0"));
  Serial.println(F("#   telem on|off  telemetry streaming"));
  Serial.println(F("#   help          this list"));
}

static void printTelemetryHeader() {
  Serial.println(F("# t_s,theta_wrap_rad,omega_rad_s,duty,DIR1,DIR2,mode"));
}

static void handleCommand(char* line) {
  // tokenize on space
  char* cmd = strtok(line, " \t");
  if (cmd == NULL) return;

  if (strcmp(cmd, "v") == 0) {
    char* arg = strtok(NULL, " \t");
    if (arg) {
      g_commandV = clampVolts(atof(arg));
      g_mode = MODE_MANUAL;
      Serial.print(F("# v = ")); Serial.println(g_commandV, 3);
    } else {
      Serial.println(F("# usage: v <volts>"));
    }
  } else if (strcmp(cmd, "sweep") == 0) {
    char* arg = strtok(NULL, " \t");
    if (arg && strcmp(arg, "off") == 0) {
      g_mode = MODE_MANUAL;
      g_commandV = 0.0f;          // fall back to brake
      Serial.println(F("# sweep OFF (braked)"));
    } else {                      // "on" or bare "sweep" -> start
      sweepStart();
      Serial.println(F("# sweep ON: 2->12 V, +1 V/10s, reset+reverse at top"));
    }
  } else if (strcmp(cmd, "brake") == 0) {
    g_commandV = 0.0f;            // |v|<=0.05 -> brake mapping
    g_mode = MODE_MANUAL;
    Serial.println(F("# brake"));
  } else if (strcmp(cmd, "stop") == 0) {
    g_mode = MODE_COAST;
    Serial.println(F("# stop (coast)"));
  } else if (strcmp(cmd, "target") == 0) {
    char* arg = strtok(NULL, " \t");
    if (arg) {
      g_targetTheta = atof(arg);
      Serial.print(F("# target = ")); Serial.println(g_targetTheta, 4);
    } else {
      Serial.println(F("# usage: target <rad>"));
    }
  } else if (strcmp(cmd, "pid") == 0) {
    char* arg = strtok(NULL, " \t");
    if (arg && strcmp(arg, "on") == 0) {
      g_pid.reset();
      g_mode = MODE_PID;
      Serial.println(F("# pid ON"));
    } else if (arg && strcmp(arg, "off") == 0) {
      g_mode = MODE_MANUAL;
      g_commandV = 0.0f;          // fall back to brake
      Serial.println(F("# pid OFF"));
    } else {
      Serial.println(F("# usage: pid on|off"));
    }
  } else if (strcmp(cmd, "zero") == 0) {
    noInterrupts();
    g_encoderCount = 0;
    interrupts();
    g_velFilter.reset();
    Serial.println(F("# encoder zeroed"));
  } else if (strcmp(cmd, "telem") == 0) {
    char* arg = strtok(NULL, " \t");
    g_telemetry = !(arg && strcmp(arg, "off") == 0);
    if (g_telemetry) printTelemetryHeader();
    Serial.print(F("# telem ")); Serial.println(g_telemetry ? F("ON") : F("OFF"));
  } else if (strcmp(cmd, "help") == 0) {
    printHelp();
  } else {
    Serial.print(F("# unknown command: ")); Serial.println(cmd);
  }
}

// Non-blocking line reader. Accepts CR, LF, or CRLF as the line terminator, so
// it works no matter what "line ending" the serial monitor is configured to
// send when you press Enter.
static void pollSerial() {
  static char buf[48];
  static uint8_t len = 0;
  while (Serial.available()) {
    char ch = (char)Serial.read();
    if (ch == '\n' || ch == '\r') {     // end of line (any convention)
      if (len > 0) {                    // ignore empty lines (e.g. LF after CR)
        buf[len] = '\0';
        handleCommand(buf);
        len = 0;
      }
    } else if (len < sizeof(buf) - 1) {
      buf[len++] = ch;
    } else {
      len = 0;   // overflow -> drop the line
    }
  }
}

// ===========================================================================
//  100 Hz control step
// ===========================================================================
static void controlStep() {
  long  count = readCount();
  float theta = countToTheta(count);
  float omega = g_velFilter.update(theta, DT_S);
  float thetaWrapped = countToWrappedTheta(count);   // [0, 2pi): LED + telemetry

  // LED "position dial": rescale the shaft angle [0, 2pi) -> PWM [0, 255] on D11.
  // Runs every step, in every mode, so the LED brightness always tracks the
  // shaft angle -- the encoder standing in for a potentiometer.
  analogWrite(PIN_LED, angleToPwm(thetaWrapped));

  switch (g_mode) {
    case MODE_COAST:
      applyCoast();
      break;

    case MODE_PID: {
      float v = g_pid.update(g_targetTheta, theta);  // gains/dt set in setup()
      applyDrive(computeDrive(v));
      break;
    }

    case MODE_SWEEP:
      sweepUpdate();                       // step the target when 10 s elapses
      applyDrive(computeDrive(sweepSlew(sweepVolts())));  // soft-start slew to it
      break;

    case MODE_MANUAL:
    default:
      applyDrive(computeDrive(g_commandV));
      break;
  }

  // ---- telemetry (decimated) ----
  static uint8_t decim = 0;
  static unsigned long startMs = 0;
  if (startMs == 0) startMs = millis();
  if (g_telemetry && (++decim >= TELEM_DECIM)) {
    decim = 0;
    float t = (millis() - startMs) * 0.001f;
    Serial.print(t, 3);              Serial.print(',');
    Serial.print(thetaWrapped, 5);   Serial.print(',');
    Serial.print(omega, 4);          Serial.print(',');
    Serial.print(g_lastDuty);        Serial.print(',');
    Serial.print(g_lastDir1 ? 1 : 0);Serial.print(',');
    Serial.print(g_lastDir2 ? 1 : 0);Serial.print(',');
    Serial.println((int)g_mode);
  }
}

// ===========================================================================
//  Arduino entry points
// ===========================================================================
void setup() {
  // Capture + clear the reset cause FIRST (before anything else can touch it).
  // If the board restarts when the motor draws more current, this tells us WHY:
  //   BORF  = brown-out (supply dipped below the BOD threshold -> power problem)
  //   WDRF  = watchdog (firmware hung)     EXTRF = reset button/line
  //   PORF  = power-on (normal cold boot / USB replug)
  uint8_t resetCause = MCUSR;
  MCUSR = 0;

  // Motor outputs: D12/D13 now drive the H-bridge inputs directly (each is
  // PWM'd on its own timer by analogWrite when that direction is active).
  pinMode(PIN_DIR1, OUTPUT);
  pinMode(PIN_DIR2, OUTPUT);

  // Indicator LED (dimmed by shaft angle); start dark.
  pinMode(PIN_LED, OUTPUT);
  analogWrite(PIN_LED, 0);

  // Encoder inputs (module has its own pull-ups; INPUT is fine, use INPUT_PULLUP
  // if your encoder outputs are open-collector).
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);

  // Start braked (|v|=0 -> short brake) so nothing lurches at power-up.
  g_encState = readEncoderState();
  applyDrive(computeDrive(0.0f));

  // Encoder interrupts on D2/D3 = INT4/INT5, "any edge" (CHANGE) => X4 decode.
  // Configured on the registers directly so we can use lean native ISR vectors
  // (ISR(INT4_vect)/ISR(INT5_vect)) instead of the much slower attachInterrupt.
  //   EICRB: ISCn1:ISCn0 = 01 selects "any logical change".
  //     INT4 -> ISC41(bit1):ISC40(bit0),  INT5 -> ISC51(bit3):ISC50(bit2)
  noInterrupts();
  EICRB = (uint8_t)((EICRB & ~((1 << ISC41) | (1 << ISC40) | (1 << ISC51) | (1 << ISC50)))
                    | (1 << ISC40) | (1 << ISC50));   // both = any-edge
  EIFR  = (uint8_t)((1 << INTF4) | (1 << INTF5));      // clear any stale flags
  EIMSK |= (uint8_t)((1 << INT4) | (1 << INT5));       // enable INT4, INT5
  interrupts();

  // PID skeleton config (inactive until `pid on`).
  g_pid.kp = PID_KP;
  g_pid.ki = PID_KI;
  g_pid.kd = PID_KD;
  g_pid.dt = DT_S;

  Serial.begin(115200);
  Serial.println(F("# MSE312 directional control ready (default: manual, braked)"));
  Serial.print(F("# last reset: 0x")); Serial.print(resetCause, HEX);
  if (resetCause & (1 << BORF))  Serial.print(F(" BROWN-OUT"));
  if (resetCause & (1 << WDRF))  Serial.print(F(" WATCHDOG"));
  if (resetCause & (1 << EXTRF)) Serial.print(F(" EXTERNAL"));
  if (resetCause & (1 << PORF))  Serial.print(F(" POWER-ON"));
  Serial.println();
  Serial.println(F("# telemetry OFF -- type 'telem on' to stream CSV"));
  printHelp();
}

void loop() {
  // Service serial as fast as possible (non-blocking).
  pollSerial();

  // Fire the control step on a fixed 100 Hz cadence using micros().
  static unsigned long nextTick = 0;
  unsigned long now = micros();
  if (nextTick == 0) nextTick = now;
  if ((long)(now - nextTick) >= 0) {
    nextTick += DT_US;            // advance by a fixed step (no drift)
    // If we fell badly behind (e.g. a long serial burst), resync instead of
    // bursting many catch-up steps.
    if ((long)(micros() - nextTick) >= (long)DT_US) nextTick = micros() + DT_US;
    controlStep();
  }
}
