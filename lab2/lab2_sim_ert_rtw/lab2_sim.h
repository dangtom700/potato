/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: lab2_sim.h
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

#ifndef RTW_HEADER_lab2_sim_h_
#define RTW_HEADER_lab2_sim_h_
#ifndef lab2_sim_COMMON_INCLUDES_
#define lab2_sim_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "rtw_extmode.h"
#include "sysran_types.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "encoder_arduino.h"
#endif                                 /* lab2_sim_COMMON_INCLUDES_ */

#include "lab2_sim_types.h"
#include "rt_nonfinite.h"
#include "MW_target_hardware_resources.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetFinalTime
#define rtmGetFinalTime(rtm)           ((rtm)->Timing.tFinal)
#endif

#ifndef rtmGetRTWExtModeInfo
#define rtmGetRTWExtModeInfo(rtm)      ((rtm)->extModeInfo)
#endif

#ifndef rtmGetErrorStatus
#define rtmGetErrorStatus(rtm)         ((rtm)->errorStatus)
#endif

#ifndef rtmSetErrorStatus
#define rtmSetErrorStatus(rtm, val)    ((rtm)->errorStatus = (val))
#endif

#ifndef rtmGetStopRequested
#define rtmGetStopRequested(rtm)       ((rtm)->Timing.stopRequestedFlag)
#endif

#ifndef rtmSetStopRequested
#define rtmSetStopRequested(rtm, val)  ((rtm)->Timing.stopRequestedFlag = (val))
#endif

#ifndef rtmGetStopRequestedPtr
#define rtmGetStopRequestedPtr(rtm)    (&((rtm)->Timing.stopRequestedFlag))
#endif

#ifndef rtmGetT
#define rtmGetT(rtm)                   ((rtm)->Timing.taskTime0)
#endif

#ifndef rtmGetTFinal
#define rtmGetTFinal(rtm)              ((rtm)->Timing.tFinal)
#endif

#ifndef rtmGetTPtr
#define rtmGetTPtr(rtm)                (&(rtm)->Timing.taskTime0)
#endif

/* Block signals (default storage) */
typedef struct {
  real_T Probe[2];                     /* '<S5>/Probe' */
  real_T AB;                           /* '<S2>/[A,B]' */
  real_T Diff;                         /* '<S1>/Diff' */
  real_T angle_rad_bounded;            /* '<Root>/MATLAB Function1' */
  real_T angle_deg_bounded;            /* '<Root>/MATLAB Function1' */
} B_lab2_sim_T;

/* Block states (default storage) for system '<Root>' */
typedef struct {
  Encoder_arduino_lab2_sim_T obj;      /* '<Root>/M1V4 Middle Connector 2,3' */
  real_T Integrator_DSTATE;            /* '<S9>/Integrator' */
  real_T UD_DSTATE;                    /* '<S1>/UD' */
  struct {
    void *LoggedData[2];
  } Scope1_PWORK;                      /* '<Root>/Scope1' */

  struct {
    void *LoggedData[2];
  } Scope2_PWORK;                      /* '<Root>/Scope2' */

  int8_T Integrator_PrevResetState;    /* '<S9>/Integrator' */
  uint8_T Integrator_IC_LOADING;       /* '<S9>/Integrator' */
  uint8_T is_active_c1_lab2_sim;       /* '<Root>/MATLAB Function1' */
  uint8_T is_active_c2_lab2_sim;       /* '<Root>/MATLAB Function' */
  boolean_T doneDoubleBufferReInit;    /* '<Root>/MATLAB Function1' */
  boolean_T doneDoubleBufferReInit_d;  /* '<Root>/MATLAB Function' */
} DW_lab2_sim_T;

/* Parameters (default storage) */
struct P_lab2_sim_T_ {
  real_T FilteredDerivativeDiscreteorCon;
                              /* Mask Parameter: FilteredDerivativeDiscreteorCon
                               * Referenced by: '<S2>/[A,B]'
                               */
  real_T FilteredDerivativeDiscreteorC_l;
                              /* Mask Parameter: FilteredDerivativeDiscreteorC_l
                               * Referenced by: '<S2>/[A,B]'
                               */
  real_T DiscreteDerivative_ICPrevScaled;
                              /* Mask Parameter: DiscreteDerivative_ICPrevScaled
                               * Referenced by: '<S1>/UD'
                               */
  real_T FilteredDerivativeDiscreteorC_i;
                              /* Mask Parameter: FilteredDerivativeDiscreteorC_i
                               * Referenced by: '<S2>/Gain'
                               */
  real_T FilteredDerivativeDiscreteorC_g;
                              /* Mask Parameter: FilteredDerivativeDiscreteorC_g
                               * Referenced by: '<S5>/Time constant'
                               */
  real_T FilteredDerivativeDiscreteorC_a;
                              /* Mask Parameter: FilteredDerivativeDiscreteorC_a
                               * Referenced by: '<S5>/Minimum sampling to time constant ratio'
                               */
  real_T Constant_Value;               /* Expression: 0
                                        * Referenced by: '<S2>/Constant'
                                        */
  real_T Integrator_gainval;           /* Computed Parameter: Integrator_gainval
                                        * Referenced by: '<S9>/Integrator'
                                        */
  real_T Integrator_UpperSat;          /* Expression: antiwindupUpperLimit
                                        * Referenced by: '<S9>/Integrator'
                                        */
  real_T Integrator_LowerSat;          /* Expression: antiwindupLowerLimit
                                        * Referenced by: '<S9>/Integrator'
                                        */
  real_T Saturation_UpperSat;          /* Expression: windupUpperLimit
                                        * Referenced by: '<S9>/Saturation'
                                        */
  real_T Saturation_LowerSat;          /* Expression: windupLowerLimit
                                        * Referenced by: '<S9>/Saturation'
                                        */
  real_T TSamp_WtEt;                   /* Computed Parameter: TSamp_WtEt
                                        * Referenced by: '<S1>/TSamp'
                                        */
};

/* Real-time Model Data Structure */
struct tag_RTM_lab2_sim_T {
  const char_T *errorStatus;
  RTWExtModeInfo *extModeInfo;

  /*
   * Sizes:
   * The following substructure contains sizes information
   * for many of the model attributes such as inputs, outputs,
   * dwork, sample times, etc.
   */
  struct {
    uint32_T checksums[4];
  } Sizes;

  /*
   * SpecialInfo:
   * The following substructure contains special information
   * related to other components that are dependent on RTW.
   */
  struct {
    const void *mappingInfo;
  } SpecialInfo;

  /*
   * Timing:
   * The following substructure contains information regarding
   * the timing information for the model.
   */
  struct {
    time_T taskTime0;
    uint32_T clockTick0;
    time_T stepSize0;
    time_T tFinal;
    boolean_T stopRequestedFlag;
  } Timing;
};

/* Block parameters (default storage) */
extern P_lab2_sim_T lab2_sim_P;

/* Block signals (default storage) */
extern B_lab2_sim_T lab2_sim_B;

/* Block states (default storage) */
extern DW_lab2_sim_T lab2_sim_DW;

/* Model entry point functions */
extern void lab2_sim_initialize(void);
extern void lab2_sim_step(void);
extern void lab2_sim_terminate(void);

/* Real-time Model object */
extern RT_MODEL_lab2_sim_T *const lab2_sim_M;
extern volatile boolean_T stopRequested;
extern volatile boolean_T runModel;

/*-
 * These blocks were eliminated from the model due to optimizations:
 *
 * Block '<S1>/Data Type Duplicate' : Unused code path elimination
 */

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Use the MATLAB hilite_system command to trace the generated code back
 * to the model.  For example,
 *
 * hilite_system('<S3>')    - opens system 3
 * hilite_system('<S3>/Kp') - opens and selects block Kp which resides in S3
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : 'lab2_sim'
 * '<S1>'   : 'lab2_sim/Discrete Derivative'
 * '<S2>'   : 'lab2_sim/Filtered Derivative (Discrete or Continuous)'
 * '<S3>'   : 'lab2_sim/MATLAB Function'
 * '<S4>'   : 'lab2_sim/MATLAB Function1'
 * '<S5>'   : 'lab2_sim/Filtered Derivative (Discrete or Continuous)/Enable//disable time constant'
 * '<S6>'   : 'lab2_sim/Filtered Derivative (Discrete or Continuous)/Initialization'
 * '<S7>'   : 'lab2_sim/Filtered Derivative (Discrete or Continuous)/Integrator (Discrete or Continuous)'
 * '<S8>'   : 'lab2_sim/Filtered Derivative (Discrete or Continuous)/Initialization/Init_u'
 * '<S9>'   : 'lab2_sim/Filtered Derivative (Discrete or Continuous)/Integrator (Discrete or Continuous)/Discrete'
 */
#endif                                 /* RTW_HEADER_lab2_sim_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
