#pragma once


/* Includes ------------------------------------------------------------------*/
#include <spi.h>

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum {
	BSP_SPI_OLED,
	BSP_SPI_IMU,
	/* BSP_SPI_XXX,*/
} BSP_SPI_t;

typedef enum {
	BSP_SPI_TX_COMPLETE_CB,			/* SPI Tx Completed callback ID */
	BSP_SPI_RX_COMPLETE_CB,			/* SPI Rx Completed callback ID */
	BSP_SPI_TX_RX_COMPLETE_CB,		/* SPI TxRx Completed callback ID */
	BSP_SPI_TX_HALF_COMPLETE_CB,	/* SPI Tx Half Completed callback ID */
	BSP_SPI_RX_HALF_COMPLETE_CB,	/* SPI Rx Half Completed callback ID */
	BSP_SPI_TX_RX_HALF_COMPLETE_CB,	/* SPI TxRx Half Completed callback ID */
	BSP_SPI_ERROR_CB,				/* SPI Error callback ID */
	BSP_SPI_ABORT_CB,				/* SPI Abort callback ID */
} BSP_SPI_Callback_t;

/* Exported functions prototypes ---------------------------------------------*/
SPI_HandleTypeDef *BSP_SPI_GetHandle(BSP_SPI_t spi);
int8_t BSP_SPI_RegisterCallback(BSP_SPI_t spi, BSP_SPI_Callback_t type, void (*callback)(void));
