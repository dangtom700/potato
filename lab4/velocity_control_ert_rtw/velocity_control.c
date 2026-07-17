/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: velocity_control.c
 *
 * Code generated for Simulink model 'velocity_control'.
 *
 * Model version                  : 1.7
 * Simulink Coder version         : 9.9 (R2023a) 19-Nov-2022
 * C/C++ source code generated on : Fri Jul 17 13:08:40 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Atmel->AVR
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "velocity_control.h"
#include "rtwtypes.h"
#include "rt_nonfinite.h"
#include "velocity_control_private.h"
#include <math.h>
#include "velocity_control_types.h"
#include <float.h>

/* Block signals (default storage) */
B_velocity_control_T velocity_control_B;

/* Block states (default storage) */
DW_velocity_control_T velocity_control_DW;

/* Real-time model */
static RT_MODEL_velocity_control_T velocity_control_M_;
RT_MODEL_velocity_control_T *const velocity_control_M = &velocity_control_M_;
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
void velocity_control_step(void)
{
  /* local block i/o variables */
  real_T rtb_FilterCoefficient;
  real_T rtb_IntegralGain;
  codertarget_arduinobase_inter_T *obj;
  real_T angle;
  real_T rtb_Integrator_c;
  real_T temp;
  int32_T rtb_M1V4MiddleConnector23_0;

  /* Gain: '<S55>/Minimum sampling to time constant ratio' */
  rtb_Integrator_c = velocity_control_P.FilteredDerivativeDiscreteorC_m *
    velocity_control_B.Probe[0];

  /* MinMax: '<S55>/MinMax' incorporates:
   *  Constant: '<S55>/Time constant'
   */
  if ((!(rtb_Integrator_c >= velocity_control_P.FilteredDerivativeDiscreteorC_a))
      && (!rtIsNaN(velocity_control_P.FilteredDerivativeDiscreteorC_a))) {
    rtb_Integrator_c = velocity_control_P.FilteredDerivativeDiscreteorC_a;
  }

  /* End of MinMax: '<S55>/MinMax' */

  /* MATLABSystem: '<S3>/M1V4 Middle Connector 2,3' */
  /*         %% Define output properties */
  /*  Call: int enc_output(int enc) */
  rtb_M1V4MiddleConnector23_0 = enc_output(0.0);

  /* MATLAB Function: '<S3>/MATLAB Function' incorporates:
   *  DataTypeConversion: '<S3>/Data Type Conversion'
   *  MATLABSystem: '<S3>/M1V4 Middle Connector 2,3'
   */
  angle = (real_T)rtb_M1V4MiddleConnector23_0 * 0.00049866550056980839;

  /* DiscreteIntegrator: '<S59>/Integrator' incorporates:
   *  Constant: '<S53>/Constant'
   */
  if (velocity_control_DW.Integrator_IC_LOADING != 0) {
    velocity_control_DW.Integrator_DSTATE = angle;
    if (velocity_control_DW.Integrator_DSTATE >=
        velocity_control_P.Integrator_UpperSat) {
      velocity_control_DW.Integrator_DSTATE =
        velocity_control_P.Integrator_UpperSat;
    } else if (velocity_control_DW.Integrator_DSTATE <=
               velocity_control_P.Integrator_LowerSat) {
      velocity_control_DW.Integrator_DSTATE =
        velocity_control_P.Integrator_LowerSat;
    }
  }

  if ((velocity_control_P.Constant_Value_h != 0.0) ||
      (velocity_control_DW.Integrator_PrevResetState != 0)) {
    velocity_control_DW.Integrator_DSTATE = angle;
    if (velocity_control_DW.Integrator_DSTATE >=
        velocity_control_P.Integrator_UpperSat) {
      velocity_control_DW.Integrator_DSTATE =
        velocity_control_P.Integrator_UpperSat;
    } else if (velocity_control_DW.Integrator_DSTATE <=
               velocity_control_P.Integrator_LowerSat) {
      velocity_control_DW.Integrator_DSTATE =
        velocity_control_P.Integrator_LowerSat;
    }
  }

  if (velocity_control_DW.Integrator_DSTATE >=
      velocity_control_P.Integrator_UpperSat) {
    velocity_control_DW.Integrator_DSTATE =
      velocity_control_P.Integrator_UpperSat;
  } else if (velocity_control_DW.Integrator_DSTATE <=
             velocity_control_P.Integrator_LowerSat) {
    velocity_control_DW.Integrator_DSTATE =
      velocity_control_P.Integrator_LowerSat;
  }

  /* Saturate: '<S59>/Saturation' incorporates:
   *  DiscreteIntegrator: '<S59>/Integrator'
   */
  if (velocity_control_DW.Integrator_DSTATE >
      velocity_control_P.Saturation_UpperSat) {
    temp = velocity_control_P.Saturation_UpperSat;
  } else if (velocity_control_DW.Integrator_DSTATE <
             velocity_control_P.Saturation_LowerSat) {
    temp = velocity_control_P.Saturation_LowerSat;
  } else {
    temp = velocity_control_DW.Integrator_DSTATE;
  }

  /* Product: '<S53>/1//T' incorporates:
   *  Fcn: '<S55>/Avoid Divide by Zero'
   *  Saturate: '<S59>/Saturation'
   *  Sum: '<S53>/Sum1'
   */
  velocity_control_B.uT = 1.0 / ((real_T)(rtb_Integrator_c == 0.0) *
    2.2204460492503131e-16 + rtb_Integrator_c) * (angle - temp);

  /* Gain: '<S53>/Gain' */
  velocity_control_B.AB = velocity_control_P.FilteredDerivativeDiscreteorC_i *
    velocity_control_B.uT;

  /* Saturate: '<S53>/[A,B]' */
  if (velocity_control_B.AB > velocity_control_P.FilteredDerivativeDiscreteorC_p)
  {
    /* Gain: '<S53>/Gain' incorporates:
     *  Saturate: '<S53>/[A,B]'
     */
    velocity_control_B.AB = velocity_control_P.FilteredDerivativeDiscreteorC_p;
  } else if (velocity_control_B.AB <
             velocity_control_P.FilteredDerivativeDiscreteorCon) {
    /* Gain: '<S53>/Gain' incorporates:
     *  Saturate: '<S53>/[A,B]'
     */
    velocity_control_B.AB = velocity_control_P.FilteredDerivativeDiscreteorCon;
  }

  /* End of Saturate: '<S53>/[A,B]' */
  /* Math: '<S3>/Mod' incorporates:
   *  Constant: '<S3>/Constant1'
   */
  velocity_control_B.Mod = rt_modd_snf(angle, velocity_control_P.Constant1_Value);

  /* SignalGenerator: '<Root>/Square Wave Generator' incorporates:
   *  SignalGenerator: '<Root>/Signal Generator'
   */
  angle = velocity_control_M->Timing.t[0];
  rtb_Integrator_c = velocity_control_P.SquareWaveGenerator_Frequency * angle;

  /* SignalGenerator: '<Root>/Sawtooth Generator' incorporates:
   *  SignalGenerator: '<Root>/Square Wave Generator'
   */
  temp = velocity_control_P.SawtoothGenerator_Frequency * angle;

  /* ManualSwitch: '<Root>/Manual Switch2' incorporates:
   *  ManualSwitch: '<Root>/Manual Switch1'
   *  SignalGenerator: '<Root>/Sawtooth Generator'
   */
  if (velocity_control_P.ManualSwitch2_CurrentSetting == 1) {
    /* ManualSwitch: '<Root>/Manual Switch' incorporates:
     *  Constant: '<Root>/Constant'
     *  SignalGenerator: '<Root>/Signal Generator'
     */
    if (velocity_control_P.ManualSwitch_CurrentSetting == 1) {
      angle = sin(velocity_control_P.SignalGenerator_Frequency * angle) *
        velocity_control_P.SignalGenerator_Amplitude;
    } else {
      angle = velocity_control_P.Constant_Value_c;
    }

    /* End of ManualSwitch: '<Root>/Manual Switch' */
  } else if (velocity_control_P.ManualSwitch1_CurrentSetting == 1) {
    /* SignalGenerator: '<Root>/Square Wave Generator' */
    if (rtb_Integrator_c - floor(rtb_Integrator_c) >= 0.5) {
      /* ManualSwitch: '<Root>/Manual Switch1' incorporates:
       *  SignalGenerator: '<Root>/Square Wave Generator'
       */
      angle = velocity_control_P.SquareWaveGenerator_Amplitude;
    } else {
      /* ManualSwitch: '<Root>/Manual Switch1' incorporates:
       *  SignalGenerator: '<Root>/Square Wave Generator'
       */
      angle = -velocity_control_P.SquareWaveGenerator_Amplitude;
    }
  } else {
    angle = (1.0 - (temp - floor(temp)) * 2.0) *
      velocity_control_P.SawtoothGenerator_Amplitude;
  }

  /* End of ManualSwitch: '<Root>/Manual Switch2' */

  /* Gain: '<S2>/Slider Gain' */
  velocity_control_B.SliderGain = velocity_control_P.SliderGain_gain * angle;

  /* Sum: '<Root>/Sum' */
  velocity_control_B.Sum = velocity_control_B.SliderGain - velocity_control_B.AB;

  /* Gain: '<S38>/Filter Coefficient' incorporates:
   *  DiscreteIntegrator: '<S30>/Filter'
   *  Gain: '<S29>/Derivative Gain'
   *  Sum: '<S30>/SumD'
   */
  rtb_FilterCoefficient = (velocity_control_P.DiscretePIDController_D *
    velocity_control_B.Sum - velocity_control_DW.Filter_DSTATE) *
    velocity_control_P.DiscretePIDController_N;

  /* Sum: '<S44>/Sum' incorporates:
   *  DiscreteIntegrator: '<S35>/Integrator'
   *  Gain: '<S40>/Proportional Gain'
   */
  velocity_control_B.Saturation_n = (velocity_control_P.DiscretePIDController_P *
    velocity_control_B.Sum + velocity_control_DW.Integrator_DSTATE_c) +
    rtb_FilterCoefficient;

  /* Saturate: '<S42>/Saturation' */
  if (velocity_control_B.Saturation_n >
      velocity_control_P.DiscretePIDController_UpperSatu) {
    /* Sum: '<S44>/Sum' incorporates:
     *  Saturate: '<S42>/Saturation'
     */
    velocity_control_B.Saturation_n =
      velocity_control_P.DiscretePIDController_UpperSatu;
  } else if (velocity_control_B.Saturation_n <
             velocity_control_P.DiscretePIDController_LowerSatu) {
    /* Sum: '<S44>/Sum' incorporates:
     *  Saturate: '<S42>/Saturation'
     */
    velocity_control_B.Saturation_n =
      velocity_control_P.DiscretePIDController_LowerSatu;
  }

  /* End of Saturate: '<S42>/Saturation' */
  /* Saturate: '<S3>/Saturation' */
  if (velocity_control_B.Saturation_n > velocity_control_P.Saturation_UpperSat_j)
  {
    angle = velocity_control_P.Saturation_UpperSat_j;
  } else if (velocity_control_B.Saturation_n <
             velocity_control_P.Saturation_LowerSat_h) {
    angle = velocity_control_P.Saturation_LowerSat_h;
  } else {
    angle = velocity_control_B.Saturation_n;
  }

  /* End of Saturate: '<S3>/Saturation' */

  /* MATLABSystem: '<S3>/Digital Output' incorporates:
   *  Constant: '<S52>/Constant'
   *  RelationalOperator: '<S52>/Compare'
   */
  writeDigitalPin(8, (uint8_T)(angle > velocity_control_P.Constant_Value));

  /* MATLABSystem: '<S3>/PWM' */
  obj = &velocity_control_DW.obj_l;
  obj->PWMDriverObj.MW_PWM_HANDLE = MW_PWM_GetHandle(9UL);

  /* Gain: '<S3>/Gain1' incorporates:
   *  Abs: '<S3>/Abs'
   */
  angle = velocity_control_P.Gain1_Gain * fabs(angle);

  /* MATLABSystem: '<S3>/PWM' */
  if (!(angle <= 255.0)) {
    angle = 255.0;
  }

  if (!(angle >= 0.0)) {
    angle = 0.0;
  }

  MW_PWM_SetDutyCycle(velocity_control_DW.obj_l.PWMDriverObj.MW_PWM_HANDLE,
                      angle);

  /* Gain: '<S32>/Integral Gain' */
  rtb_IntegralGain = velocity_control_P.DiscretePIDController_I *
    velocity_control_B.Sum;

  /* Update for DiscreteIntegrator: '<S59>/Integrator' incorporates:
   *  Constant: '<S53>/Constant'
   */
  velocity_control_DW.Integrator_IC_LOADING = 0U;
  velocity_control_DW.Integrator_DSTATE += velocity_control_P.Integrator_gainval
    * velocity_control_B.uT;
  if (velocity_control_DW.Integrator_DSTATE >=
      velocity_control_P.Integrator_UpperSat) {
    velocity_control_DW.Integrator_DSTATE =
      velocity_control_P.Integrator_UpperSat;
  } else if (velocity_control_DW.Integrator_DSTATE <=
             velocity_control_P.Integrator_LowerSat) {
    velocity_control_DW.Integrator_DSTATE =
      velocity_control_P.Integrator_LowerSat;
  }

  if (velocity_control_P.Constant_Value_h > 0.0) {
    velocity_control_DW.Integrator_PrevResetState = 1;
  } else if (velocity_control_P.Constant_Value_h < 0.0) {
    velocity_control_DW.Integrator_PrevResetState = -1;
  } else if (velocity_control_P.Constant_Value_h == 0.0) {
    velocity_control_DW.Integrator_PrevResetState = 0;
  } else {
    velocity_control_DW.Integrator_PrevResetState = 2;
  }

  /* End of Update for DiscreteIntegrator: '<S59>/Integrator' */

  /* Update for DiscreteIntegrator: '<S35>/Integrator' */
  velocity_control_DW.Integrator_DSTATE_c +=
    velocity_control_P.Integrator_gainval_l * rtb_IntegralGain;

  /* Update for DiscreteIntegrator: '<S30>/Filter' */
  velocity_control_DW.Filter_DSTATE += velocity_control_P.Filter_gainval *
    rtb_FilterCoefficient;

  {                                    /* Sample time: [0.0s, 0.0s] */
    extmodeErrorCode_T errorCode = EXTMODE_SUCCESS;
    extmodeSimulationTime_T currentTime = (extmodeSimulationTime_T)
      ((velocity_control_M->Timing.clockTick0 * 1) + 0)
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
      ((velocity_control_M->Timing.clockTick1 * 1) + 0)
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
  velocity_control_M->Timing.t[0] =
    ((time_T)(++velocity_control_M->Timing.clockTick0)) *
    velocity_control_M->Timing.stepSize0;

  {
    /* Update absolute timer for sample time: [0.01s, 0.0s] */
    /* The "clockTick1" counts the number of times the code of this task has
     * been executed. The resolution of this integer timer is 0.01, which is the step size
     * of the task. Size of "clockTick1" ensures timer will not overflow during the
     * application lifespan selected.
     */
    velocity_control_M->Timing.clockTick1++;
  }
}

/* Model initialize function */
void velocity_control_initialize(void)
{
  /* Registration code */

  /* initialize non-finites */
  rt_InitInfAndNaN(sizeof(real_T));

  /* non-finite (run-time) assignments */
  velocity_control_P.FilteredDerivativeDiscreteorCon = rtMinusInf;
  velocity_control_P.FilteredDerivativeDiscreteorC_p = rtInf;
  velocity_control_P.Integrator_UpperSat = rtInf;
  velocity_control_P.Integrator_LowerSat = rtMinusInf;
  velocity_control_P.Saturation_UpperSat = rtInf;
  velocity_control_P.Saturation_LowerSat = rtMinusInf;

  {
    /* Setup solver object */
    rtsiSetSimTimeStepPtr(&velocity_control_M->solverInfo,
                          &velocity_control_M->Timing.simTimeStep);
    rtsiSetTPtr(&velocity_control_M->solverInfo, &rtmGetTPtr(velocity_control_M));
    rtsiSetStepSizePtr(&velocity_control_M->solverInfo,
                       &velocity_control_M->Timing.stepSize0);
    rtsiSetErrorStatusPtr(&velocity_control_M->solverInfo, (&rtmGetErrorStatus
      (velocity_control_M)));
    rtsiSetRTModelPtr(&velocity_control_M->solverInfo, velocity_control_M);
  }

  rtsiSetSimTimeStep(&velocity_control_M->solverInfo, MAJOR_TIME_STEP);
  rtsiSetSolverName(&velocity_control_M->solverInfo,"FixedStepDiscrete");
  rtmSetTPtr(velocity_control_M, &velocity_control_M->Timing.tArray[0]);
  rtmSetTFinal(velocity_control_M, -1);
  velocity_control_M->Timing.stepSize0 = 0.01;

  /* External mode info */
  velocity_control_M->Sizes.checksums[0] = (227866837U);
  velocity_control_M->Sizes.checksums[1] = (1514004273U);
  velocity_control_M->Sizes.checksums[2] = (3039173455U);
  velocity_control_M->Sizes.checksums[3] = (3237608998U);

  {
    static const sysRanDType rtAlwaysEnabled = SUBSYS_RAN_BC_ENABLE;
    static RTWExtModeInfo rt_ExtModeInfo;
    static const sysRanDType *systemRan[7];
    velocity_control_M->extModeInfo = (&rt_ExtModeInfo);
    rteiSetSubSystemActiveVectorAddresses(&rt_ExtModeInfo, systemRan);
    systemRan[0] = &rtAlwaysEnabled;
    systemRan[1] = &rtAlwaysEnabled;
    systemRan[2] = &rtAlwaysEnabled;
    systemRan[3] = &rtAlwaysEnabled;
    systemRan[4] = &rtAlwaysEnabled;
    systemRan[5] = &rtAlwaysEnabled;
    systemRan[6] = &rtAlwaysEnabled;
    rteiSetModelMappingInfoPtr(velocity_control_M->extModeInfo,
      &velocity_control_M->SpecialInfo.mappingInfo);
    rteiSetChecksumsPtr(velocity_control_M->extModeInfo,
                        velocity_control_M->Sizes.checksums);
    rteiSetTPtr(velocity_control_M->extModeInfo, rtmGetTPtr(velocity_control_M));
  }

  {
    codertarget_arduinobase_inter_T *obj;

    /* Start for Probe: '<S55>/Probe' */
    velocity_control_B.Probe[0] = 0.01;
    velocity_control_B.Probe[1] = 0.0;

    /* InitializeConditions for DiscreteIntegrator: '<S59>/Integrator' */
    velocity_control_DW.Integrator_IC_LOADING = 1U;

    /* InitializeConditions for DiscreteIntegrator: '<S35>/Integrator' */
    velocity_control_DW.Integrator_DSTATE_c =
      velocity_control_P.DiscretePIDController_Initial_l;

    /* InitializeConditions for DiscreteIntegrator: '<S30>/Filter' */
    velocity_control_DW.Filter_DSTATE =
      velocity_control_P.DiscretePIDController_InitialCo;

    /* Start for MATLABSystem: '<S3>/M1V4 Middle Connector 2,3' */
    /*  Constructor */
    /*  Support name-value pair arguments when constructing the object. */
    velocity_control_DW.obj_j.matlabCodegenIsDeleted = false;

    /*  Do not generate code for sprintf */
    velocity_control_DW.obj_j.isInitialized = 1L;

    /*         %% Define output properties */
    /*    Call: void enc_init(int enc, int pinA, int pinB) */
    enc_init(0.0, 2.0, 3.0);

    /*                  if(obj.PWMTimer> 0) */
    /*                      coder.cinclude('PWMFSelect.h'); */
    /*                      coder.ceval('PWM_Select', obj.PWMFSelect, obj.PWMTimer); */
    /*                      disp('skipp!!!') */
    /*                  end */
    velocity_control_DW.obj_j.isSetupComplete = true;

    /* Start for MATLABSystem: '<S3>/Digital Output' */
    velocity_control_DW.obj.matlabCodegenIsDeleted = false;
    velocity_control_DW.obj.isInitialized = 1L;
    digitalIOSetup(8, 1);
    velocity_control_DW.obj.isSetupComplete = true;

    /* Start for MATLABSystem: '<S3>/PWM' */
    velocity_control_DW.obj_l.matlabCodegenIsDeleted = false;
    obj = &velocity_control_DW.obj_l;
    velocity_control_DW.obj_l.isInitialized = 1L;
    obj->PWMDriverObj.MW_PWM_HANDLE = MW_PWM_Open(9UL, 0.0, 0.0);
    velocity_control_DW.obj_l.isSetupComplete = true;
  }
}

/* Model terminate function */
void velocity_control_terminate(void)
{
  codertarget_arduinobase_inter_T *obj;

  /* Terminate for MATLABSystem: '<S3>/M1V4 Middle Connector 2,3' */
  if (!velocity_control_DW.obj_j.matlabCodegenIsDeleted) {
    velocity_control_DW.obj_j.matlabCodegenIsDeleted = true;
  }

  /* End of Terminate for MATLABSystem: '<S3>/M1V4 Middle Connector 2,3' */
  /* Terminate for MATLABSystem: '<S3>/Digital Output' */
  if (!velocity_control_DW.obj.matlabCodegenIsDeleted) {
    velocity_control_DW.obj.matlabCodegenIsDeleted = true;
  }

  /* End of Terminate for MATLABSystem: '<S3>/Digital Output' */

  /* Terminate for MATLABSystem: '<S3>/PWM' */
  obj = &velocity_control_DW.obj_l;
  if (!velocity_control_DW.obj_l.matlabCodegenIsDeleted) {
    velocity_control_DW.obj_l.matlabCodegenIsDeleted = true;
    if ((velocity_control_DW.obj_l.isInitialized == 1L) &&
        velocity_control_DW.obj_l.isSetupComplete) {
      obj->PWMDriverObj.MW_PWM_HANDLE = MW_PWM_GetHandle(9UL);
      MW_PWM_SetDutyCycle(velocity_control_DW.obj_l.PWMDriverObj.MW_PWM_HANDLE,
                          0.0);
      obj->PWMDriverObj.MW_PWM_HANDLE = MW_PWM_GetHandle(9UL);
      MW_PWM_Close(velocity_control_DW.obj_l.PWMDriverObj.MW_PWM_HANDLE);
    }
  }

  /* End of Terminate for MATLABSystem: '<S3>/PWM' */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
