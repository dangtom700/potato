// ============================================================================
//  test_logic.cpp -- host unit tests for the pure control logic
//  Runs on the PlatformIO `native` env (Unity). No hardware required.
//    pio test -e native
// ============================================================================
#include <unity.h>
#include "MotorLogic.h"

void setUp(void)    {}
void tearDown(void) {}

// ---- helper -----------------------------------------------------------------
static void assert_drive(float v, int duty, bool dir1, bool dir2) {
  DriveCmd c = computeDrive(v);
  TEST_ASSERT_EQUAL_INT(duty, c.duty);
  TEST_ASSERT_EQUAL(dir1, c.dir1);
  TEST_ASSERT_EQUAL(dir2, c.dir2);
}

// ===========================================================================
//  drive(v) truth table
//     v >  +0.05 : forward  DIR1=H DIR2=L  duty=round(|v|/12*255)
//     v <  -0.05 : reverse  DIR1=L DIR2=H  duty=round(|v|/12*255)
//     |v| <= 0.05: BRAKE    DIR1=H DIR2=H  duty=255
// ===========================================================================

void test_forward_full(void)      { assert_drive( 12.0f, 255, true,  false); }
void test_forward_half(void)      { assert_drive(  6.0f, 128, true,  false); } // round(127.5)=128
void test_forward_3v5(void)       { assert_drive(  3.5f,  74, true,  false); } // round(74.375)=74
void test_forward_just_above(void){ assert_drive(  0.06f,  1, true,  false); } // round(1.275)=1

void test_reverse_full(void)      { assert_drive(-12.0f, 255, false, true ); }
void test_reverse_half(void)      { assert_drive( -6.0f, 128, false, true ); }
void test_reverse_just_above(void){ assert_drive(-0.06f,   1, false, true ); }

// Brake band: |v| <= 0.05  ->  both DIR high, full PWM (short brake, holds)
void test_brake_zero(void)        { assert_drive(  0.0f,  255, true,  true ); }
void test_brake_pos_edge(void)    { assert_drive(  0.05f, 255, true,  true ); } // boundary is brake
void test_brake_neg_edge(void)    { assert_drive( -0.05f, 255, true,  true ); }
void test_brake_tiny(void)        { assert_drive(  0.01f, 255, true,  true ); }

// Clamping to +/-12 V before mapping
void test_clamp_high(void)        { assert_drive( 20.0f, 255, true,  false); }
void test_clamp_low(void)         { assert_drive(-20.0f, 255, false, true ); }
void test_clamp_huge(void)        { assert_drive(100.0f, 255, true,  false); }

// ===========================================================================
//  count -> theta (output-shaft angle, radians), Q*N*G = 12600
// ===========================================================================
void test_theta_zero(void)     { TEST_ASSERT_FLOAT_WITHIN(1e-4f, 0.0f,            countToTheta(0)); }
void test_theta_full_rev(void) { TEST_ASSERT_FLOAT_WITHIN(1e-3f, TWO_PI_F,        countToTheta(12600)); }
void test_theta_half_rev(void) { TEST_ASSERT_FLOAT_WITHIN(1e-3f, TWO_PI_F / 2.0f, countToTheta(6300)); }
void test_theta_quarter(void)  { TEST_ASSERT_FLOAT_WITHIN(1e-3f, TWO_PI_F / 4.0f, countToTheta(3150)); }
void test_theta_negative(void) { TEST_ASSERT_FLOAT_WITHIN(1e-3f, -TWO_PI_F,       countToTheta(-12600)); }

// ===========================================================================
//  count -> WRAPPED angle in [0, 2*pi): stays bounded across many revolutions
// ===========================================================================
void test_wrap_zero(void)      { TEST_ASSERT_FLOAT_WITHIN(1e-3f, 0.0f,                countToWrappedTheta(0)); }
void test_wrap_full_rev(void)  { TEST_ASSERT_FLOAT_WITHIN(1e-3f, 0.0f,                countToWrappedTheta(12600)); }        // exactly one rev -> 0
void test_wrap_half(void)      { TEST_ASSERT_FLOAT_WITHIN(1e-3f, TWO_PI_F / 2.0f,     countToWrappedTheta(6300)); }
void test_wrap_quarter(void)   { TEST_ASSERT_FLOAT_WITHIN(1e-3f, TWO_PI_F / 4.0f,     countToWrappedTheta(3150)); }
void test_wrap_many_revs(void) { TEST_ASSERT_FLOAT_WITHIN(1e-3f, TWO_PI_F / 4.0f,     countToWrappedTheta(12600L * 10 + 3150)); } // 10 revs + quarter
void test_wrap_negative(void)  { TEST_ASSERT_FLOAT_WITHIN(1e-3f, 3.0f * TWO_PI_F / 4.0f, countToWrappedTheta(-3150)); }        // -pi/2 -> 3pi/2
void test_wrap_big_negative(void){ TEST_ASSERT_FLOAT_WITHIN(1e-3f, 3.0f * TWO_PI_F / 4.0f, countToWrappedTheta(-12600L * 7 - 3150)); }

// ===========================================================================
//  angle -> LED PWM: [0, 2*pi) rescaled to [0, 255] (encoder as a dimmer dial)
// ===========================================================================
void test_pwm_zero(void)      { TEST_ASSERT_EQUAL_INT(0,   angleToPwm(0.0f)); }
void test_pwm_quarter(void)   { TEST_ASSERT_EQUAL_INT(64,  angleToPwm(TWO_PI_F / 4.0f)); }  // round(63.75)
void test_pwm_half(void)      { TEST_ASSERT_EQUAL_INT(128, angleToPwm(TWO_PI_F / 2.0f)); }  // round(127.5)
void test_pwm_near_top(void)  { TEST_ASSERT_EQUAL_INT(255, angleToPwm(countToWrappedTheta(12599))); } // just under a rev
void test_pwm_clamp_high(void){ TEST_ASSERT_EQUAL_INT(255, angleToPwm(TWO_PI_F * 1.01f)); } // past the seam -> clamp, not 258
void test_pwm_clamp_neg(void) { TEST_ASSERT_EQUAL_INT(0,   angleToPwm(-0.1f)); }            // negative slop -> clamp, not < 0

// ===========================================================================
//  bonus: X4 quadrature decode direction
// ===========================================================================
void test_quad_forward(void) {
  // forward Gray sequence of state=(A<<1)|B : 0 -> 1 -> 3 -> 2 -> 0
  TEST_ASSERT_EQUAL_INT(+1, quadDelta(0, 1));
  TEST_ASSERT_EQUAL_INT(+1, quadDelta(1, 3));
  TEST_ASSERT_EQUAL_INT(+1, quadDelta(3, 2));
  TEST_ASSERT_EQUAL_INT(+1, quadDelta(2, 0));
}
void test_quad_reverse(void) {
  // reverse: 0 -> 2 -> 3 -> 1 -> 0
  TEST_ASSERT_EQUAL_INT(-1, quadDelta(0, 2));
  TEST_ASSERT_EQUAL_INT(-1, quadDelta(2, 3));
  TEST_ASSERT_EQUAL_INT(-1, quadDelta(3, 1));
  TEST_ASSERT_EQUAL_INT(-1, quadDelta(1, 0));
}
void test_quad_no_move_and_illegal(void) {
  TEST_ASSERT_EQUAL_INT(0, quadDelta(2, 2)); // no change
  TEST_ASSERT_EQUAL_INT(0, quadDelta(0, 3)); // illegal double transition
  TEST_ASSERT_EQUAL_INT(0, quadDelta(1, 2)); // illegal double transition
}
void test_quad_full_rev_sums_to_4(void) {
  // one X4 electrical cycle = 4 counts
  int sum = quadDelta(0,1) + quadDelta(1,3) + quadDelta(3,2) + quadDelta(2,0);
  TEST_ASSERT_EQUAL_INT(4, sum);
}

// ===========================================================================
//  bonus: velocity filter is the discrete equivalent of s/(c*s+1)
//     with c = dt = 0.01 :  omega = 0.5*omega_prev + 50*dtheta
// ===========================================================================
void test_velocity_filter_first_sample_zero(void) {
  VelocityFilter f;
  TEST_ASSERT_FLOAT_WITHIN(1e-6f, 0.0f, f.update(1.234f, 0.01f)); // primes, no vel
}
void test_velocity_filter_step_response(void) {
  VelocityFilter f;      // c = 0.01
  f.update(0.0f, 0.01f); // prime at theta=0
  // constant increment of 0.02 rad each 0.01 s -> steady omega should approach 2 rad/s
  float w = 0.0f;
  float theta = 0.0f;
  for (int i = 0; i < 200; ++i) { theta += 0.02f; w = f.update(theta, 0.01f); }
  TEST_ASSERT_FLOAT_WITHIN(1e-2f, 2.0f, w);
}
void test_velocity_filter_recursion(void) {
  VelocityFilter f;
  f.update(0.0f, 0.01f);            // prime
  float w = f.update(0.02f, 0.01f); // dtheta=0.02 -> 0.5*0 + 50*0.02 = 1.0
  TEST_ASSERT_FLOAT_WITHIN(1e-4f, 1.0f, w);
}

// ---------------------------------------------------------------------------
int main(int argc, char **argv) {
  (void)argc; (void)argv;
  UNITY_BEGIN();

  RUN_TEST(test_forward_full);
  RUN_TEST(test_forward_half);
  RUN_TEST(test_forward_3v5);
  RUN_TEST(test_forward_just_above);
  RUN_TEST(test_reverse_full);
  RUN_TEST(test_reverse_half);
  RUN_TEST(test_reverse_just_above);
  RUN_TEST(test_brake_zero);
  RUN_TEST(test_brake_pos_edge);
  RUN_TEST(test_brake_neg_edge);
  RUN_TEST(test_brake_tiny);
  RUN_TEST(test_clamp_high);
  RUN_TEST(test_clamp_low);
  RUN_TEST(test_clamp_huge);

  RUN_TEST(test_theta_zero);
  RUN_TEST(test_theta_full_rev);
  RUN_TEST(test_theta_half_rev);
  RUN_TEST(test_theta_quarter);
  RUN_TEST(test_theta_negative);

  RUN_TEST(test_wrap_zero);
  RUN_TEST(test_wrap_full_rev);
  RUN_TEST(test_wrap_half);
  RUN_TEST(test_wrap_quarter);
  RUN_TEST(test_wrap_many_revs);
  RUN_TEST(test_wrap_negative);
  RUN_TEST(test_wrap_big_negative);

  RUN_TEST(test_pwm_zero);
  RUN_TEST(test_pwm_quarter);
  RUN_TEST(test_pwm_half);
  RUN_TEST(test_pwm_near_top);
  RUN_TEST(test_pwm_clamp_high);
  RUN_TEST(test_pwm_clamp_neg);

  RUN_TEST(test_quad_forward);
  RUN_TEST(test_quad_reverse);
  RUN_TEST(test_quad_no_move_and_illegal);
  RUN_TEST(test_quad_full_rev_sums_to_4);

  RUN_TEST(test_velocity_filter_first_sample_zero);
  RUN_TEST(test_velocity_filter_step_response);
  RUN_TEST(test_velocity_filter_recursion);

  return UNITY_END();
}
