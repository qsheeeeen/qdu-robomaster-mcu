/*
  CAN总线上的设
  将所有CAN总线上挂载的设抽象成一设进行配和控制
*/

/* Includes ----------------------------------------------------------------- */
#include "can.h"

#include <stdbool.h>
#include <string.h>

#include "bsp\can.h"
#include "bsp\mm.h"
#include "component\user_math.h"
#include "device\referee.h"

/* Private define ----------------------------------------------------------- */
/* Motor */
/* id		feedback id		control id */
/* 1-4		0x205 to 0x208  0x1ff */
/* 5-6		0x209 to 0x20B  0x2ff */
#define CAN_GM6020_FB_ID_BASE (0x205)
#define CAN_GM6020_CTRL_ID_BASE (0x1ff)
#define CAN_GM6020_CTRL_ID_EXTAND (0x2ff)

/* id		feedback id		control id */
/* 1-4		0x201 to 0x204  0x200 */
/* 5-6		0x205 to 0x208  0x1ff */
#define CAN_M3508_M2006_FB_ID_BASE (0x201)
#define CAN_M3508_M2006_CTRL_ID_BASE (0x200)
#define CAN_M3508_M2006_CTRL_ID_EXTAND (0x1ff)
#define CAN_M3508_M2006_ID_SETTING_ID (0x700)

/* 电机最大控制输出绝对值 */
#define CAN_GM6020_MAX_ABS_LSB (30000)
#define CAN_M3508_MAX_ABS_LSB (16384)
#define CAN_M2006_MAX_ABS_LSB (10000)

/* 电机最大电流绝对值 */
#define CAN_GM6020_MAX_ABS_CUR (1)
#define CAN_M3508_MAX_ABS_CUR (20)
#define CAN_M2006_MAX_ABS_CUR (10)

#define CAN_MOTOR_TX_BUF_SIZE (8)
#define CAN_MOTOR_RX_BUF_SIZE (8)

#define CAN_MOTOR_ENC_RES (8192)  /* 电机编码器分辨率 */
#define CAN_MOTOR_CUR_RES (16384) /* 电机转矩电流分辨率 */

#define CAN_MOTOR_RX_FIFO CAN_RX_FIFO0

/* Super capacitor */
#define CAN_CAP_FB_ID_BASE (0x000)
#define CAN_CAP_CTRL_ID_BASE (0x000)

#define CAN_CAP_RES (100) /* 电容数据分辨率 */

#define CAN_CAP_RX_FIFO CAN_RX_FIFO1

/* Private macro ------------------------------------------------------------ */
/* Private typedef ---------------------------------------------------------- */

/* Private variables -------------------------------------------------------- */
static CAN_RawRx_t raw_rx1;
static CAN_RawRx_t raw_rx2;
static CAN_RawTx_t raw_tx1;
static CAN_RawTx_t raw_tx2;

static osThreadId_t thread_alert;
static CAN_t *gcan;
static bool inited = false;

/* Private function  -------------------------------------------------------- */
static void CAN_Motor_Parse(CAN_MotorFeedback_t *feedback, const uint8_t *raw) {
  uint16_t raw_angle = (uint16_t)((raw[0] << 8) | raw[1]);
  uint16_t raw_current = (int16_t)((raw[4] << 8) | raw[5]);

  feedback->rotor_angle = raw_angle / (float)CAN_MOTOR_ENC_RES * M_2PI;
  feedback->rotor_speed = (int16_t)((raw[2] << 8) | raw[3]);
  feedback->torque_current =
      raw_current * CAN_M3508_MAX_ABS_CUR / (float)CAN_MOTOR_CUR_RES;
  feedback->temp = raw[6];
}

void CAN_Cap_Decode(CAN_CapFeedback_t *feedback, const uint8_t *raw) {
  feedback->input_volt = (float)((raw[1] << 8) | raw[0]) / (float)CAN_CAP_RES;
  feedback->cap_volt = (float)((raw[3] << 8) | raw[2]) / (float)CAN_CAP_RES;
  feedback->input_volt = (float)((raw[5] << 8) | raw[4]) / (float)CAN_CAP_RES;
  feedback->input_volt = (float)((raw[7] << 8) | raw[6]) / (float)CAN_CAP_RES;
}

static void CAN_CAN1RxFifoMsgPendingCallback(void) {
  HAL_CAN_GetRxMessage(BSP_CAN_GetHandle(BSP_CAN_1), CAN_MOTOR_RX_FIFO,
                       &raw_rx1.rx_header, raw_rx1.rx_data);

  osMessageQueuePut(gcan->msgq_raw_motor, &raw_rx1, 0, 0);
}

static void CAN_CAN2RxFifoMsgPendingCallback(void) {
  HAL_CAN_GetRxMessage(BSP_CAN_GetHandle(BSP_CAN_2), CAN_CAP_RX_FIFO,
                       &raw_rx2.rx_header, raw_rx2.rx_data);
  if (raw_rx2.rx_header.StdId == CAN_CAP_FB_ID_BASE) {
    /* TODO: 添加cap接收msgq。然后放进去 */
    osMessageQueuePut(gcan->msgq_raw_cap, &raw_rx2, 0, 0);
  } else {
    osMessageQueuePut(gcan->msgq_raw_motor, &raw_rx2, 0, 0);
  }
}

/* Exported functions ------------------------------------------------------- */
int8_t CAN_Init(CAN_t *can) {
  if (can == NULL) return DEVICE_ERR_NULL;
  if (inited) return DEVICE_ERR_INITED;
  if ((thread_alert = osThreadGetId()) == NULL) return DEVICE_ERR_NULL;

  /* 初始化接收原始CAN消息的队列，要在中断开启前初始化 */
  can->msgq_raw_motor = osMessageQueueNew(32, sizeof(CAN_RawRx_t), NULL);
  can->msgq_raw_cap = osMessageQueueNew(32, sizeof(CAN_RawRx_t), NULL);

  CAN_FilterTypeDef can_filter = {0};

  can_filter.FilterBank = 0;
  can_filter.FilterIdHigh = 0;
  can_filter.FilterIdLow = 0;
  can_filter.FilterMode = CAN_FILTERMODE_IDMASK;
  can_filter.FilterScale = CAN_FILTERSCALE_32BIT;
  can_filter.FilterMaskIdHigh = 0;
  can_filter.FilterMaskIdLow = 0;
  can_filter.FilterActivation = ENABLE;
  can_filter.SlaveStartFilterBank = 14;
  can_filter.FilterFIFOAssignment = CAN_MOTOR_RX_FIFO;

  HAL_CAN_ConfigFilter(BSP_CAN_GetHandle(BSP_CAN_1), &can_filter);
  HAL_CAN_Start(BSP_CAN_GetHandle(BSP_CAN_1));
  BSP_CAN_RegisterCallback(BSP_CAN_1, HAL_CAN_RX_FIFO0_MSG_PENDING_CB,
                           CAN_CAN1RxFifoMsgPendingCallback);
  HAL_CAN_ActivateNotification(BSP_CAN_GetHandle(BSP_CAN_1),
                               CAN_IT_RX_FIFO0_MSG_PENDING);

  can_filter.FilterBank = 14;
  can_filter.FilterFIFOAssignment = CAN_CAP_RX_FIFO;

  HAL_CAN_ConfigFilter(BSP_CAN_GetHandle(BSP_CAN_2), &can_filter);
  HAL_CAN_Start(BSP_CAN_GetHandle(BSP_CAN_2));
  BSP_CAN_RegisterCallback(BSP_CAN_2, HAL_CAN_RX_FIFO1_MSG_PENDING_CB,
                           CAN_CAN2RxFifoMsgPendingCallback);
  HAL_CAN_ActivateNotification(BSP_CAN_GetHandle(BSP_CAN_2),
                               CAN_IT_RX_FIFO1_MSG_PENDING);

  gcan = can;
  inited = true;
  return DEVICE_OK;
}

CAN_t *CAN_GetDevice(void) {
  if (inited) {
    return gcan;
  }
  return NULL;
}

int8_t CAN_Motor_Control(CAN_MotorGroup_t group, CAN_Output_t *output) {
  if (output == NULL) return DEVICE_ERR_NULL;

  int16_t motor1, motor2, motor3, motor4;
  int16_t yaw_motor, pit_motor;
  int16_t fric1_motor, fric2_motor, trig_motor;

  switch (group) {
    case CAN_MOTOR_GROUT_CHASSIS:
      motor1 =
          (int16_t)(output->chassis.named.m1 * (float)CAN_M3508_MAX_ABS_LSB);
      motor2 =
          (int16_t)(output->chassis.named.m2 * (float)CAN_M3508_MAX_ABS_LSB);
      motor3 =
          (int16_t)(output->chassis.named.m3 * (float)CAN_M3508_MAX_ABS_LSB);
      motor4 =
          (int16_t)(output->chassis.named.m4 * (float)CAN_M3508_MAX_ABS_LSB);

      raw_tx1.tx_header.StdId = CAN_M3508_M2006_CTRL_ID_BASE;
      raw_tx1.tx_header.IDE = CAN_ID_STD;
      raw_tx1.tx_header.RTR = CAN_RTR_DATA;
      raw_tx1.tx_header.DLC = CAN_MOTOR_TX_BUF_SIZE;

      raw_tx1.tx_data[0] = (uint8_t)((motor1 >> 8) & 0xFF);
      raw_tx1.tx_data[1] = (uint8_t)(motor1 & 0xFF);
      raw_tx1.tx_data[2] = (uint8_t)((motor2 >> 8) & 0xFF);
      raw_tx1.tx_data[3] = (uint8_t)(motor2 & 0xFF);
      raw_tx1.tx_data[4] = (uint8_t)((motor3 >> 8) & 0xFF);
      raw_tx1.tx_data[5] = (uint8_t)(motor3 & 0xFF);
      raw_tx1.tx_data[6] = (uint8_t)((motor4 >> 8) & 0xFF);
      raw_tx1.tx_data[7] = (uint8_t)(motor4 & 0xFF);

      HAL_CAN_AddTxMessage(BSP_CAN_GetHandle(BSP_CAN_1), &raw_tx1.tx_header,
                           raw_tx1.tx_data, (uint32_t *)CAN_TX_MAILBOX0);
      break;

    case CAN_MOTOR_GROUT_GIMBAL1:
    case CAN_MOTOR_GROUT_GIMBAL2:
      yaw_motor =
          (int16_t)(output->gimbal.named.yaw * (float)CAN_GM6020_MAX_ABS_LSB);
      pit_motor =
          (int16_t)(output->gimbal.named.pit * (float)CAN_GM6020_MAX_ABS_LSB);

      raw_tx1.tx_header.StdId = CAN_GM6020_CTRL_ID_EXTAND;
      raw_tx1.tx_header.IDE = CAN_ID_STD;
      raw_tx1.tx_header.RTR = CAN_RTR_DATA;
      raw_tx1.tx_header.DLC = CAN_MOTOR_TX_BUF_SIZE;

      raw_tx1.tx_data[0] = (uint8_t)((yaw_motor >> 8) & 0xFF);
      raw_tx1.tx_data[1] = (uint8_t)(yaw_motor & 0xFF);
      raw_tx1.tx_data[2] = (uint8_t)((pit_motor >> 8) & 0xFF);
      raw_tx1.tx_data[3] = (uint8_t)(pit_motor & 0xFF);
      raw_tx1.tx_data[4] = 0;
      raw_tx1.tx_data[5] = 0;
      raw_tx1.tx_data[6] = 0;
      raw_tx1.tx_data[7] = 0;

      HAL_CAN_AddTxMessage(BSP_CAN_GetHandle(BSP_CAN_1), &raw_tx1.tx_header,
                           raw_tx1.tx_data, (uint32_t *)CAN_TX_MAILBOX1);
      break;

    case CAN_MOTOR_GROUT_SHOOT1:
    case CAN_MOTOR_GROUT_SHOOT2:
      fric1_motor =
          (int16_t)(output->shoot.named.fric1 * (float)CAN_M3508_MAX_ABS_LSB);
      fric2_motor =
          (int16_t)(output->shoot.named.fric2 * (float)CAN_M3508_MAX_ABS_LSB);
      trig_motor =
          (int16_t)(output->shoot.named.trig * (float)CAN_M2006_MAX_ABS_LSB);

      raw_tx2.tx_header.StdId = CAN_M3508_M2006_CTRL_ID_EXTAND;
      raw_tx2.tx_header.IDE = CAN_ID_STD;
      raw_tx2.tx_header.RTR = CAN_RTR_DATA;
      raw_tx2.tx_header.DLC = CAN_MOTOR_TX_BUF_SIZE;

      raw_tx2.tx_data[0] = (uint8_t)((fric1_motor >> 8) & 0xFF);
      raw_tx2.tx_data[1] = (uint8_t)(fric1_motor & 0xFF);
      raw_tx2.tx_data[2] = (uint8_t)((fric2_motor >> 8) & 0xFF);
      raw_tx2.tx_data[3] = (uint8_t)(fric2_motor & 0xFF);
      raw_tx2.tx_data[4] = (uint8_t)((trig_motor >> 8) & 0xFF);
      raw_tx2.tx_data[5] = (uint8_t)(trig_motor & 0xFF);
      raw_tx2.tx_data[6] = 0;
      raw_tx2.tx_data[7] = 0;

      HAL_CAN_AddTxMessage(BSP_CAN_GetHandle(BSP_CAN_2), &raw_tx2.tx_header,
                           raw_tx2.tx_data, (uint32_t *)CAN_TX_MAILBOX2);
      break;

    default:
      break;
  }
  return DEVICE_OK;
}

int8_t CAN_Motor_StoreMsg(CAN_t *can, CAN_RawRx_t *can_motor_rx) {
  if (can == NULL) return DEVICE_ERR_NULL;
  if (can_motor_rx == NULL) return DEVICE_ERR_NULL;

  int index;
  switch (can_motor_rx->rx_header.StdId) {
    case CAN_M3508_M1_ID:
    case CAN_M3508_M2_ID:
    case CAN_M3508_M3_ID:
    case CAN_M3508_M4_ID:
      index = can_motor_rx->rx_header.StdId - CAN_M3508_M1_ID;
      CAN_Motor_Parse(&(can->chassis_motor.as_array[index]),
                      can_motor_rx->rx_data);
      can->motor_flag |= 1 << index;
      break;

    case CAN_M3508_FRIC1_ID:
    case CAN_M3508_FRIC2_ID:
    case CAN_M2006_TRIG_ID:
      index = can_motor_rx->rx_header.StdId - CAN_M3508_FRIC1_ID;
      can->motor_flag |= 1 << (index + 6);
      CAN_Motor_Parse(&(can->shoot_motor.as_array[index]),
                      can_motor_rx->rx_data);
      break;

    case CAN_GM6020_YAW_ID:
    case CAN_GM6020_PIT_ID:
      index = can_motor_rx->rx_header.StdId - CAN_GM6020_YAW_ID;
      can->motor_flag |= 1 << (index + 4);
      CAN_Motor_Parse(&(can->gimbal_motor.as_array[index]),
                      can_motor_rx->rx_data);
      break;

    default:
      break;
  }
  return DEVICE_OK;
}

bool CAN_Motor_CheckFlag(CAN_t *can, uint32_t flag) {
  if (can == NULL) return false;
  return (can->motor_flag & flag) == flag;
}

int8_t CAN_Motor_ClearFlag(CAN_t *can, uint32_t flag) {
  if (can == NULL) return DEVICE_ERR_NULL;
  can->motor_flag &= ~flag;
  return DEVICE_OK;
}

void CAN_ResetChassisOut(CAN_ChassisOutput_t *chassis_out) {
  memset(chassis_out, 0, sizeof(CAN_ChassisOutput_t));
}

void CAN_ResetGimbalOut(CAN_GimbalOutput_t *gimbal_out) {
  memset(gimbal_out, 0, sizeof(CAN_GimbalOutput_t));
}

void CAN_ResetShootOut(CAN_ShootOutput_t *shoot_out) {
  memset(shoot_out, 0, sizeof(CAN_ShootOutput_t));
}

int8_t CAN_CapControl(float power_limit) {
  (void)power_limit;

  raw_tx2.tx_header.StdId = CAN_M3508_M2006_ID_SETTING_ID;
  raw_tx2.tx_header.IDE = CAN_ID_STD;
  raw_tx2.tx_header.RTR = CAN_RTR_DATA;
  raw_tx2.tx_header.DLC = CAN_MOTOR_TX_BUF_SIZE;

  HAL_CAN_AddTxMessage(BSP_CAN_GetHandle(BSP_CAN_1), &raw_tx2.tx_header,
                       raw_tx2.tx_data, (uint32_t *)CAN_TX_MAILBOX0);
  return DEVICE_OK;
}

void CAN_ResetCapOut(CAN_CapOutput_t *shoot_out) {
  memset(shoot_out, 0, sizeof(CAN_CapOutput_t));
}
