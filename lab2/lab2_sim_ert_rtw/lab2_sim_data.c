/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: lab2_sim_data.c
 *
 * Code generated for Simulink model 'lab2_sim'.
 *
 * Model version                  : 1.9
 * Simulink Coder version         : 9.9 (R2023a) 19-Nov-2022
 * C/C++ source code generated on : Mon Jul 13 18:27:41 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Atmel->AVR
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "lab2_sim.h"

/* Block parameters (default storage) */
P_lab2_sim_T lab2_sim_P = {
  /* Mask Parameter: FilteredDerivativeDiscreteorCon
   * Referenced by: '<S2>/[A,B]'
   */
  0.0,

  /* Mask Parameter: FilteredDerivativeDiscreteorC_l
   * Referenced by: '<S2>/[A,B]'
   */
  0.0,

  /* Mask Parameter: DiscreteDerivative_ICPrevScaled
   * Referenced by: '<S1>/UD'
   */
  0.0,

  /* Mask Parameter: FilteredDerivativeDiscreteorC_i
   * Referenced by: '<S2>/Gain'
   */
  1.0,

  /* Mask Parameter: FilteredDerivativeDiscreteorC_g
   * Referenced by: '<S5>/Time constant'
   */
  0.1,

  /* Mask Parameter: FilteredDerivativeDiscreteorC_a
   * Referenced by: '<S5>/Minimum sampling to time constant ratio'
   */
  10.0,

  /* Expression: 0
   * Referenced by: '<S2>/Constant'
   */
  0.0,

  /* Computed Parameter: Integrator_gainval
   * Referenced by: '<S9>/Integrator'
   */
  0.01,

  /* Expression: antiwindupUpperLimit
   * Referenced by: '<S9>/Integrator'
   */
  0.0,

  /* Expression: antiwindupLowerLimit
   * Referenced by: '<S9>/Integrator'
   */
  0.0,

  /* Expression: windupUpperLimit
   * Referenced by: '<S9>/Saturation'
   */
  0.0,

  /* Expression: windupLowerLimit
   * Referenced by: '<S9>/Saturation'
   */
  0.0,

  /* Computed Parameter: TSamp_WtEt
   * Referenced by: '<S1>/TSamp'
   */
  100.0
};

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
