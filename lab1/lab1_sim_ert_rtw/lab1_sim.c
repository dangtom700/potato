/*
 * Academic License - for use in teaching, academic research, and meeting
 * course requirements at degree granting institutions only.  Not for
 * government, commercial, or other organizational use.
 *
 * File: lab1_sim.c
 *
 * Code generated for Simulink model 'lab1_sim'.
 *
 * Model version                  : 1.1
 * Simulink Coder version         : 9.9 (R2023a) 19-Nov-2022
 * C/C++ source code generated on : Mon Jul 13 16:50:09 2026
 *
 * Target selection: ert.tlc
 * Embedded hardware selection: Atmel->AVR
 * Code generation objectives: Unspecified
 * Validation result: Not run
 */

#include "lab1_sim.h"
#include "lab1_sim_types.h"
#include "rtwtypes.h"
#include "lab1_sim_private.h"

/* Block signals (default storage) */
B_lab1_sim_T lab1_sim_B;

/* Block states (default storage) */
DW_lab1_sim_T lab1_sim_DW;

/* Real-time model */
static RT_MODEL_lab1_sim_T lab1_sim_M_;
RT_MODEL_lab1_sim_T *const lab1_sim_M = &lab1_sim_M_;

/* Model step function */
void lab1_sim_step(void)
{
  MW_AnalogIn_ResultDataType_Type datatype_id;
  codertarget_arduinobase_inter_T *obj;
  boolean_T rtb_Compare;

  /* MATLABSystem: '<Root>/Analog Input' */
  if (lab1_sim_DW.obj.SampleTime != lab1_sim_P.AnalogInput_SampleTime) {
    lab1_sim_DW.obj.SampleTime = lab1_sim_P.AnalogInput_SampleTime;
  }

  obj = &lab1_sim_DW.obj;
  obj->AnalogInDriverObj.MW_ANALOGIN_HANDLE = MW_AnalogIn_GetHandle(58UL);
  datatype_id = MW_ANALOGIN_UINT16;

  /* MATLABSystem: '<Root>/Analog Input' */
  MW_AnalogInSingle_ReadResult
    (lab1_sim_DW.obj.AnalogInDriverObj.MW_ANALOGIN_HANDLE,
     &lab1_sim_B.AnalogInput, datatype_id);

  /* RelationalOperator: '<S1>/Compare' incorporates:
   *  Constant: '<S1>/Constant'
   */
  rtb_Compare = (lab1_sim_B.AnalogInput >= lab1_sim_P.CompareToConstant_const);

  /* MATLABSystem: '<Root>/Digital Output' */
  writeDigitalPin(9, (uint8_T)rtb_Compare);

  /* Gain: '<Root>/Gain' */
  lab1_sim_B.Gain = (uint8_T)(rtb_Compare ? (int16_T)lab1_sim_P.Gain_Gain : 0);

  {                                    /* Sample time: [0.01s, 0.0s] */
  }

  /* Update absolute time for base rate */
  /* The "clockTick0" counts the number of times the code of this task has
   * been executed. The absolute time is the multiplication of "clockTick0"
   * and "Timing.stepSize0". Size of "clockTick0" ensures timer will not
   * overflow during the application lifespan selected.
   */
  lab1_sim_M->Timing.taskTime0 =
    ((time_T)(++lab1_sim_M->Timing.clockTick0)) * lab1_sim_M->Timing.stepSize0;
}

/* Model initialize function */
void lab1_sim_initialize(void)
{
  /* Registration code */
  rtmSetTFinal(lab1_sim_M, -1);
  lab1_sim_M->Timing.stepSize0 = 0.01;

  /* External mode info */
  lab1_sim_M->Sizes.checksums[0] = (921327104U);
  lab1_sim_M->Sizes.checksums[1] = (2709188611U);
  lab1_sim_M->Sizes.checksums[2] = (4001815662U);
  lab1_sim_M->Sizes.checksums[3] = (2266017215U);

  {
    static const sysRanDType rtAlwaysEnabled = SUBSYS_RAN_BC_ENABLE;
    static RTWExtModeInfo rt_ExtModeInfo;
    static const sysRanDType *systemRan[3];
    lab1_sim_M->extModeInfo = (&rt_ExtModeInfo);
    rteiSetSubSystemActiveVectorAddresses(&rt_ExtModeInfo, systemRan);
    systemRan[0] = &rtAlwaysEnabled;
    systemRan[1] = &rtAlwaysEnabled;
    systemRan[2] = &rtAlwaysEnabled;
    rteiSetModelMappingInfoPtr(lab1_sim_M->extModeInfo,
      &lab1_sim_M->SpecialInfo.mappingInfo);
    rteiSetChecksumsPtr(lab1_sim_M->extModeInfo, lab1_sim_M->Sizes.checksums);
    rteiSetTPtr(lab1_sim_M->extModeInfo, rtmGetTPtr(lab1_sim_M));
  }

  {
    codertarget_arduinobase_inter_T *obj;

    /* Start for MATLABSystem: '<Root>/Analog Input' */
    lab1_sim_DW.obj.matlabCodegenIsDeleted = false;
    lab1_sim_DW.obj.SampleTime = lab1_sim_P.AnalogInput_SampleTime;
    obj = &lab1_sim_DW.obj;
    lab1_sim_DW.obj.isInitialized = 1L;
    obj->AnalogInDriverObj.MW_ANALOGIN_HANDLE = MW_AnalogInSingle_Open(58UL);
    lab1_sim_DW.obj.isSetupComplete = true;

    /* Start for MATLABSystem: '<Root>/Digital Output' */
    lab1_sim_DW.obj_h.matlabCodegenIsDeleted = false;
    lab1_sim_DW.obj_h.isInitialized = 1L;
    digitalIOSetup(9, 1);
    lab1_sim_DW.obj_h.isSetupComplete = true;
  }
}

/* Model terminate function */
void lab1_sim_terminate(void)
{
  codertarget_arduinobase_inter_T *obj;

  /* Terminate for MATLABSystem: '<Root>/Analog Input' */
  obj = &lab1_sim_DW.obj;
  if (!lab1_sim_DW.obj.matlabCodegenIsDeleted) {
    lab1_sim_DW.obj.matlabCodegenIsDeleted = true;
    if ((lab1_sim_DW.obj.isInitialized == 1L) && lab1_sim_DW.obj.isSetupComplete)
    {
      obj->AnalogInDriverObj.MW_ANALOGIN_HANDLE = MW_AnalogIn_GetHandle(58UL);
      MW_AnalogIn_Close(lab1_sim_DW.obj.AnalogInDriverObj.MW_ANALOGIN_HANDLE);
    }
  }

  /* End of Terminate for MATLABSystem: '<Root>/Analog Input' */

  /* Terminate for MATLABSystem: '<Root>/Digital Output' */
  if (!lab1_sim_DW.obj_h.matlabCodegenIsDeleted) {
    lab1_sim_DW.obj_h.matlabCodegenIsDeleted = true;
  }

  /* End of Terminate for MATLABSystem: '<Root>/Digital Output' */
}

/*
 * File trailer for generated code.
 *
 * [EOF]
 */
