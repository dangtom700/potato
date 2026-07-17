/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: velocity_control_data.c
 *
 * Code generated for Simulink model 'velocity_control'.
 *
 * Model version                  : 1.2
 * Simulink Coder version         : 9.9 (R2023a) 19-Nov-2022
 * C/C++ source code generated on : Fri Jul 17 11:15:16 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Atmel->AVR
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "velocity_control.h"

/* Block parameters (default storage) */
P_velocity_control_T velocity_control_P = {
  /* Mask Parameter: FilteredDerivativeDiscreteorCon
   * Referenced by: '<S53>/[A,B]'
   */
  0.0,

  /* Mask Parameter: FilteredDerivativeDiscreteorC_p
   * Referenced by: '<S53>/[A,B]'
   */
  0.0,

  /* Mask Parameter: DiscretePIDController_D
   * Referenced by: '<S29>/Derivative Gain'
   */
  0.0,

  /* Mask Parameter: DiscretePIDController_I
   * Referenced by: '<S32>/Integral Gain'
   */
  15.3541,

  /* Mask Parameter: DiscretePIDController_InitialCo
   * Referenced by: '<S30>/Filter'
   */
  0.0,

  /* Mask Parameter: DiscretePIDController_Initial_l
   * Referenced by: '<S35>/Integrator'
   */
  0.0,

  /* Mask Parameter: FilteredDerivativeDiscreteorC_i
   * Referenced by: '<S53>/Gain'
   */
  1.0,

  /* Mask Parameter: DiscretePIDController_N
   * Referenced by: '<S38>/Filter Coefficient'
   */
  100.0,

  /* Mask Parameter: DiscretePIDController_P
   * Referenced by: '<S40>/Proportional Gain'
   */
  0.7754,

  /* Mask Parameter: FilteredDerivativeDiscreteorC_a
   * Referenced by: '<S55>/Time constant'
   */
  0.1,

  /* Mask Parameter: SliderGain_gain
   * Referenced by: '<S2>/Slider Gain'
   */
  12.0,

  /* Mask Parameter: FilteredDerivativeDiscreteorC_m
   * Referenced by: '<S55>/Minimum sampling to time constant ratio'
   */
  10.0,

  /* Expression: 0
   * Referenced by: '<S52>/Constant'
   */
  0.0,

  /* Expression: 0
   * Referenced by: '<S53>/Constant'
   */
  0.0,

  /* Computed Parameter: Integrator_gainval
   * Referenced by: '<S59>/Integrator'
   */
  0.01,

  /* Expression: antiwindupUpperLimit
   * Referenced by: '<S59>/Integrator'
   */
  0.0,

  /* Expression: antiwindupLowerLimit
   * Referenced by: '<S59>/Integrator'
   */
  0.0,

  /* Expression: windupUpperLimit
   * Referenced by: '<S59>/Saturation'
   */
  0.0,

  /* Expression: windupLowerLimit
   * Referenced by: '<S59>/Saturation'
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

  /* Computed Parameter: Filter_gainval
   * Referenced by: '<S30>/Filter'
   */
  0.01,

  /* Computed Parameter: Integrator_gainval_l
   * Referenced by: '<S35>/Integrator'
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

  /* Computed Parameter: ManualSwitch_CurrentSetting
   * Referenced by: '<Root>/Manual Switch'
   */
  0U,

  /* Computed Parameter: ManualSwitch1_CurrentSetting
   * Referenced by: '<Root>/Manual Switch1'
   */
  1U,

  /* Computed Parameter: ManualSwitch2_CurrentSetting
   * Referenced by: '<Root>/Manual Switch2'
   */
  0U
};

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
