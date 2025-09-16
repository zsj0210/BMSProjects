#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "freertos.h"
#include "task.h"
#include "cmsis_os2.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RS485_EN_Pin GPIO_PIN_4
#define RS485_EN_GPIO_Port GPIOA
#define RS485_PWREN_Pin GPIO_PIN_5
#define RS485_PWREN_GPIO_Port GPIOA
#define CAN_PWREN_Pin GPIO_PIN_10
#define CAN_PWREN_GPIO_Port GPIOB
#define I2C1_SDA_Pin GPIO_PIN_13
#define I2C1_SDA_GPIO_Port GPIOB
#define I2C1_SCL_Pin GPIO_PIN_14
#define I2C1_SCL_GPIO_Port GPIOB
#define BQ769X0_TS1_Pin GPIO_PIN_15
#define BQ769X0_TS1_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_5
#define LED_GPIO_Port GPIOB
#define LED1_Pin GPIO_PIN_6
#define LED1_GPIO_Port GPIOB
#define LED2_Pin GPIO_PIN_7
#define LED2_GPIO_Port GPIOB
#define LED3_Pin GPIO_PIN_8
#define LED3_GPIO_Port GPIOB
#define LED4_Pin GPIO_PIN_9
#define LED4_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
