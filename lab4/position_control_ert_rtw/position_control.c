/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: position_control.c
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
#include "rtwtypes.h"
#include "rt_nonfinite.h"
#include "position_control_private.h"
#include <math.h>
#include "position_control_types.h"
#include <float.h>

/* Block signals (default storage) */
B_position_control_T position_control_B;

/* Block states (default storage) */
DW_position_control_T position_control_DW;

/* Real-time model */
static RT_MODEL_position_control_T position_control_M_;
RT_MODEL_position_control_T *const position_control_M = &position_control_M_;
real_T rt_modd_snf(real_T u0, real_T u1)
{
  real_T q;
  real_T y;
  boolean_T yEq;
  y = u0;
  if (u1 == 0.0) {
    if (u0 == 0.0) {
      y = u1;
    }
  } else if (rtIsNaN(u0) || rtIsNaN(u1) || rtIsInf(u0)) {
    y = (rtNaN);
  } else if (u0 == 0.0) {
    y = 0.0 / u1;
  } else if (rtIsInf(u1)) {
    if ((u1 < 0.0) != (u0 < 0.0)) {
      y = u1;
    }
  } else {
    y = fmod(u0, u1);
    yEq = (y == 0.0);
    if ((!yEq) && (u1 > floor(u1))) {
      q = fabs(u0 / u1);
      yEq = !(fabs(q - floor(q + 0.5)) > DBL_EPSILON * q);
    }

    if (yEq) {
      y = u1 * 0.0;
    } else if ((u0 < 0.0) != (u1 < 0.0)) {
      y += u1;
    }
  }

  return y;
}

/* Model step function */
void position_control_step(void)
{
  /* local block i/o variables */
  real_T rtb_FilterCoefficient;
  real_T rtb_Switch;
  codertarget_arduinobase_inter_T *obj;
  real_T angle;
  real_T rtb_Integrator_c;
  real_T temp;
  int32_T rtb_M1V4MiddleConnector23_0;
  int8_T tmp;
  int8_T tmp_0;

  /* Gain: '<S57>/Minimum sampling to time constant ratio' */
  rtb_Integrator_c = position_control_P.FilteredDerivativeDiscreteorC_m *
    position_control_B.Probe[0];

  /* MinMax: '<S57>/MinMax' incorporates:
   *  Constant: '<S57>/Time constant'
   */
  if ((!(rtb_Integrator_c >= position_control_P.FilteredDerivativeDiscreteorC_a))
      && (!rtIsNaN(position_control_P.FilteredDerivativeDiscreteorC_a))) {
    rtb_Integrator_c = position_control_P.FilteredDerivativeDiscreteorC_a;
  }

  /* End of MinMax: '<S57>/MinMax' */

  /* MATLABSystem: '<S3>/M1V4 Middle Connector 2,3' */
  /*         %% Define output properties */
  /*  Call: int enc_output(int enc) */
  rtb_M1V4MiddleConnector23_0 = enc_output(0.0);

  /* MATLAB Function: '<S3>/MATLAB Function' incorporates:
   *  DataTypeConversion: '<S3>/Data Type Conversion'
   *  MATLABSystem: '<S3>/M1V4 Middle Connector 2,3'
   */
  angle = (real_T)rtb_M1V4MiddleConnector23_0 * 0.00049866550056980839;

  /* DiscreteIntegrator: '<S61>/Integrator' incorporates:
   *  Constant: '<S55>/Constant'
   */
  if (position_control_DW.Integrator_IC_LOADING != 0) {
    position_control_DW.Integrator_DSTATE = angle;
    if (position_control_DW.Integrator_DSTATE >=
        position_control_P.Integrator_UpperSat) {
      position_control_DW.Integrator_DSTATE =
        position_control_P.Integrator_UpperSat;
    } else if (position_control_DW.Integrator_DSTATE <=
               position_control_P.Integrator_LowerSat) {
      position_control_DW.Integrator_DSTATE =
        position_control_P.Integrator_LowerSat;
    }
  }

  if ((position_control_P.Constant_Value_h != 0.0) ||
      (position_control_DW.Integrator_PrevResetState != 0)) {
    position_control_DW.Integrator_DSTATE = angle;
    if (position_control_DW.Integrator_DSTATE >=
        position_control_P.Integrator_UpperSat) {
      position_control_DW.Integrator_DSTATE =
        position_control_P.Integrator_UpperSat;
    } else if (position_control_DW.Integrator_DSTATE <=
               position_control_P.Integrator_LowerSat) {
      position_control_DW.Integrator_DSTATE =
        position_control_P.Integrator_LowerSat;
    }
  }

  if (position_control_DW.Integrator_DSTATE >=
      position_control_P.Integrator_UpperSat) {
    position_control_DW.Integrator_DSTATE =
      position_control_P.Integrator_UpperSat;
  } else if (position_control_DW.Integrator_DSTATE <=
             position_control_P.Integrator_LowerSat) {
    position_control_DW.Integrator_DSTATE =
      position_control_P.Integrator_LowerSat;
  }

  /* Saturate: '<S61>/Saturation' incorporates:
   *  DiscreteIntegrator: '<S61>/Integrator'
   */
  if (position_control_DW.Integrator_DSTATE >
      position_control_P.Saturation_UpperSat) {
    temp = position_control_P.Saturation_UpperSat;
  } else if (position_control_DW.Integrator_DSTATE <
             position_control_P.Saturation_LowerSat) {
    temp = position_control_P.Saturation_LowerSat;
  } else {
    temp = position_control_DW.Integrator_DSTATE;
  }

  /* Product: '<S55>/1//T' incorporates:
   *  Fcn: '<S57>/Avoid Divide by Zero'
   *  Saturate: '<S61>/Saturation'
   *  Sum: '<S55>/Sum1'
   */
  position_control_B.uT = 1.0 / ((real_T)(rtb_Integrator_c == 0.0) *
    2.2204460492503131e-16 + rtb_Integrator_c) * (angle - temp);

  /* Gain: '<S55>/Gain' */
  position_control_B.AB = position_control_P.FilteredDerivativeDiscreteorC_i *
    position_control_B.uT;

  /* Saturate: '<S55>/[A,B]' */
  if (position_control_B.AB > position_control_P.FilteredDerivativeDiscreteorC_p)
  {
    /* Gain: '<S55>/Gain' incorporates:
     *  Saturate: '<S55>/[A,B]'
     */
    position_control_B.AB = position_control_P.FilteredDerivativeDiscreteorC_p;
  } else if (position_control_B.AB <
             position_control_P.FilteredDerivativeDiscreteorCon) {
    /* Gain: '<S55>/Gain' incorporates:
     *  Saturate: '<S55>/[A,B]'
     */
    position_control_B.AB = position_control_P.FilteredDerivativeDiscreteorCon;
  }

  /* End of Saturate: '<S55>/[A,B]' */
  /* Math: '<S3>/Mod' incorporates:
   *  Constant: '<S3>/Constant1'
   */
  position_control_B.Mod = rt_modd_snf(angle,
    position_control_P.Constant1_Value_g);

  /* SignalGenerator: '<Root>/Square Wave Generator' incorporates:
   *  SignalGenerator: '<Root>/Signal Generator'
   */
  angle = position_control_M->Timing.t[0];
  rtb_Integrator_c = position_control_P.SquareWaveGenerator_Frequency * angle;

  /* SignalGenerator: '<Root>/Sawtooth Generator' incorporates:
   *  SignalGenerator: '<Root>/Square Wave Generator'
   */
  temp = position_control_P.SawtoothGenerator_Frequency * angle;

  /* ManualSwitch: '<Root>/Manual Switch2' incorporates:
   *  ManualSwitch: '<Root>/Manual Switch1'
   *  SignalGenerator: '<Root>/Sawtooth Generator'
   */
  if (position_control_P.ManualSwitch2_CurrentSetting == 1) {
    /* ManualSwitch: '<Root>/Manual Switch' incorporates:
     *  Constant: '<Root>/Constant'
     *  SignalGenerator: '<Root>/Signal Generator'
     */
    if (position_control_P.ManualSwitch_CurrentSetting == 1) {
      angle = sin(position_control_P.SignalGenerator_Frequency * angle) *
        position_control_P.SignalGenerator_Amplitude;
    } else {
      angle = position_control_P.Constant_Value_c;
    }

    /* End of ManualSwitch: '<Root>/Manual Switch' */
  } else if (position_control_P.ManualSwitch1_CurrentSetting == 1) {
    /* SignalGenerator: '<Root>/Square Wave Generator' */
    if (rtb_Integrator_c - floor(rtb_Integrator_c) >= 0.5) {
      /* ManualSwitch: '<Root>/Manual Switch1' incorporates:
       *  SignalGenerator: '<Root>/Square Wave Generator'
       */
      angle = position_control_P.SquareWaveGenerator_Amplitude;
    } else {
      /* ManualSwitch: '<Root>/Manual Switch1' incorporates:
       *  SignalGenerator: '<Root>/Square Wave Generator'
       */
      angle = -position_control_P.SquareWaveGenerator_Amplitude;
    }
  } else {
    angle = (1.0 - (temp - floor(temp)) * 2.0) *
      position_control_P.SawtoothGenerator_Amplitude;
  }

  /* End of ManualSwitch: '<Root>/Manual Switch2' */

  /* Gain: '<S2>/Slider Gain' */
  position_control_B.SliderGain = position_control_P.SliderGain_gain * angle;

  /* DiscreteIntegrator: '<Root>/Discrete-Time Integrator' */
  position_control_B.DiscreteTimeIntegrator =
    position_control_DW.DiscreteTimeIntegrator_DSTATE;

  /* Sum: '<Root>/Sum' */
  position_control_B.Sum = position_control_B.SliderGain -
    position_control_B.DiscreteTimeIntegrator;

  /* Gain: '<S40>/Filter Coefficient' incorporates:
   *  DiscreteIntegrator: '<S32>/Filter'
   *  Gain: '<S31>/Derivative Gain'
   *  Sum: '<S32>/SumD'
   */
  rtb_FilterCoefficient = (position_control_P.DiscretePIDController_D *
    position_control_B.Sum - position_control_DW.Filter_DSTATE) *
    position_control_P.DiscretePIDController_N;

  /* Sum: '<S46>/Sum' incorporates:
   *  DiscreteIntegrator: '<S37>/Integrator'
   *  Gain: '<S42>/Proportional Gain'
   */
  angle = (position_control_P.DiscretePIDController_P * position_control_B.Sum +
           position_control_DW.Integrator_DSTATE_c) + rtb_FilterCoefficient;

  /* Saturate: '<S44>/Saturation' */
  if (angle > position_control_P.DiscretePIDController_UpperSatu) {
    /* Saturate: '<S44>/Saturation' */
    position_control_B.Saturation_n =
      position_control_P.DiscretePIDController_UpperSatu;
  } else if (angle < position_control_P.DiscretePIDController_LowerSatu) {
    /* Saturate: '<S44>/Saturation' */
    position_control_B.Saturation_n =
      position_control_P.DiscretePIDController_LowerSatu;
  } else {
    /* Saturate: '<S44>/Saturation' */
    position_control_B.Saturation_n = angle;
  }

  /* End of Saturate: '<S44>/Saturation' */
  /* Saturate: '<S3>/Saturation' */
  if (position_control_B.Saturation_n > position_control_P.Saturation_UpperSat_j)
  {
    /* Gain: '<S34>/Integral Gain' */
    rtb_Switch = position_control_P.Saturation_UpperSat_j;
  } else if (position_control_B.Saturation_n <
             position_control_P.Saturation_LowerSat_h) {
    /* Gain: '<S34>/Integral Gain' */
    rtb_Switch = position_control_P.Saturation_LowerSat_h;
  } else {
    /* Gain: '<S34>/Integral Gain' */
    rtb_Switch = position_control_B.Saturation_n;
  }

  /* End of Saturate: '<S3>/Saturation' */

  /* MATLABSystem: '<S3>/Digital Output' incorporates:
   *  Constant: '<S54>/Constant'
   *  RelationalOperator: '<S54>/Compare'
   */
  writeDigitalPin(8, (uint8_T)(rtb_Switch > position_control_P.Constant_Value));

  /* MATLABSystem: '<S3>/PWM' */
  obj = &position_control_DW.obj_l;
  obj->PWMDriverObj.MW_PWM_HANDLE = MW_PWM_GetHandle(9UL);

  /* Gain: '<S3>/Gain1' incorporates:
   *  Abs: '<S3>/Abs'
   */
  rtb_Integrator_c = position_control_P.Gain1_Gain * fabs(rtb_Switch);

  /* MATLABSystem: '<S3>/PWM' */
  if (!(rtb_Integrator_c <= 255.0)) {
    rtb_Integrator_c = 255.0;
  }

  if (!(rtb_Integrator_c >= 0.0)) {
    rtb_Integrator_c = 0.0;
  }

  MW_PWM_SetDutyCycle(position_control_DW.obj_l.PWMDriverObj.MW_PWM_HANDLE,
                      rtb_Integrator_c);

  /* DeadZone: '<S30>/DeadZone' */
  if (angle > position_control_P.DiscretePIDController_UpperSatu) {
    angle -= position_control_P.DiscretePIDController_UpperSatu;
  } else if (angle >= position_control_P.DiscretePIDController_LowerSatu) {
    angle = 0.0;
  } else {
    angle -= position_control_P.DiscretePIDController_LowerSatu;
  }

  /* End of DeadZone: '<S30>/DeadZone' */

  /* Gain: '<S34>/Integral Gain' */
  rtb_Switch = position_control_P.DiscretePIDController_I *
    position_control_B.Sum;

  /* Switch: '<S28>/Switch1' incorporates:
   *  Constant: '<S28>/Clamping_zero'
   *  Constant: '<S28>/Constant'
   *  Constant: '<S28>/Constant2'
   *  RelationalOperator: '<S28>/fix for DT propagation issue'
   */
  if (angle > position_control_P.Clamping_zero_Value) {
    tmp = position_control_P.Constant_Value_l;
  } else {
    tmp = position_control_P.Constant2_Value;
  }

  /* Switch: '<S28>/Switch2' incorporates:
   *  Constant: '<S28>/Clamping_zero'
   *  Constant: '<S28>/Constant3'
   *  Constant: '<S28>/Constant4'
   *  RelationalOperator: '<S28>/fix for DT propagation issue1'
   */
  if (rtb_Switch > position_control_P.Clamping_zero_Value) {
    tmp_0 = position_control_P.Constant3_Value;
  } else {
    tmp_0 = position_control_P.Constant4_Value;
  }

  /* Switch: '<S28>/Switch' incorporates:
   *  Constant: '<S28>/Clamping_zero'
   *  Logic: '<S28>/AND3'
   *  RelationalOperator: '<S28>/Equal1'
   *  RelationalOperator: '<S28>/Relational Operator'
   *  Switch: '<S28>/Switch1'
   *  Switch: '<S28>/Switch2'
   */
  if ((position_control_P.Clamping_zero_Value != angle) && (tmp == tmp_0)) {
    /* Gain: '<S34>/Integral Gain' incorporates:
     *  Constant: '<S28>/Constant1'
     *  Switch: '<S28>/Switch'
     */
    rtb_Switch = position_control_P.Constant1_Value;
  }

  /* End of Switch: '<S28>/Switch' */

  /* Update for DiscreteIntegrator: '<S61>/Integrator' incorporates:
   *  Constant: '<S55>/Constant'
   */
  position_control_DW.Integrator_IC_LOADING = 0U;
  position_control_DW.Integrator_DSTATE += position_control_P.Integrator_gainval
    * position_control_B.uT;
  if (position_control_DW.Integrator_DSTATE >=
      position_control_P.Integrator_UpperSat) {
    position_control_DW.Integrator_DSTATE =
      position_control_P.Integrator_UpperSat;
  } else if (position_control_DW.Integrator_DSTATE <=
             position_control_P.Integrator_LowerSat) {
    position_control_DW.Integrator_DSTATE =
      position_control_P.Integrator_LowerSat;
  }

  if (position_control_P.Constant_Value_h > 0.0) {
    position_control_DW.Integrator_PrevResetState = 1;
  } else if (position_control_P.Constant_Value_h < 0.0) {
    position_control_DW.Integrator_PrevResetState = -1;
  } else if (position_control_P.Constant_Value_h == 0.0) {
    position_control_DW.Integrator_PrevResetState = 0;
  } else {
    position_control_DW.Integrator_PrevResetState = 2;
  }

  /* End of Update for DiscreteIntegrator: '<S61>/Integrator' */

  /* Update for DiscreteIntegrator: '<Root>/Discrete-Time Integrator' */
  position_control_DW.DiscreteTimeIntegrator_DSTATE +=
    position_control_P.DiscreteTimeIntegrator_gainval * position_control_B.AB;

  /* Update for DiscreteIntegrator: '<S37>/Integrator' */
  position_control_DW.Integrator_DSTATE_c +=
    position_control_P.Integrator_gainval_l * rtb_Switch;

  /* Update for DiscreteIntegrator: '<S32>/Filter' */
  position_control_DW.Filter_DSTATE += position_control_P.Filter_gainval *
    rtb_FilterCoefficient;

  {                                    /* Sample time: [0.0s, 0.0s] */
    extmodeErrorCode_T errorCode = EXTMODE_SUCCESS;
    extmodeSimulationTime_T currentTime = (extmodeSimulationTime_T)
      ((position_control_M->Timing.clockTick0 * 1) + 0)
      ;

    /* Trigger External Mode event */
    errorCode = extmodeEvent(0,currentTime);
    if (errorCode != EXTMODE_SUCCESS) {
      /* Code to handle External Mode event errors
         may be added here */
    }
  }

  {                                    /* Sample time: [0.01s, 0.0s] */
    extmodeErrorCode_T errorCode = EXTMODE_SUCCESS;
    extmodeSimulationTime_T currentTime = (extmodeSimulationTime_T)
      ((position_control_M->Timing.clockTick1 * 1) + 0)
      ;

    /* Trigger External Mode event */
    errorCode = extmodeEvent(1,currentTime);
    if (errorCode != EXTMODE_SUCCESS) {
      /* Code to handle External Mode event errors
         may be added here */
    }
  }

  /* Update absolute time for base rate */
  /* The "clockTick0" counts the number of times the code of this task has
   * been executed. The absolute time is the multiplication of "clockTick0"
   * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
   * overflow during the application lifespan selected.
   */
  position_control_M->Timing.t[0] =
    ((time_T)(++position_control_M->Timing.clockTick0)) *
    position_control_M->Timing.stepSize0;

  {
    /* Update absolute timer for sample time: [0.01s, 0.0s] */
    /* The "clockTick1" counts the number of times the code of this task has
     * been executed. The resolution of this integer timer is 0.01, which is the step size
     * of the task. Size of "clockTick1" ensures timer will not overflow during the
     * application lifespan selected.
     */
    position_control_M->Timing.clockTick1++;
  }
}

/* Model initialize function */
void position_control_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* non-finite (run-time) assignments */
  position_control_P.FilteredDerivativeDiscreteorCon = rtMinusInf;
  position_control_P.FilteredDerivativeDiscreteorC_p = rtInf;
  position_control_P.Integrator_UpperSat = rtInf;
  position_control_P.Integrator_LowerSat = rtMinusInf;
  position_control_P.Saturation_UpperSat = rtInf;
  position_control_P.Saturation_LowerSat = rtMinusInf;

  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&position_control_M->solverInfo,
                          &position_control_M->Timing.simTimeStep);
    rtsiSetTPtr(&position_control_M->solverInfo, &rtmGetTPtr(position_control_M));
    rtsiSetStepSizePtr(&position_control_M->solverInfo,
                       &position_control_M->Timing.stepSize0);
    rtsiSetErrorStatusPtr(&position_control_M->solverInfo, (&rtmGetErrorStatus
      (position_control_M)));
    rtsiSetRTModelPtr(&position_control_M->solverInfo, position_control_M);
  }

  rtsiSetSimTimeStep(&position_control_M->solverInfo, MAJOR_TIME_STEP);
  rtsiSetSolverName(&position_control_M->solverInfo,"FixedStepDiscrete");
  rtmSetTPtr(position_control_M, &position_control_M->Timing.tArray[0]);
  rtmSetTFinal(position_control_M, -1);
  position_control_M->Timing.stepSize0 = 0.01;

  /* External mode info */
  position_control_M->Sizes.checksums[0] = (2992833657U);
  position_control_M->Sizes.checksums[1] = (486786571U);
  position_control_M->Sizes.checksums[2] = (2420211508U);
  position_control_M->Sizes.checksums[3] = (169196995U);

  {
    static const sysRanDType rtAlwaysEnabled = SUBSYS_RAN_BC_ENABLE;
    static RTWExtModeInfo rt_ExtModeInfo;
    static const sysRanDType *systemRan[11];
    position_control_M->extModeInfo = (&rt_ExtModeInfo);
    rteiSetSubSystemActiveVectorAddresses(&rt_ExtModeInfo, systemRan);
    systemRan[0] = &rtAlwaysEnabled;
    systemRan[1] = &rtAlwaysEnabled;
    systemRan[2] = &rtAlwaysEnabled;
    systemRan[3] = &rtAlwaysEnabled;
    systemRan[4] = &rtAlwaysEnabled;
    systemRan[5] = &rtAlwaysEnabled;
    systemRan[6] = &rtAlwaysEnabled;
    systemRan[7] = &rtAlwaysEnabled;
    systemRan[8] = &rtAlwaysEnabled;
    systemRan[9] = &rtAlwaysEnabled;
    systemRan[10] = &rtAlwaysEnabled;
    rteiSetModelMappingInfoPtr(position_control_M->extModeInfo,
      &position_control_M->SpecialInfo.mappingInfo);
    rteiSetChecksumsPtr(position_control_M->extModeInfo,
                        position_control_M->Sizes.checksums);
    rteiSetTPtr(position_control_M->extModeInfo, rtmGetTPtr(position_control_M));
  }

  {
    codertarget_arduinobase_inter_T *obj;

    /* Start for Probe: '<S57>/Probe' */
    position_control_B.Probe[0] = 0.01;
    position_control_B.Probe[1] = 0.0;

    /* InitializeConditions for DiscreteIntegrator: '<S61>/Integrator' */
    position_control_DW.Integrator_IC_LOADING = 1U;

    /* InitializeConditions for DiscreteIntegrator: '<Root>/Discrete-Time Integrator' */
    position_control_DW.DiscreteTimeIntegrator_DSTATE =
      position_control_P.DiscreteTimeIntegrator_IC;

    /* InitializeConditions for DiscreteIntegrator: '<S37>/Integrator' */
    position_control_DW.Integrator_DSTATE_c =
      position_control_P.DiscretePIDController_Initial_l;

    /* InitializeConditions for DiscreteIntegrator: '<S32>/Filter' */
    position_control_DW.Filter_DSTATE =
      position_control_P.DiscretePIDController_InitialCo;

    /* Start for MATLABSystem: '<S3>/M1V4 Middle Connector 2,3' */
    /*  Constructor */
    /*  Support name-value pair arguments when constructing the object. */
    position_control_DW.obj_j.matlabCodegenIsDeleted = false;

    /*  Do not generate code for sprintf */
    position_control_DW.obj_j.isInitialized = 1L;

    /*         %% Define output properties */
    /*    Call: void enc_init(int enc, int pinA, int pinB) */
    enc_init(0.0, 2.0, 3.0);

    /*                  if(obj.PWMTimer> 0) */
    /*                      coder.cinclude('PWMFSelect.h'); */
    /*                      coder.ceval('PWM_Select', obj.PWMFSelect, obj.PWMTimer); */
    /*                      disp('skipp!!!') */
    /*                  end */
    position_control_DW.obj_j.isSetupComplete = true;

    /* Start for MATLABSystem: '<S3>/Digital Output' */
    position_control_DW.obj.matlabCodegenIsDeleted = false;
    position_control_DW.obj.isInitialized = 1L;
    digitalIOSetup(8, 1);
    position_control_DW.obj.isSetupComplete = true;

    /* Start for MATLABSystem: '<S3>/PWM' */
    position_control_DW.obj_l.matlabCodegenIsDeleted = false;
    obj = &position_control_DW.obj_l;
    position_control_DW.obj_l.isInitialized = 1L;
    obj->PWMDriverObj.MW_PWM_HANDLE = MW_PWM_Open(9UL, 0.0, 0.0);
    position_control_DW.obj_l.isSetupComplete = true;
  }
}

/* Model terminate function */
void position_control_terminate(void)
{
  codertarget_arduinobase_inter_T *obj;

  /* Terminate for MATLABSystem: '<S3>/M1V4 Middle Connector 2,3' */
  if (!position_control_DW.obj_j.matlabCodegenIsDeleted) {
    position_control_DW.obj_j.matlabCodegenIsDeleted = true;
  }

  /* End of Terminate for MATLABSystem: '<S3>/M1V4 Middle Connector 2,3' */
  /* Terminate for MATLABSystem: '<S3>/Digital Output' */
  if (!position_control_DW.obj.matlabCodegenIsDeleted) {
    position_control_DW.obj.matlabCodegenIsDeleted = true;
  }

  /* End of Terminate for MATLABSystem: '<S3>/Digital Output' */

  /* Terminate for MATLABSystem: '<S3>/PWM' */
  obj = &position_control_DW.obj_l;
  if (!position_control_DW.obj_l.matlabCodegenIsDeleted) {
    position_control_DW.obj_l.matlabCodegenIsDeleted = true;
    if ((position_control_DW.obj_l.isInitialized == 1L) &&
        position_control_DW.obj_l.isSetupComplete) {
      obj->PWMDriverObj.MW_PWM_HANDLE = MW_PWM_GetHandle(9UL);
      MW_PWM_SetDutyCycle(position_control_DW.obj_l.PWMDriverObj.MW_PWM_HANDLE,
                          0.0);
      obj->PWMDriverObj.MW_PWM_HANDLE = MW_PWM_GetHandle(9UL);
      MW_PWM_Close(position_control_DW.obj_l.PWMDriverObj.MW_PWM_HANDLE);
    }
  }

  /* End of Terminate for MATLABSystem: '<S3>/PWM' */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
