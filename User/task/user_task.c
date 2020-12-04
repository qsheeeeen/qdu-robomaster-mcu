/*
  保存任务属性，生成任务时使用。
*/

/* Includes ----------------------------------------------------------------- */
#include "task\user_task.h"

/* 机器人运行时的数据 */
Task_Runtime_t task_runtime;

/* 各个任务的参数 */
const osThreadAttr_t attr_init = {
    .name = "init",
    .priority = osPriorityLow,
    .stack_size = 256 * 4,
};

const osThreadAttr_t attr_atti_esti = {
    .name = "atti_esti",
    .priority = osPriorityRealtime,
    .stack_size = 256 * 4,
};

const osThreadAttr_t attr_cli = {
    .name = "cli",
    .priority = osPriorityLow,
    .stack_size = 256 * 4,
};

const osThreadAttr_t attr_command = {
    .name = "command",
    .priority = osPriorityHigh,
    .stack_size = 128 * 4,
};

const osThreadAttr_t attr_ctrl_chassis = {
    .name = "ctrl_chassis",
    .priority = osPriorityAboveNormal,
    .stack_size = 256 * 4,
};

const osThreadAttr_t attr_ctrl_gimbal = {
    .name = "ctrl_gimbal",
    .priority = osPriorityAboveNormal,
    .stack_size = 256 * 4,
};

const osThreadAttr_t attr_ctrl_shoot = {
    .name = "ctrl_shoot",
    .priority = osPriorityAboveNormal,
    .stack_size = 256 * 4,
};

const osThreadAttr_t attr_info = {
    .name = "info",
    .priority = osPriorityBelowNormal,
    .stack_size = 128 * 4,
};

const osThreadAttr_t attr_monitor = {
    .name = "monitor",
    .priority = osPriorityNormal,
    .stack_size = 128 * 4,
};

const osThreadAttr_t attr_motor = {
    .name = "motor",
    .priority = osPriorityRealtime,
    .stack_size = 128 * 4,
};

const osThreadAttr_t attr_referee = {
    .name = "referee",
    .priority = osPriorityNormal,
    .stack_size = 128 * 4,
};

const osThreadAttr_t attr_ai = {
    .name = "ai",
    .priority = osPriorityNormal,
    .stack_size = 128 * 4,
};

const osThreadAttr_t attr_rc = {
    .name = "rc",
    .priority = osPriorityNormal,
    .stack_size = 128 * 4,
};

const osThreadAttr_t attr_cap = {
    .name = "rc", //TODO: 名字没改
    .priority = osPriorityNormal,
    .stack_size = 128 * 4,
};
