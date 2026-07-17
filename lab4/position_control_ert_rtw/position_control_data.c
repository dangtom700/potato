/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: position_control_data.c
 *
 * Code generated for Simulink model 'position_control'.
 *
 * Model version                  : 1.5
 * Simulink Coder version         : 9.9 (R2023a) 19-Nov-2022
 * C/C++ source code generated on : Fri Jul 17 13:14:09 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Atmel->AVR
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "position_control.h"

/* Block parameters (default storage) */
P_position_control_T position_control_P = {
  /* Mask Parameter: FilteredDerivativeDiscreteorCon
   * Referenced by: '<S55>/[A,B]'
   */
  0.0,

  /* Mask Parameter: FilteredDerivativeDiscreteorC_p
   * Referenced by: '<S55>/[A,B]'
   */
  0.0,

  /* Mask Parameter: DiscretePIDController_D
   * Referenced by: '<S31>/Derivative Gain'
   */
  0.0016,

  /* Mask Parameter: DiscretePIDController_I
   * Referenced by: '<S34>/Integral Gain'
   */
  0.0,

  /* Mask Parameter: DiscretePIDController_InitialCo
   * Referenced by: '<S32>/Filter'
   */
  0.0,

  /* Mask Parameter: DiscretePIDController_Initial_l
   * Referenced by: '<S37>/Integrator'
   */
  0.0,

  /* Mask Parameter: FilteredDerivativeDiscreteorC_i
   * Referenced by: '<S55>/Gain'
   */
  1.0,

  /* Mask Parameter: DiscretePIDController_LowerSatu
   * Referenced by:
   *   '<S44>/Saturation'
   *   '<S30>/DeadZone'
   */
  -6.0,

  /* Mask Parameter: DiscretePIDController_N
   * Referenced by: '<S40>/Filter Coefficient'
   */
  100.0,

  /* Mask Parameter: DiscretePIDController_P
   * Referenced by: '<S42>/Proportional Gain'
   */
  1.2237,

  /* Mask Parameter: FilteredDerivativeDiscreteorC_a
   * Referenced by: '<S57>/Time constant'
   */
  0.1,

  /* Mask Parameter: DiscretePIDController_UpperSatu
   * Referenced by:
   *   '<S44>/Saturation'
   *   '<S30>/DeadZone'
   */
  6.0,

  /* Mask Parameter: SliderGain_gain
   * Referenced by: '<S2>/Slider Gain'
   */
  3.0,

  /* Mask Parameter: FilteredDerivativeDiscreteorC_m
   * Referenced by: '<S57>/Minimum sampling to time constant ratio'
   */
  10.0,

  /* Expression: 0
   * Referenced by: '<S28>/Constant1'
   */
  0.0,

  /* Expression: 0
   * Referenced by: '<S54>/Constant'
   */
  0.0,

  /* Expression: 0
   * Referenced by: '<S55>/Constant'
   */
  0.0,

  /* Computed Parameter: Integrator_gainval
   * Referenced by: '<S61>/Integrator'
   */
  0.01,

  /* Expression: antiwindupUpperLimit
   * Referenced by: '<S61>/Integrator'
   */
  0.0,

  /* Expression: antiwindupLowerLimit
   * Referenced by: '<S61>/Integrator'
   */
  0.0,

  /* Expression: windupUpperLimit
   * Referenced by: '<S61>/Saturation'
   */
  0.0,

  /* Expression: windupLowerLimit
   * Referenced by: '<S61>/Saturation'
   */
  0.0,

  /* Expression: pi
   * Referenced by: '<S3>/Constant1'
   */
  3.1415926535897931,

  /* Expression: 1
   * Referenced by: '<Root>/Signal Generator'
   */
  1.0,

  /* Expression: 1
   * Referenced by: '<Root>/Signal Generator'
   */
  1.0,

  /* Expression: 1
   * Referenced by: '<Root>/Constant'
   */
  1.0,

  /* Expression: 1
   * Referenced by: '<Root>/Square Wave Generator'
   */
  1.0,

  /* Computed Parameter: SquareWaveGenerator_Frequency
   * Referenced by: '<Root>/Square Wave Generator'
   */
  0.15915494309189535,

  /* Expression: 1
   * Referenced by: '<Root>/Sawtooth Generator'
   */
  1.0,

  /* Computed Parameter: SawtoothGenerator_Frequency
   * Referenced by: '<Root>/Sawtooth Generator'
   */
  0.15915494309189535,

  /* Computed Parameter: DiscreteTimeIntegrator_gainval
   * Referenced by: '<Root>/Discrete-Time Integrator'
   */
  0.01,

  /* Expression: 0
   * Referenced by: '<Root>/Discrete-Time Integrator'
   */
  0.0,

  /* Computed Parameter: Integrator_gainval_l
   * Referenced by: '<S37>/Integrator'
   */
  0.01,

  /* Computed Parameter: Filter_gainval
   * Referenced by: '<S32>/Filter'
   */
  0.01,

  /* Expression: 6
   * Referenced by: '<S3>/Saturation'
   */
  6.0,

  /* Expression: -6
   * Referenced by: '<S3>/Saturation'
   */
  -6.0,

  /* Expression: 255/12
   * Referenced by: '<S3>/Gain1'
   */
  21.25,

  /* Expression: 0
   * Referenced by: '<S28>/Clamping_zero'
   */
  0.0,

  /* Computed Parameter: Constant_Value_l
   * Referenced by: '<S28>/Constant'
   */
  1,

  /* Computed Parameter: Constant2_Value
   * Referenced by: '<S28>/Constant2'
   */
  -1,

  /* Computed Parameter: Constant3_Value
   * Referenced by: '<S28>/Constant3'
   */
  1,

  /* Computed Parameter: Constant4_Value
   * Referenced by: '<S28>/Constant4'
   */
  -1,

  /* Computed Parameter: ManualSwitch_CurrentSetting
   * Referenced by: '<Root>/Manual Switch'
   */
  1U,

  /* Computed Parameter: ManualSwitch1_CurrentSetting
   * Referenced by: '<Root>/Manual Switch1'
   */
  1U,

  /* Computed Parameter: ManualSwitch2_CurrentSetting
   * Referenced by: '<Root>/Manual Switch2'
   */
  1U
};

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
