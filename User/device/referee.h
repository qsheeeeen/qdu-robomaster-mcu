/*
  裁判系统抽象。
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ----------------------------------------------------------------- */
#include <cmsis_os2.h>
#include <stdbool.h>

#include "component\cmd.h"
#include "component\ui.h"
#include "component\user_math.h"
#include "device\device.h"

/* Exported constants ------------------------------------------------------- */
/* Exported macro ----------------------------------------------------------- */
#define REF_SWITCH_STATUS(ref, stat) ((ref).ref_status = (stat))
#define CHASSIS_POWER_MAX_WITHOUT_REF 40.0f /* 裁判系统离线底盘最大功率 */

/* Exported types ----------------------------------------------------------- */
typedef struct __packed {
  uint8_t sof;
  uint16_t data_length;
  uint8_t seq;
  uint8_t crc8;
} Referee_Header_t;

typedef enum { REF_STATUS_OFFLINE = 0, REF_STATUS_RUNNING } Referee_Status_t;

typedef enum {
  REF_CMD_ID_GAME_STATUS = 0x0001,
  REF_CMD_ID_GAME_RESULT = 0x0002,
  REF_CMD_ID_GAME_ROBOT_HP = 0x0003,
  REF_CMD_ID_DART_STATUS = 0x0004,
  REF_CMD_ID_ICRA_ZONE_STATUS = 0x0005,
  REF_CMD_ID_FIELD_EVENTS = 0x0101,
  REF_CMD_ID_SUPPLY_ACTION = 0x0102,
  REF_CMD_ID_REQUEST_SUPPLY = 0x0103,
  REF_CMD_ID_WARNING = 0x0104,
  REF_CMD_ID_DART_COUNTDOWN = 0x0105,
  REF_CMD_ID_ROBOT_STATUS = 0x0201,
  REF_CMD_ID_POWER_HEAT_DATA = 0x0202,
  REF_CMD_ID_ROBOT_POS = 0x0203,
  REF_CMD_ID_ROBOT_BUFF = 0x0204,
  REF_CMD_ID_DRONE_ENERGY = 0x0205,
  REF_CMD_ID_ROBOT_DMG = 0x0206,
  REF_CMD_ID_SHOOT_DATA = 0x0207,
  REF_CMD_ID_BULLET_REMAINING = 0x0208,
  REF_CMD_ID_RFID = 0x0209,
  REF_CMD_ID_DART_CLIENT = 0x020A,
  REF_CMD_ID_INTER_STUDENT = 0x0301,
  REF_CMD_ID_INTER_STUDENT_CUSTOM = 0x0302,
  REF_CMD_ID_CLIENT_MAP = 0x0303,
  REF_CMD_ID_KEYBOARD_MOUSE = 0x0304,
} Referee_CMDID_t;

typedef struct __packed {
  uint8_t game_type : 4;
  uint8_t game_progress : 4;
  uint16_t stage_remain_time;
  uint64_t sync_time_stamp;
} Referee_GameStatus_t;

typedef struct __packed {
  uint8_t winner;
} Referee_GameResult_t;

typedef struct __packed {
  uint16_t red_1;
  uint16_t red_2;
  uint16_t red_3;
  uint16_t red_4;
  uint16_t red_5;
  uint16_t red_6;
  uint16_t red_7;
  uint16_t red_outpose;
  uint16_t red_base;
  uint16_t blue_1;
  uint16_t blue_2;
  uint16_t blue_3;
  uint16_t blue_4;
  uint16_t blue_5;
  uint16_t blue_6;
  uint16_t blue_7;
  uint16_t blue_outpose;
  uint16_t blue_base;
} Referee_GameRobotHP_t;

typedef struct __packed {
  uint8_t dart_belong;
  uint16_t stage_remain_time;
} Referee_DartStatus_t;

typedef struct __packed {
  uint8_t f1_status : 1;
  uint8_t f1_buff_status : 3;
  uint8_t f2_status : 1;
  uint8_t f2_buff_status : 3;
  uint8_t f3_status : 1;
  uint8_t f3_buff_status : 3;
  uint8_t f4_status : 1;
  uint8_t f4_buff_status : 3;
  uint8_t f5_status : 1;
  uint8_t f5_buff_status : 3;
  uint8_t f6_status : 1;
  uint8_t f6_buff_status : 3;
  uint16_t red1_bullet_remain;
  uint16_t red2_bullet_remain;
  uint16_t blue1_bullet_remain;
  uint16_t blue2_bullet_remain;
} Referee_ICRAZoneStatus_t;

typedef struct __packed {
  uint8_t copter_pad : 2;
  uint8_t energy_mech : 2;
  uint8_t virtual_shield : 1;
  uint32_t res : 27;
} Referee_FieldEvents_t;

typedef struct __packed {
  uint8_t supply_id;
  uint8_t robot_id;
  uint8_t supply_step;
  uint8_t supply_sum;
} Referee_SupplyAction_t;

typedef struct __packed {
  uint8_t place_holder; /* TODO */
} Referee_RequestSupply_t;

typedef struct __packed {
  uint8_t level;
  uint8_t robot_id;
} Referee_Warning_t;

typedef struct __packed {
  uint8_t countdown;
} Referee_DartCountdown_t;

typedef struct __packed {
  uint8_t robot_id;
  uint8_t robot_level;
  uint16_t remain_hp;
  uint16_t max_hp;
  uint16_t shoot_id1_17_cooling_rate;
  uint16_t shoot_id1_17_heat_limit;
  uint16_t shoot_id1_17_speed_limit;
  uint16_t shoot_id2_17_cooling_rate;
  uint16_t shoot_id2_17_heat_limit;
  uint16_t shoot_id2_17_speed_limit;
  uint16_t shoot_42_cooling_rate;
  uint16_t shoot_42_heat_limit;
  uint16_t shoot_42_speed_limit;
  uint16_t chassis_power_limit;
  uint8_t power_gimbal_output : 1;
  uint8_t power_chassis_output : 1;
  uint8_t power_shoot_output : 1;
} Referee_RobotStatus_t;

typedef struct __packed {
  uint16_t chassis_volt;
  uint16_t chassis_amp;
  float chassis_watt;
  uint16_t chassis_pwr_buff;
  uint16_t shoot_id1_17_heat;
  uint16_t shoot_id2_17_heat;
  uint16_t shoot_42_heat;
} Referee_PowerHeat_t;

typedef struct __packed {
  float x;
  float y;
  float z;
  float yaw;
} Referee_RobotPos_t;

typedef struct __packed {
  uint8_t healing : 1;
  uint8_t cooling_acc : 1;
  uint8_t defense_buff : 1;
  uint8_t attack_buff : 1;
  uint8_t res : 4;
} Referee_RobotBuff_t;

typedef struct __packed {
  uint8_t attack_countdown;
} Referee_DroneEnergy_t;

typedef struct __packed {
  uint8_t armor_id : 4;
  uint8_t damage_type : 4;
} Referee_RobotDamage_t;

typedef struct __packed {
  uint8_t bullet_type;
  uint8_t shooter_id;
  uint8_t bullet_freq;
  float bullet_speed;
} Referee_ShootData_t;

typedef struct __packed {
  uint16_t bullet_17_remain;
  uint16_t bullet_42_remain;
  uint16_t coin_remain;
} Referee_BulletRemain_t;

typedef struct __packed {
  uint8_t base : 1;
  uint8_t high_ground : 1;
  uint8_t energy_mech : 1;
  uint8_t slope : 1;
  uint8_t outpose : 1;
  uint8_t resource : 1;
  uint8_t healing_card : 1;
  uint32_t res : 24;
} Referee_RFID_t;

typedef struct __packed {
  uint8_t opening;
  uint8_t target;
  uint8_t target_changable_countdown;
  uint8_t dart1_speed;
  uint8_t dart2_speed;
  uint8_t dart3_speed;
  uint8_t dart4_speed;
  uint16_t last_dart_launch_time;
  uint16_t operator_cmd_launch_time;
} Referee_DartClient_t;

typedef struct __packed {
  float position_x;
  float position_y;
  float position_z;
  uint8_t commd_keyboard;
  uint16_t robot_id;
} Referee_ClientMap_t;

typedef struct __packed {
  int16_t mouse_x;
  int16_t mouse_y;
  int16_t mouse_wheel;
  int8_t button_l;
  int8_t button_r;
  uint16_t keyboard_value;
  uint16_t res;
} Referee_KeyboardMouse_t;

typedef uint16_t Referee_Tail_t;

typedef enum {
  REF_BOT_RED_HERO = 1,
  REF_BOT_RED_ENGINEER = 2,
  REF_BOT_RED_INFANTRY_1 = 3,
  REF_BOT_RED_INFANTRY_2 = 4,
  REF_BOT_RED_INFANTRY_3 = 5,
  REF_BOT_RED_DRONE = 6,
  REF_BOT_RED_SENTRY = 7,
  REF_BOT_RED_RADER = 9,
  REF_BOT_BLU_HERO = 101,
  REF_BOT_BLU_ENGINEER = 102,
  REF_BOT_BLU_INFANTRY_1 = 103,
  REF_BOT_BLU_INFANTRY_2 = 104,
  REF_BOT_BLU_INFANTRY_3 = 105,
  REF_BOT_BLU_DRONE = 106,
  REF_BOT_BLU_SENTRY = 107,
  REF_BOT_BLU_RADER = 109,
} Referee_RobotID_t;

typedef enum {
  REF_CL_RED_HERO = 0x0101,
  REF_CL_RED_ENGINEER = 0x0102,
  REF_CL_RED_INFANTRY_1 = 0x0103,
  REF_CL_RED_INFANTRY_2 = 0x0104,
  REF_CL_RED_INFANTRY_3 = 0x0105,
  REF_CL_RED_DRONE = 0x0106,
  REF_CL_BLU_HERO = 0x0165,
  REF_CL_BLU_ENGINEER = 0x0166,
  REF_CL_BLU_INFANTRY_1 = 0x0167,
  REF_CL_BLU_INFANTRY_2 = 0x0168,
  REF_CL_BLU_INFANTRY_3 = 0x0169,
  REF_CL_BLU_DRONE = 0x016A,
} Referee_ClientID_t;

typedef enum {
  REF_STDNT_CMD_ID_UI_DEL = 0x0100,
  REF_STDNT_CMD_ID_UI_DRAW1 = 0x0101,
  REF_STDNT_CMD_ID_UI_DRAW2 = 0x0102,
  REF_STDNT_CMD_ID_UI_DRAW5 = 0x0103,
  REF_STDNT_CMD_ID_UI_DRAW7 = 0x0104,
  REF_STDNT_CMD_ID_UI_STR = 0x0110,

  REF_STDNT_CMD_ID_CUSTOM = 0x0200,
} Referee_StudentCMDID_t;

typedef struct __packed {
  Referee_StudentCMDID_t data_cmd_id;
  uint16_t id_sender;
  uint16_t id_receiver;
  uint8_t *data;
} Referee_InterStudent_t;

typedef struct __packed {
  uint8_t place_holder;
} Referee_InterStudent_Custom_t;

typedef struct {
  osThreadId_t thread_alert;

  Referee_Status_t ref_status;
  Referee_GameStatus_t game_status;
  Referee_GameResult_t game_result;
  Referee_GameRobotHP_t game_robot_hp;
  Referee_DartStatus_t dart_status;
  Referee_ICRAZoneStatus_t icra_zone;
  Referee_FieldEvents_t field_event;
  Referee_SupplyAction_t supply_action;
  Referee_RequestSupply_t request_supply;
  Referee_Warning_t warning;
  Referee_DartCountdown_t dart_countdown;
  Referee_RobotStatus_t robot_status;
  Referee_PowerHeat_t power_heat;
  Referee_RobotPos_t robot_pos;
  Referee_RobotBuff_t robot_buff;
  Referee_DroneEnergy_t drone_energy;
  Referee_RobotDamage_t robot_danage;
  Referee_ShootData_t shoot_data;
  Referee_BulletRemain_t bullet_remain;
  Referee_RFID_t rfid;
  Referee_DartClient_t dart_client;
  Referee_InterStudent_Custom_t custom;
  Referee_ClientMap_t client_map;
  Referee_KeyboardMouse_t keyboard_mouse;
} Referee_t;

typedef struct __packed {
  UI_Ele_t grapic[7];
  UI_Drawcharacter_t character_data[3];
  uint8_t grapic_counter;
  uint8_t character_counter;
} Referee_UI_t;

typedef struct __packed {
  uint16_t data_cmd_id;
  uint16_t sender_ID;
  uint16_t receiver_ID;
} Referee_Interactive_Header_t;

typedef struct __packed {
  Referee_Header_t header;
  uint16_t cmd_id;
  Referee_Interactive_Header_t IA_header;
  UI_Drawgrapic_1_t data;
  uint16_t crc16;
} Referee_UI_Drawgrapic_1_t;

typedef struct __packed {
  Referee_Header_t header;
  uint16_t cmd_id;
  Referee_Interactive_Header_t IA_header;
  UI_Drawgrapic_2_t data;
  uint16_t crc16;
} Referee_UI_Drawgrapic_2_t;

typedef struct __packed {
  Referee_Header_t header;
  uint16_t cmd_id;
  Referee_Interactive_Header_t IA_header;
  UI_Drawgrapic_5_t data;
  uint16_t crc16;
} Referee_UI_Drawgrapic_5_t;

typedef struct __packed {
  Referee_Header_t header;
  uint16_t cmd_id;
  Referee_Interactive_Header_t IA_header;
  UI_Drawgrapic_7_t data;
  uint16_t crc16;
} Referee_UI_Drawgrapic_7_t;

typedef struct __packed {
  Referee_Header_t header;
  uint16_t cmd_id;
  Referee_Interactive_Header_t IA_header;
  UI_Drawcharacter_t data;
  uint16_t crc16;
} Referee_UI_Drawcharacter_t;

typedef struct {
  Referee_Status_t ref_status;
  float chassis_watt;
  float chassis_power_limit;
  float chassis_pwr_buff;
} Referee_ForCap_t;

typedef struct {
  Referee_Status_t ref_status;
} Referee_ForAI_t;

typedef struct {
  Referee_Status_t ref_status;
  float chassis_power_limit;
  float chassis_pwr_buff;
} Referee_ForChassis_t;

typedef struct {
  Referee_Status_t ref_status;
  Referee_PowerHeat_t power_heat;
  Referee_RobotStatus_t robot_status;
} Referee_ForShoot_t;

/* Exported functions prototypes -------------------------------------------- */
int8_t Referee_Init(Referee_t *ref, osThreadId_t thread_alert);
int8_t Referee_Restart(void);

int8_t Referee_StartReceiving(Referee_t *ref);
int8_t Referee_Parse(Referee_t *ref);
void Referee_HandleOffline(Referee_t *referee);
int8_t Referee_SetHeader(Referee_Interactive_Header_t *header,
                         Referee_StudentCMDID_t cmd_id, uint8_t sender_id);
int8_t Referee_StartSend(uint8_t *data, uint32_t len);
int8_t Referee_MoveData(void *data, void *tmp, uint32_t len);
int8_t Referee_PackUI(Referee_UI_t *ui, Referee_t *ref);
UI_Ele_t *Referee_GetGrapicAdd(Referee_UI_t *ref_ui);
UI_Drawcharacter_t *Referee_GetCharacterAdd(Referee_UI_t *ref_ui);
uint8_t Referee_PraseCmd(Referee_UI_t *ref_ui, CMD_UI_t cmd);
uint8_t Referee_PackCap(Referee_ForCap_t *cap, const Referee_t *ref);
uint8_t Referee_PackShoot(Referee_ForShoot_t *ai, Referee_t *ref);
uint8_t Referee_PackChassis(Referee_ForChassis_t *chassis,
                            const Referee_t *ref);
uint8_t Referee_PackAI(Referee_ForAI_t *shoot, const Referee_t *ref);
#ifdef __cplusplus
}
#endif
