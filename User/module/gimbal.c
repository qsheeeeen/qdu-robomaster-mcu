/*
 * 云台模组
 */

/* Includes ----------------------------------------------------------------- */
#include "gimbal.h"

#include "bsp/mm.h"

/* Private typedef ---------------------------------------------------------- */
/* Private define ----------------------------------------------------------- */
/* Private macro ------------------------------------------------------------ */
/* Private variables -------------------------------------------------------- */
/* Private function  -------------------------------------------------------- */

/**
 * @brief 设置云台模式
 *
 * @param g 包含云台数据的结构体
 * @param mode 要设置的模式
 */
static void Gimbal_SetMode(Gimbal_t *g, Game_GimbalMode_t mode) {
  ASSERT(g);
  if (mode == g->mode) return;

  /* 切换模式后重置PID和滤波器 */
  for (size_t i = 0; i < GIMBAL_CTRL_NUM; i++) {
    PID_Reset(g->pid + i);
  }
  for (size_t i = 0; i < GIMBAL_ACTR_NUM; i++) {
    LowPassFilter2p_Reset(g->filter_out + i, 0.0f);
  }

  AHRS_ResetEulr(&(g->setpoint.eulr)); /* 切换模式后重置设定值 */
  if (g->mode == GIMBAL_MODE_RELAX) {
    if (mode == GIMBAL_MODE_ABSOLUTE) {
      g->setpoint.eulr.yaw = g->feedback.eulr.imu.yaw;
    } else if (mode == GIMBAL_MODE_RELATIVE) {
      g->setpoint.eulr.yaw = g->feedback.eulr.encoder.yaw;
    }
  }

  g->mode = mode;
}

/* Exported functions ------------------------------------------------------- */

/**
 * @brief 初始化云台
 *
 * @param g 包含云台数据的结构体
 * @param param 包含云台参数的结构体指针
 * @param target_freq 任务预期的运行频率
 */
void Gimbal_Init(Gimbal_t *g, const Gimbal_Params_t *param, float limit_max,
                 float target_freq) {
  ASSERT(g);
  ASSERT(param);

  g->param = param;            /* 初始化参数 */
  g->mode = GIMBAL_MODE_RELAX; /* 设置默认模式 */

  /* 设置软件限位 */
  if (g->param->reverse.pit) CircleReverse(&limit_max);
  g->limit.min = g->limit.max = limit_max;
  CircleAdd(&(g->limit.min), -g->param->pitch_travel_rad, M_2PI);

  /* 初始化云台电机控制PID和LPF */
  PID_Init(g->pid + GIMBAL_CTRL_YAW_ANGLE_IDX, KPID_MODE_NO_D, target_freq,
           g->param->pid + GIMBAL_CTRL_YAW_ANGLE_IDX);
  PID_Init(g->pid + GIMBAL_CTRL_YAW_OMEGA_IDX, KPID_MODE_CALC_D, target_freq,
           g->param->pid + GIMBAL_CTRL_YAW_OMEGA_IDX);

  PID_Init(g->pid + GIMBAL_CTRL_PIT_ANGLE_IDX, KPID_MODE_NO_D, target_freq,
           g->param->pid + GIMBAL_CTRL_PIT_ANGLE_IDX);
  PID_Init(g->pid + GIMBAL_CTRL_PIT_OMEGA_IDX, KPID_MODE_CALC_D, target_freq,
           g->param->pid + GIMBAL_CTRL_PIT_OMEGA_IDX);

  for (size_t i = 0; i < GIMBAL_ACTR_NUM; i++) {
    LowPassFilter2p_Init(g->filter_out + i, target_freq,
                         g->param->low_pass_cutoff_freq.out);
  }
}

/**
 * @brief 通过CAN设备更新云台反馈信息
 *
 * @param g 云台
 * @param can CAN设备
 */
void Gimbal_UpdateFeedback(Gimbal_t *g, const CAN_t *can) {
  ASSERT(g);
  ASSERT(can);

  g->feedback.eulr.encoder.yaw = can->motor.gimbal.named.yaw.rotor_abs_angle;
  g->feedback.eulr.encoder.pit = can->motor.gimbal.named.pit.rotor_abs_angle;

  if (g->param->reverse.yaw) CircleReverse(&(g->feedback.eulr.encoder.yaw));
  if (g->param->reverse.pit) CircleReverse(&(g->feedback.eulr.encoder.pit));
}

/**
 * @brief 运行云台控制逻辑
 *
 * @param g 包含云台数据的结构体
 * @param fb 云台反馈信息
 * @param g_cmd 云台控制指令
 * @param dt_sec 两次调用的时间间隔
 */
void Gimbal_Control(Gimbal_t *g, CMD_GimbalCmd_t *g_cmd, uint32_t now) {
  ASSERT(g);
  ASSERT(g_cmd);

  g->dt = (float)(now - g->lask_wakeup) / 1000.0f;
  g->lask_wakeup = now;

  Gimbal_SetMode(g, g_cmd->mode);

  /* yaw坐标正方向与遥控器操作逻辑相反 */
  g_cmd->delta_eulr.pit = g_cmd->delta_eulr.pit;
  g_cmd->delta_eulr.yaw = -g_cmd->delta_eulr.yaw;

  /* 处理yaw控制命令 */
  CircleAdd(&(g->setpoint.eulr.yaw), g_cmd->delta_eulr.yaw, M_2PI);

  /* 处理pitch控制命令，软件限位 */
  const float delta_max =
      CircleError(g->limit.max,
                  (g->feedback.eulr.encoder.pit + g->setpoint.eulr.pit -
                   g->feedback.eulr.imu.pit),
                  M_2PI);
  const float delta_min =
      CircleError(g->limit.min,
                  (g->feedback.eulr.encoder.pit + g->setpoint.eulr.pit -
                   g->feedback.eulr.imu.pit),
                  M_2PI);
  Clamp(&(g_cmd->delta_eulr.pit), delta_min, delta_max);
  g->setpoint.eulr.pit += g_cmd->delta_eulr.pit;

  /* 重置输入指令，防止重复处理 */
  AHRS_ResetEulr(&(g_cmd->delta_eulr));

  /* 控制相关逻辑 */
  float yaw_omega_set_point, pit_omega_set_point;
  switch (g->mode) {
    case GIMBAL_MODE_RELAX:
      for (size_t i = 0; i < GIMBAL_ACTR_NUM; i++) g->out[i] = 0.0f;
      break;

    case GIMBAL_MODE_ABSOLUTE:
      /* TODO: 可以试着在Pitch轴这里引入前馈（预测）
       * 通过实验计算得到保持在特定角度需要的电机输出值，补偿连杆损失
       * PID中的I专注于补偿摩擦力，数值可以相对较小
       * Yaw轴也可以引入前馈控制，主要受到载弹量影响，但实现太过复杂，不予考虑
       */

      /* Yaw轴角度 反馈控制 */
      yaw_omega_set_point =
          PID_Calc(g->pid + GIMBAL_CTRL_YAW_ANGLE_IDX, g->setpoint.eulr.yaw,
                   g->feedback.eulr.imu.yaw, 0.0f, g->dt);

      /* Yaw轴角速度 反馈控制 */
      g->out[GIMBAL_ACTR_YAW_IDX] =
          PID_Calc(g->pid + GIMBAL_CTRL_YAW_OMEGA_IDX, yaw_omega_set_point,
                   g->feedback.gyro.z, 0.f, g->dt);

      /* Pitch轴角度 反馈控制 */
      pit_omega_set_point =
          PID_Calc(g->pid + GIMBAL_CTRL_PIT_ANGLE_IDX, g->setpoint.eulr.pit,
                   g->feedback.eulr.imu.pit, 0.0f, g->dt);

      /* Pitch轴角速度 反馈控制 */
      g->out[GIMBAL_ACTR_PIT_IDX] =
          PID_Calc(g->pid + GIMBAL_CTRL_PIT_OMEGA_IDX, pit_omega_set_point,
                   g->feedback.gyro.x, 0.f, g->dt);
      break;

    case GIMBAL_MODE_RELATIVE:
      for (size_t i = 0; i < GIMBAL_ACTR_NUM; i++) g->out[i] = 0.0f;
      break;
  }

  /* 输出滤波 */
  for (size_t i = 0; i < GIMBAL_ACTR_NUM; i++)
    g->out[i] = LowPassFilter2p_Apply(g->filter_out + i, g->out[i]);

  /* 处理电机反装 */
  if (g->param->reverse.yaw)
    g->out[GIMBAL_ACTR_YAW_IDX] = -g->out[GIMBAL_ACTR_YAW_IDX];
  if (g->param->reverse.pit)
    g->out[GIMBAL_ACTR_PIT_IDX] = -g->out[GIMBAL_ACTR_PIT_IDX];
}

/**
 * @brief 复制云台输出值
 *
 * @param g 包含云台数据的结构体
 * @param out CAN设备云台输出结构体
 */
void Gimbal_PackOutput(Gimbal_t *g, CAN_GimbalOutput_t *out) {
  ASSERT(g);
  ASSERT(out);
  out->named.yaw = g->out[GIMBAL_ACTR_YAW_IDX];
  out->named.pit = g->out[GIMBAL_ACTR_PIT_IDX];
}

/**
 * @brief 清空输出值
 *
 * @param output 要清空的结构体
 */
void Gimbal_ResetOutput(CAN_GimbalOutput_t *output) {
  ASSERT(output);
  memset(output, 0, sizeof(*output));
}

/**
 * @brief 导出云台UI数据
 *
 * @param g 云台结构体
 * @param ui UI结构体
 */
void Gimbal_PackUi(const Gimbal_t *g, UI_GimbalUI_t *ui) {
  ASSERT(g);
  ASSERT(ui);
  ui->mode = g->mode;
}
