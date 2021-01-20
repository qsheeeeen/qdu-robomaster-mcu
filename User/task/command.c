/*
  命令接收任务。

  接收机器人的控制指令。

  从DR16中接收数据，转换为通用的CMD_RC_t控制信号。
  根据CMD_RC_t计算最终命令CMD_t。
  把计算好的CMD_t细分后放到对应的消息队列中。
  超时未收到则认为是丢控等特殊情况，把CMD_RC_t中的内容置零，
  在后续的CMD_Parse中会根据此内容发现错误，保证机器人不失控。
*/

/* Includes ----------------------------------------------------------------- */
#include <string.h>

#include "device\dr16.h"
#include "task\user_task.h"

/* Private typedef ---------------------------------------------------------- */
/* Private define ----------------------------------------------------------- */
/* Private macro ------------------------------------------------------------ */
/* Private variables -------------------------------------------------------- */
#ifdef DEBUG
CMD_RC_t rc;
CMD_Host_t host;
CMD_t cmd;
#else
static CMD_RC_t rc;
static CMD_Host_t host;
static CMD_t cmd;
#endif

/* Private function --------------------------------------------------------- */
/* Exported functions ------------------------------------------------------- */

/**
 * \brief 控制指令接收
 *
 * \param argument 未使用
 */
void Task_Command(void *argument) {
  (void)argument; /* 未使用argument，消除警告 */

  /* 计算任务运行到指定频率，需要延时的时间 */
  const uint32_t delay_tick = osKernelGetTickFreq() / TASK_FREQ_CTRL_COMMAND;

  /* 初始化指令处理 */
  CMD_Init(&cmd, &(task_runtime.cfg.pilot_cfg->param.cmd));
  uint32_t tick = osKernelGetTickCount(); /* 控制任务运行频率的计时 */

  /* 用于计算遥控器数据频率 */
  while (1) {
#ifdef DEBUG
    /* 记录任务所使用的的栈空间 */
    task_runtime.stack_water_mark.command = osThreadGetStackSpace(NULL);
#endif
    tick += delay_tick; /* 计算下一个唤醒时刻 */

    osMessageQueueGet(task_runtime.msgq.cmd.raw.rc, &rc, 0, 0);
    osMessageQueueGet(task_runtime.msgq.cmd.raw.host, &host, 0, 0);

    osKernelLock(); /* 锁住RTOS内核防止控制过程中断，造成错误 */

    /* 将接收机数据解析为指令数据 */
    CMD_ParseRc(&rc, &cmd, 1.0f / (float)TASK_FREQ_CTRL_COMMAND);

    /* 判断是否需要让上位机覆写指令 */
    if (CMD_CHECK_HOST_OVERWRITE(&cmd))

      CMD_ParseHost(&host, &cmd, 1.0f / (float)TASK_FREQ_CTRL_COMMAND);

    osKernelUnlock(); /* 锁住RTOS内核防止控制过程中断，造成错误 */

    /* 将需要与其他任务分享的数据放到消息队列中 */
    osMessageQueueReset(task_runtime.msgq.cmd.ai);
    osMessageQueuePut(task_runtime.msgq.cmd.ai, &(cmd.ai_status), 0, 0);
    osMessageQueuePut(task_runtime.msgq.cmd.chassis, &(cmd.chassis), 0, 0);
    osMessageQueuePut(task_runtime.msgq.cmd.gimbal, &(cmd.gimbal), 0, 0);
    osMessageQueuePut(task_runtime.msgq.cmd.shoot, &(cmd.shoot), 0, 0);

    osDelayUntil(tick); /* 运行结束，等待下一次唤醒 */
  }
}
