/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: position_control.h
 *
 * Code generated for Simulink model 'position_control'.
 *
 * Model version                  : 1.3
 * Simulink Coder version         : 9.9 (R2023a) 19-Nov-2022
 * C/C++ source code generated on : Fri Jul 17 11:20:43 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Atmel->AVR
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#ifndef RTW_HEADER_position_control_h_
#define RTW_HEADER_position_control_h_
#ifndef position_control_COMMON_INCLUDES_
#define position_control_COMMON_INCLUDES_
#include "rtwtypes.h"
#include "rtw_extmode.h"
#include "sysran_types.h"
#include "rtw_continuous.h"
#include "rtw_solver.h"
#include "ext_mode.h"
#include "MW_arduino_digitalio.h"
#include "encoder_arduino.h"
#include "MW_PWM.h"
#endif                                 /* position_control_COMMON_INCLUDES_ */

#include "position_control_types.h"
#include "rtGetNaN.h"
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
#define rtmGetT(rtm)                   (rtmGetTPtr((rtm))[0])
#endif

#ifndef rtmGetTFinal
#define rtmGetTFinal(rtm)              ((rtm)->Timing.tFinal)
#endif

#ifndef rtmGetTPtr
#define rtmGetTPtr(rtm)                ((rtm)->Timing.t)
#endif

/* Block signals (default storage) */
typedef struct {
  real_T Probe[2];                     /* '<S55>/Probe' */
  real_T uT;                           /* '<S53>/1//T' */
  real_T AB;                           /* '<S53>/[A,B]' */
  real_T Mod;                          /* '<S3>/Mod' */
  real_T SliderGain;                   /* '<S2>/Slider Gain' */
  real_T DiscreteTimeIntegrator;       /* '<Root>/Discrete-Time Integrator' */
} B_position_control_T;

/* Block states (default storage) for system '<Root>' */
typedef struct {
  codertarget_arduinobase_block_T obj; /* '<S3>/Digital Output' */
  Encoder_arduino_position_cont_T obj_j;/* '<S3>/M1V4 Middle Connector 2,3' */
  codertarget_arduinobase_inter_T obj_l;/* '<S3>/PWM' */
  real_T Integrator_DSTATE;            /* '<S59>/Integrator' */
  real_T DiscreteTimeIntegrator_DSTATE;/* '<Root>/Discrete-Time Integrator' */
  real_T Filter_DSTATE;                /* '<S30>/Filter' */
  real_T Integrator_DSTATE_c;          /* '<S35>/Integrator' */
  struct {
    void *LoggedData[4];
  } Scope_PWORK;                       /* '<Root>/Scope' */

  int8_T Integrator_PrevResetState;    /* '<S59>/Integrator' */
  uint8_T Integrator_IC_LOADING;       /* '<S59>/Integrator' */
  uint8_T is_active_c2_position_control;/* '<S3>/MATLAB Function' */
  boolean_T doneDoubleBufferReInit;    /* '<S3>/MATLAB Function' */
} DW_position_control_T;

/* Parameters (default storage) */
struct P_position_control_T_ {
  real_T FilteredDerivativeDiscreteorCon;
                              /* Mask Parameter: FilteredDerivativeDiscreteorCon
                               * Referenced by: '<S53>/[A,B]'
                               */
  real_T FilteredDerivativeDiscreteorC_p;
                              /* Mask Parameter: FilteredDerivativeDiscreteorC_p
                               * Referenced by: '<S53>/[A,B]'
                               */
  real_T DiscretePIDController_D;     /* Mask Parameter: DiscretePIDController_D
                                       * Referenced by: '<S29>/Derivative Gain'
                                       */
  real_T DiscretePIDController_I;     /* Mask Parameter: DiscretePIDController_I
                                       * Referenced by: '<S32>/Integral Gain'
                                       */
  real_T DiscretePIDController_InitialCo;
                              /* Mask Parameter: DiscretePIDController_InitialCo
                               * Referenced by: '<S30>/Filter'
                               */
  real_T DiscretePIDController_Initial_l;
                              /* Mask Parameter: DiscretePIDController_Initial_l
                               * Referenced by: '<S35>/Integrator'
                               */
  real_T FilteredDerivativeDiscreteorC_i;
                              /* Mask Parameter: FilteredDerivativeDiscreteorC_i
                               * Referenced by: '<S53>/Gain'
                               */
  real_T DiscretePIDController_N;     /* Mask Parameter: DiscretePIDController_N
                                       * Referenced by: '<S38>/Filter Coefficient'
                                       */
  real_T DiscretePIDController_P;     /* Mask Parameter: DiscretePIDController_P
                                       * Referenced by: '<S40>/Proportional Gain'
                                       */
  real_T FilteredDerivativeDiscreteorC_a;
                              /* Mask Parameter: FilteredDerivativeDiscreteorC_a
                               * Referenced by: '<S55>/Time constant'
                               */
  real_T SliderGain_gain;              /* Mask Parameter: SliderGain_gain
                                        * Referenced by: '<S2>/Slider Gain'
                                        */
  real_T FilteredDerivativeDiscreteorC_m;
                              /* Mask Parameter: FilteredDerivativeDiscreteorC_m
                               * Referenced by: '<S55>/Minimum sampling to time constant ratio'
                               */
  real_T Constant_Value;               /* Expression: 0
                                        * Referenced by: '<S52>/Constant'
                                        */
  real_T Constant_Value_h;             /* Expression: 0
                                        * Referenced by: '<S53>/Constant'
                                        */
  real_T Integrator_gainval;           /* Computed Parameter: Integrator_gainval
                                        * Referenced by: '<S59>/Integrator'
                                        */
  real_T Integrator_UpperSat;          /* Expression: antiwindupUpperLimit
                                        * Referenced by: '<S59>/Integrator'
                                        */
  real_T Integrator_LowerSat;          /* Expression: antiwindupLowerLimit
                                        * Referenced by: '<S59>/Integrator'
                                        */
  real_T Saturation_UpperSat;          /* Expression: windupUpperLimit
                                        * Referenced by: '<S59>/Saturation'
                                        */
  real_T Saturation_LowerSat;          /* Expression: windupLowerLimit
                                        * Referenced by: '<S59>/Saturation'
                                        */
  real_T Constant1_Value;              /* Expression: pi
                                        * Referenced by: '<S3>/Constant1'
                                        */
  real_T SignalGenerator_Amplitude;    /* Expression: 1
                                        * Referenced by: '<Root>/Signal Generator'
                                        */
  real_T SignalGenerator_Frequency;    /* Expression: 1
                                        * Referenced by: '<Root>/Signal Generator'
                                        */
  real_T Constant_Value_c;             /* Expression: 1
                                        * Referenced by: '<Root>/Constant'
                                        */
  real_T SquareWaveGenerator_Amplitude;/* Expression: 1
                                        * Referenced by: '<Root>/Square Wave Generator'
                                        */
  real_T SquareWaveGenerator_Frequency;
                            /* Computed Parameter: SquareWaveGenerator_Frequency
                             * Referenced by: '<Root>/Square Wave Generator'
                             */
  real_T SawtoothGenerator_Amplitude;  /* Expression: 1
                                        * Referenced by: '<Root>/Sawtooth Generator'
                                        */
  real_T SawtoothGenerator_Frequency;
                              /* Computed Parameter: SawtoothGenerator_Frequency
                               * Referenced by: '<Root>/Sawtooth Generator'
                               */
  real_T DiscreteTimeIntegrator_gainval;
                           /* Computed Parameter: DiscreteTimeIntegrator_gainval
                            * Referenced by: '<Root>/Discrete-Time Integrator'
                            */
  real_T DiscreteTimeIntegrator_IC;    /* Expression: 0
                                        * Referenced by: '<Root>/Discrete-Time Integrator'
                                        */
  real_T Filter_gainval;               /* Computed Parameter: Filter_gainval
                                        * Referenced by: '<S30>/Filter'
                                        */
  real_T Integrator_gainval_l;       /* Computed Parameter: Integrator_gainval_l
                                      * Referenced by: '<S35>/Integrator'
                                      */
  real_T Saturation_UpperSat_j;        /* Expression: 6
                                        * Referenced by: '<S3>/Saturation'
                                        */
  real_T Saturation_LowerSat_h;        /* Expression: -6
                                        * Referenced by: '<S3>/Saturation'
                                        */
  real_T Gain1_Gain;                   /* Expression: 255/12
                                        * Referenced by: '<S3>/Gain1'
                                        */
  uint8_T ManualSwitch_CurrentSetting;
                              /* Computed Parameter: ManualSwitch_CurrentSetting
                               * Referenced by: '<Root>/Manual Switch'
                               */
  uint8_T ManualSwitch1_CurrentSetting;
                             /* Computed Parameter: ManualSwitch1_CurrentSetting
                              * Referenced by: '<Root>/Manual Switch1'
                              */
  uint8_T ManualSwitch2_CurrentSetting;
                             /* Computed Parameter: ManualSwitch2_CurrentSetting
                              * Referenced by: '<Root>/Manual Switch2'
                              */
};

/* Real-time Model Data Structure */
struct tag_RTM_position_control_T {
  const char_T *errorStatus;
  RTWExtModeInfo *extModeInfo;
  RTWSolverInfo solverInfo;

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
    uint32_T clockTick0;
    time_T stepSize0;
    uint32_T clockTick1;
    time_T tFinal;
    SimTimeStep simTimeStep;
    boolean_T stopRequestedFlag;
    time_T *t;
    time_T tArray[2];
  } Timing;
};

/* Block parameters (default storage) */
extern P_position_control_T position_control_P;

/* Block signals (default storage) */
extern B_position_control_T position_control_B;

/* Block states (default storage) */
extern DW_position_control_T position_control_DW;

/* Model entry point functions */
extern void position_control_initialize(void);
extern void position_control_step(void);
extern void position_control_terminate(void);

/* Real-time Model object */
extern RT_MODEL_position_control_T *const position_control_M;
extern volatile boolean_T stopRequested;
extern volatile boolean_T runModel;

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
 * '<Root>' : 'position_control'
 * '<S1>'   : 'position_control/Discrete PID Controller'
 * '<S2>'   : 'position_control/Slider Gain'
 * '<S3>'   : 'position_control/Subsystem'
 * '<S4>'   : 'position_control/Discrete PID Controller/Anti-windup'
 * '<S5>'   : 'position_control/Discrete PID Controller/D Gain'
 * '<S6>'   : 'position_control/Discrete PID Controller/Filter'
 * '<S7>'   : 'position_control/Discrete PID Controller/Filter ICs'
 * '<S8>'   : 'position_control/Discrete PID Controller/I Gain'
 * '<S9>'   : 'position_control/Discrete PID Controller/Ideal P Gain'
 * '<S10>'  : 'position_control/Discrete PID Controller/Ideal P Gain Fdbk'
 * '<S11>'  : 'position_control/Discrete PID Controller/Integrator'
 * '<S12>'  : 'position_control/Discrete PID Controller/Integrator ICs'
 * '<S13>'  : 'position_control/Discrete PID Controller/N Copy'
 * '<S14>'  : 'position_control/Discrete PID Controller/N Gain'
 * '<S15>'  : 'position_control/Discrete PID Controller/P Copy'
 * '<S16>'  : 'position_control/Discrete PID Controller/Parallel P Gain'
 * '<S17>'  : 'position_control/Discrete PID Controller/Reset Signal'
 * '<S18>'  : 'position_control/Discrete PID Controller/Saturation'
 * '<S19>'  : 'position_control/Discrete PID Controller/Saturation Fdbk'
 * '<S20>'  : 'position_control/Discrete PID Controller/Sum'
 * '<S21>'  : 'position_control/Discrete PID Controller/Sum Fdbk'
 * '<S22>'  : 'position_control/Discrete PID Controller/Tracking Mode'
 * '<S23>'  : 'position_control/Discrete PID Controller/Tracking Mode Sum'
 * '<S24>'  : 'position_control/Discrete PID Controller/Tsamp - Integral'
 * '<S25>'  : 'position_control/Discrete PID Controller/Tsamp - Ngain'
 * '<S26>'  : 'position_control/Discrete PID Controller/postSat Signal'
 * '<S27>'  : 'position_control/Discrete PID Controller/preSat Signal'
 * '<S28>'  : 'position_control/Discrete PID Controller/Anti-windup/Passthrough'
 * '<S29>'  : 'position_control/Discrete PID Controller/D Gain/Internal Parameters'
 * '<S30>'  : 'position_control/Discrete PID Controller/Filter/Disc. Forward Euler Filter'
 * '<S31>'  : 'position_control/Discrete PID Controller/Filter ICs/Internal IC - Filter'
 * '<S32>'  : 'position_control/Discrete PID Controller/I Gain/Internal Parameters'
 * '<S33>'  : 'position_control/Discrete PID Controller/Ideal P Gain/Passthrough'
 * '<S34>'  : 'position_control/Discrete PID Controller/Ideal P Gain Fdbk/Disabled'
 * '<S35>'  : 'position_control/Discrete PID Controller/Integrator/Discrete'
 * '<S36>'  : 'position_control/Discrete PID Controller/Integrator ICs/Internal IC'
 * '<S37>'  : 'position_control/Discrete PID Controller/N Copy/Disabled'
 * '<S38>'  : 'position_control/Discrete PID Controller/N Gain/Internal Parameters'
 * '<S39>'  : 'position_control/Discrete PID Controller/P Copy/Disabled'
 * '<S40>'  : 'position_control/Discrete PID Controller/Parallel P Gain/Internal Parameters'
 * '<S41>'  : 'position_control/Discrete PID Controller/Reset Signal/Disabled'
 * '<S42>'  : 'position_control/Discrete PID Controller/Saturation/Passthrough'
 * '<S43>'  : 'position_control/Discrete PID Controller/Saturation Fdbk/Disabled'
 * '<S44>'  : 'position_control/Discrete PID Controller/Sum/Sum_PID'
 * '<S45>'  : 'position_control/Discrete PID Controller/Sum Fdbk/Disabled'
 * '<S46>'  : 'position_control/Discrete PID Controller/Tracking Mode/Disabled'
 * '<S47>'  : 'position_control/Discrete PID Controller/Tracking Mode Sum/Passthrough'
 * '<S48>'  : 'position_control/Discrete PID Controller/Tsamp - Integral/TsSignalSpecification'
 * '<S49>'  : 'position_control/Discrete PID Controller/Tsamp - Ngain/Passthrough'
 * '<S50>'  : 'position_control/Discrete PID Controller/postSat Signal/Forward_Path'
 * '<S51>'  : 'position_control/Discrete PID Controller/preSat Signal/Forward_Path'
 * '<S52>'  : 'position_control/Subsystem/Compare To Zero'
 * '<S53>'  : 'position_control/Subsystem/Filtered Derivative (Discrete or Continuous)'
 * '<S54>'  : 'position_control/Subsystem/MATLAB Function'
 * '<S55>'  : 'position_control/Subsystem/Filtered Derivative (Discrete or Continuous)/Enable//disable time constant'
 * '<S56>'  : 'position_control/Subsystem/Filtered Derivative (Discrete or Continuous)/Initialization'
 * '<S57>'  : 'position_control/Subsystem/Filtered Derivative (Discrete or Continuous)/Integrator (Discrete or Continuous)'
 * '<S58>'  : 'position_control/Subsystem/Filtered Derivative (Discrete or Continuous)/Initialization/Init_u'
 * '<S59>'  : 'position_control/Subsystem/Filtered Derivative (Discrete or Continuous)/Integrator (Discrete or Continuous)/Discrete'
 */
#endif                                 /* RTW_HEADER_position_control_h_ */

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
