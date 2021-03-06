/**
 * @file init.c
 * @author Qu Shen (503578404@qq.com)
 * @brief 初始化任务
 * @version 1.0.0
 * @date 2021-04-15
 *
 * @copyright Copyright (c) 2021
 *
 * 根据机器人的FLASH配置里的型号，决定生成哪些任务。
 * 初始化所有消息队列
 *
 */

/* Includes ----------------------------------------------------------------- */
#include "bsp/flash.h"
#include "bsp/usb.h"
#include "component/cmd.h"
#include "device/ai.h"
#include "device/bmi088.h"
#include "device/can.h"
#include "device/ist8310.h"
#include "device/referee.h"
#include "module/cap.h"
#include "module/chassis.h"
#include "module/gimbal.h"
#include "module/launcher.h"
#include "task/user_task.h"

/* Private typedef ---------------------------------------------------------- */
/* Private define ----------------------------------------------------------- */
/* Private macro ------------------------------------------------------------ */
/* Private variables -------------------------------------------------------- */
/* Private function --------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

/**
 * @brief 初始化
 *
 * @param argument 未使用
 */
void Task_Init(void *argument) {
  UNUSED(argument); /* 未使用argument，消除警告 */

  Config_Get(&runtime.cfg); /* 获取机器人配置 */

  osKernelLock();
  /* 创建任务 */
  runtime.thread.atti_esti = osThreadNew(Task_AttiEsti, NULL, &attr_atti_esti);
  runtime.thread.cli = osThreadNew(Task_CLI, NULL, &attr_cli);
  runtime.thread.cmd = osThreadNew(Task_Cmd, NULL, &attr_cmd);
  runtime.thread.ctrl_cap = osThreadNew(Task_CtrlCap, NULL, &attr_ctrl_cap);
  runtime.thread.ctrl_chassis =
      osThreadNew(Task_CtrlChassis, NULL, &attr_ctrl_chassis);
  runtime.thread.ctrl_gimbal =
      osThreadNew(Task_CtrlGimbal, NULL, &attr_ctrl_gimbal);
  runtime.thread.ctrl_launcher =
      osThreadNew(Task_CtrlLauncher, NULL, &attr_ctrl_launcher);
  runtime.thread.info = osThreadNew(Task_Info, NULL, &attr_info);
  runtime.thread.monitor = osThreadNew(Task_Monitor, NULL, &attr_monitor);
  runtime.thread.can = osThreadNew(Task_Can, NULL, &attr_can);
  runtime.thread.referee = osThreadNew(Task_Referee, NULL, &attr_referee);
  runtime.thread.ai = osThreadNew(Task_Ai, NULL, &attr_ai);
  runtime.thread.rc = osThreadNew(Task_RC, NULL, &attr_rc);

  /* 创建消息队列 */
  /* motor */
  runtime.msgq.can.feedback.chassis =
      osMessageQueueNew(2u, sizeof(CAN_t), NULL);
  runtime.msgq.can.feedback.gimbal = osMessageQueueNew(2u, sizeof(CAN_t), NULL);
  runtime.msgq.can.feedback.launcher =
      osMessageQueueNew(2u, sizeof(CAN_t), NULL);
  runtime.msgq.can.feedback.cap = osMessageQueueNew(2u, sizeof(CAN_t), NULL);
  runtime.msgq.can.output.chassis =
      osMessageQueueNew(2u, sizeof(CAN_ChassisOutput_t), NULL);
  runtime.msgq.can.output.gimbal =
      osMessageQueueNew(2u, sizeof(CAN_GimbalOutput_t), NULL);
  runtime.msgq.can.output.launcher =
      osMessageQueueNew(2u, sizeof(CAN_LauncherOutput_t), NULL);
  runtime.msgq.can.output.cap =
      osMessageQueueNew(2u, sizeof(CAN_CapOutput_t), NULL);

  /* cmd */
  runtime.msgq.cmd.chassis =
      osMessageQueueNew(3u, sizeof(CMD_ChassisCmd_t), NULL);
  runtime.msgq.cmd.gimbal =
      osMessageQueueNew(3u, sizeof(CMD_GimbalCmd_t), NULL);
  runtime.msgq.cmd.launcher =
      osMessageQueueNew(3u, sizeof(CMD_LauncherCmd_t), NULL);
  runtime.msgq.cmd.ai = osMessageQueueNew(3u, sizeof(Game_AI_Status_t), NULL);

  /* atti_esti */
  runtime.msgq.cmd.src.rc = osMessageQueueNew(3u, sizeof(CMD_RC_t), NULL);
  runtime.msgq.cmd.src.host = osMessageQueueNew(3u, sizeof(CMD_Host_t), NULL);

  runtime.msgq.gimbal.accl = osMessageQueueNew(2u, sizeof(Vector3_t), NULL);
  runtime.msgq.gimbal.eulr_imu =
      osMessageQueueNew(2u, sizeof(AHRS_Eulr_t), NULL);
  runtime.msgq.gimbal.gyro = osMessageQueueNew(2u, sizeof(Vector3_t), NULL);

  runtime.msgq.cap_info = osMessageQueueNew(2u, sizeof(Cap_t), NULL);

  /* AI */
  runtime.msgq.ai.quat = osMessageQueueNew(2u, sizeof(AHRS_Quaternion_t), NULL);

  /* 裁判系统 */
  runtime.msgq.referee.ai =
      osMessageQueueNew(2u, sizeof(Referee_ForAI_t), NULL);
  runtime.msgq.referee.chassis =
      osMessageQueueNew(2u, sizeof(Referee_ForChassis_t), NULL);
  runtime.msgq.referee.cap =
      osMessageQueueNew(2u, sizeof(Referee_ForCap_t), NULL);
  runtime.msgq.referee.launcher =
      osMessageQueueNew(2u, sizeof(Referee_ForLauncher_t), NULL);

  /* UI */
  runtime.msgq.ui.chassis = osMessageQueueNew(2u, sizeof(UI_ChassisUI_t), NULL);
  runtime.msgq.ui.cap = osMessageQueueNew(2u, sizeof(UI_CapUI_t), NULL);
  runtime.msgq.ui.gimbal = osMessageQueueNew(2u, sizeof(UI_GimbalUI_t), NULL);
  runtime.msgq.ui.launcher =
      osMessageQueueNew(2u, sizeof(UI_LauncherUI_t), NULL);
  runtime.msgq.ui.cmd = osMessageQueueNew(2u, sizeof(CMD_UI_t), NULL);
  runtime.msgq.ui.ai = osMessageQueueNew(2u, sizeof(AI_UI_t), NULL);

  osKernelUnlock();
  osThreadTerminate(osThreadGetId()); /* 结束自身 */
}
