/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: lab2_sim.c
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
#include "rt_nonfinite.h"
#include <math.h>
#include "rtwtypes.h"
#include "lab2_sim_private.h"

/* Block signals (default storage) */
B_lab2_sim_T lab2_sim_B;

/* Block states (default storage) */
DW_lab2_sim_T lab2_sim_DW;

/* Real-time model */
static RT_MODEL_lab2_sim_T lab2_sim_M_;
RT_MODEL_lab2_sim_T *const lab2_sim_M = &lab2_sim_M_;

/* Model step function */
void lab2_sim_step(void)
{
  real_T q;
  real_T rtb_Integrator;
  real_T rtb_TSamp;
  real_T theta;
  int32_T rtb_M1V4MiddleConnector23_0;
  boolean_T rEQ0;

  /* Gain: '<S5>/Minimum sampling to time constant ratio' */
  rtb_Integrator = lab2_sim_P.FilteredDerivativeDiscreteorC_a *
    lab2_sim_B.Probe[0];

  /* MinMax: '<S5>/MinMax' incorporates:
   *  Constant: '<S5>/Time constant'
   */
  if ((!(rtb_Integrator >= lab2_sim_P.FilteredDerivativeDiscreteorC_g)) &&
      (!rtIsNaN(lab2_sim_P.FilteredDerivativeDiscreteorC_g))) {
    rtb_Integrator = lab2_sim_P.FilteredDerivativeDiscreteorC_g;
  }

  /* End of MinMax: '<S5>/MinMax' */

  /* MATLABSystem: '<Root>/M1V4 Middle Connector 2,3' */
  /*         %% Define output properties */
  /*  Call: int enc_output(int enc) */
  rtb_M1V4MiddleConnector23_0 = enc_output(0.0);

  /* MATLAB Function: '<Root>/MATLAB Function' incorporates:
   *  DataTypeConversion: '<Root>/Cast To Double'
   *  MATLABSystem: '<Root>/M1V4 Middle Connector 2,3'
   */
  theta = (real_T)rtb_M1V4MiddleConnector23_0 * 2.0 * 3.1415926535897931 /
    12600.0;

  /* DiscreteIntegrator: '<S9>/Integrator' incorporates:
   *  Constant: '<S2>/Constant'
   */
  if (lab2_sim_DW.Integrator_IC_LOADING != 0) {
    lab2_sim_DW.Integrator_DSTATE = theta;
    if (lab2_sim_DW.Integrator_DSTATE >= lab2_sim_P.Integrator_UpperSat) {
      lab2_sim_DW.Integrator_DSTATE = lab2_sim_P.Integrator_UpperSat;
    } else if (lab2_sim_DW.Integrator_DSTATE <= lab2_sim_P.Integrator_LowerSat)
    {
      lab2_sim_DW.Integrator_DSTATE = lab2_sim_P.Integrator_LowerSat;
    }
  }

  if ((lab2_sim_P.Constant_Value != 0.0) ||
      (lab2_sim_DW.Integrator_PrevResetState != 0)) {
    lab2_sim_DW.Integrator_DSTATE = theta;
    if (lab2_sim_DW.Integrator_DSTATE >= lab2_sim_P.Integrator_UpperSat) {
      lab2_sim_DW.Integrator_DSTATE = lab2_sim_P.Integrator_UpperSat;
    } else if (lab2_sim_DW.Integrator_DSTATE <= lab2_sim_P.Integrator_LowerSat)
    {
      lab2_sim_DW.Integrator_DSTATE = lab2_sim_P.Integrator_LowerSat;
    }
  }

  if (lab2_sim_DW.Integrator_DSTATE >= lab2_sim_P.Integrator_UpperSat) {
    lab2_sim_DW.Integrator_DSTATE = lab2_sim_P.Integrator_UpperSat;
  } else if (lab2_sim_DW.Integrator_DSTATE <= lab2_sim_P.Integrator_LowerSat) {
    lab2_sim_DW.Integrator_DSTATE = lab2_sim_P.Integrator_LowerSat;
  }

  /* Saturate: '<S9>/Saturation' incorporates:
   *  DiscreteIntegrator: '<S9>/Integrator'
   */
  if (lab2_sim_DW.Integrator_DSTATE > lab2_sim_P.Saturation_UpperSat) {
    rtb_TSamp = lab2_sim_P.Saturation_UpperSat;
  } else if (lab2_sim_DW.Integrator_DSTATE < lab2_sim_P.Saturation_LowerSat) {
    rtb_TSamp = lab2_sim_P.Saturation_LowerSat;
  } else {
    rtb_TSamp = lab2_sim_DW.Integrator_DSTATE;
  }

  /* Product: '<S2>/1//T' incorporates:
   *  Fcn: '<S5>/Avoid Divide by Zero'
   *  Saturate: '<S9>/Saturation'
   *  Sum: '<S2>/Sum1'
   */
  rtb_Integrator = 1.0 / ((real_T)(rtb_Integrator == 0.0) *
    2.2204460492503131e-16 + rtb_Integrator) * (theta - rtb_TSamp);

  /* Gain: '<S2>/Gain' */
  lab2_sim_B.AB = lab2_sim_P.FilteredDerivativeDiscreteorC_i * rtb_Integrator;

  /* Saturate: '<S2>/[A,B]' */
  if (lab2_sim_B.AB > lab2_sim_P.FilteredDerivativeDiscreteorC_l) {
    /* Gain: '<S2>/Gain' incorporates:
     *  Saturate: '<S2>/[A,B]'
     */
    lab2_sim_B.AB = lab2_sim_P.FilteredDerivativeDiscreteorC_l;
  } else if (lab2_sim_B.AB < lab2_sim_P.FilteredDerivativeDiscreteorCon) {
    /* Gain: '<S2>/Gain' incorporates:
     *  Saturate: '<S2>/[A,B]'
     */
    lab2_sim_B.AB = lab2_sim_P.FilteredDerivativeDiscreteorCon;
  }

  /* End of Saturate: '<S2>/[A,B]' */

  /* SampleTimeMath: '<S1>/TSamp'
   *
   * About '<S1>/TSamp':
   *  y = u * K where K = 1 / ( w * Ts )
   */
  rtb_TSamp = theta * lab2_sim_P.TSamp_WtEt;

  /* Sum: '<S1>/Diff' incorporates:
   *  UnitDelay: '<S1>/UD'
   *
   * Block description for '<S1>/Diff':
   *
   *  Add in CPU
   *
   * Block description for '<S1>/UD':
   *
   *  Store in Global RAM
   */
  lab2_sim_B.Diff = rtb_TSamp - lab2_sim_DW.UD_DSTATE;

  /* MATLAB Function: '<Root>/MATLAB Function1' */
  if (theta == 0.0) {
    lab2_sim_B.angle_rad_bounded = 0.0;
  } else {
    lab2_sim_B.angle_rad_bounded = fmod(theta, 6.2831853071795862);
    rEQ0 = (lab2_sim_B.angle_rad_bounded == 0.0);
    if (!rEQ0) {
      q = fabs(theta / 6.2831853071795862);
      rEQ0 = !(fabs(q - floor(q + 0.5)) > 2.2204460492503131E-16 * q);
    }

    if (rEQ0) {
      lab2_sim_B.angle_rad_bounded = 0.0;
    } else if (theta < 0.0) {
      lab2_sim_B.angle_rad_bounded += 6.2831853071795862;
    }
  }

  lab2_sim_B.angle_deg_bounded = 57.295779513082323 *
    lab2_sim_B.angle_rad_bounded;

  /* End of MATLAB Function: '<Root>/MATLAB Function1' */
  /* Update for DiscreteIntegrator: '<S9>/Integrator' incorporates:
   *  Constant: '<S2>/Constant'
   */
  lab2_sim_DW.Integrator_IC_LOADING = 0U;
  lab2_sim_DW.Integrator_DSTATE += lab2_sim_P.Integrator_gainval *
    rtb_Integrator;
  if (lab2_sim_DW.Integrator_DSTATE >= lab2_sim_P.Integrator_UpperSat) {
    lab2_sim_DW.Integrator_DSTATE = lab2_sim_P.Integrator_UpperSat;
  } else if (lab2_sim_DW.Integrator_DSTATE <= lab2_sim_P.Integrator_LowerSat) {
    lab2_sim_DW.Integrator_DSTATE = lab2_sim_P.Integrator_LowerSat;
  }

  if (lab2_sim_P.Constant_Value > 0.0) {
    lab2_sim_DW.Integrator_PrevResetState = 1;
  } else if (lab2_sim_P.Constant_Value < 0.0) {
    lab2_sim_DW.Integrator_PrevResetState = -1;
  } else if (lab2_sim_P.Constant_Value == 0.0) {
    lab2_sim_DW.Integrator_PrevResetState = 0;
  } else {
    lab2_sim_DW.Integrator_PrevResetState = 2;
  }

  /* End of Update for DiscreteIntegrator: '<S9>/Integrator' */

  /* Update for UnitDelay: '<S1>/UD'
   *
   * Block description for '<S1>/UD':
   *
   *  Store in Global RAM
   */
  lab2_sim_DW.UD_DSTATE = rtb_TSamp;

  {                                    /* Sample time: [0.01s, 0.0s] */
  }

  /* Update absolute time for base rate */
  /* The "clockTick0" counts the number of times the code of this task has
   * been executed. The absolute time is the multiplication of "clockTick0"
   * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
   * overflow during the application lifespan selected.
   */
  lab2_sim_M->Timing.taskTime0 =
    ((time_T)(++lab2_sim_M->Timing.clockTick0)) * lab2_sim_M->Timing.stepSize0;
}

/* Model initialize function */
void lab2_sim_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* non-finite (run-time) assignments */
  lab2_sim_P.FilteredDerivativeDiscreteorCon = rtMinusInf;
  lab2_sim_P.FilteredDerivativeDiscreteorC_l = rtInf;
  lab2_sim_P.Integrator_UpperSat = rtInf;
  lab2_sim_P.Integrator_LowerSat = rtMinusInf;
  lab2_sim_P.Saturation_UpperSat = rtInf;
  lab2_sim_P.Saturation_LowerSat = rtMinusInf;
  rtmSetTFinal(lab2_sim_M, -1);
  lab2_sim_M->Timing.stepSize0 = 0.01;

  /* External mode info */
  lab2_sim_M->Sizes.checksums[0] = (2401558168U);
  lab2_sim_M->Sizes.checksums[1] = (3747604862U);
  lab2_sim_M->Sizes.checksums[2] = (3711196973U);
  lab2_sim_M->Sizes.checksums[3] = (3268399248U);

  {
    static const sysRanDType rtAlwaysEnabled = SUBSYS_RAN_BC_ENABLE;
    static RTWExtModeInfo rt_ExtModeInfo;
    static const sysRanDType *systemRan[4];
    lab2_sim_M->extModeInfo = (&rt_ExtModeInfo);
    rteiSetSubSystemActiveVectorAddresses(&rt_ExtModeInfo, systemRan);
    systemRan[0] = &rtAlwaysEnabled;
    systemRan[1] = &rtAlwaysEnabled;
    systemRan[2] = &rtAlwaysEnabled;
    systemRan[3] = &rtAlwaysEnabled;
    rteiSetModelMappingInfoPtr(lab2_sim_M->extModeInfo,
      &lab2_sim_M->SpecialInfo.mappingInfo);
    rteiSetChecksumsPtr(lab2_sim_M->extModeInfo, lab2_sim_M->Sizes.checksums);
    rteiSetTPtr(lab2_sim_M->extModeInfo, rtmGetTPtr(lab2_sim_M));
  }

  /* Start for Probe: '<S5>/Probe' */
  lab2_sim_B.Probe[0] = 0.01;
  lab2_sim_B.Probe[1] = 0.0;

  /* InitializeConditions for DiscreteIntegrator: '<S9>/Integrator' */
  lab2_sim_DW.Integrator_IC_LOADING = 1U;

  /* InitializeConditions for UnitDelay: '<S1>/UD'
   *
   * Block description for '<S1>/UD':
   *
   *  Store in Global RAM
   */
  lab2_sim_DW.UD_DSTATE = lab2_sim_P.DiscreteDerivative_ICPrevScaled;

  /* Start for MATLABSystem: '<Root>/M1V4 Middle Connector 2,3' */
  /*  Constructor */
  /*  Support name-value pair arguments when constructing the object. */
  lab2_sim_DW.obj.matlabCodegenIsDeleted = false;

  /*  Do not generate code for sprintf */
  lab2_sim_DW.obj.isInitialized = 1L;

  /*         %% Define output properties */
  /*    Call: void enc_init(int enc, int pinA, int pinB) */
  enc_init(0.0, 2.0, 3.0);

  /*                  if(obj.PWMTimer> 0) */
  /*                      coder.cinclude('PWMFSelect.h'); */
  /*                      coder.ceval('PWM_Select', obj.PWMFSelect, obj.PWMTimer); */
  /*                      disp('skipp!!!') */
  /*                  end */
  lab2_sim_DW.obj.isSetupComplete = true;
}

/* Model terminate function */
void lab2_sim_terminate(void)
{
  /* Terminate for MATLABSystem: '<Root>/M1V4 Middle Connector 2,3' */
  if (!lab2_sim_DW.obj.matlabCodegenIsDeleted) {
    lab2_sim_DW.obj.matlabCodegenIsDeleted = true;
  }

  /* End of Terminate for MATLABSystem: '<Root>/M1V4 Middle Connector 2,3' */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
