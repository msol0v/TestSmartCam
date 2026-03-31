//
// Created by msol0v on 30.03.2026.
//

#ifndef ETHTEST_1_MOTOR_H
#define ETHTEST_1_MOTOR_H

#include <stdint.h>
#include <stdio.h>
#include "cmsis_os2.h"

typedef enum {
    MOTOR_DIR_CW = 0,
    MOTOR_DIR_CCW = 1
  } MotorDir_t;

typedef struct {
    uint8_t motor_id;   // Какой мотор (1, 2 или 3)
    int32_t steps;      // Сколько шагов
    MotorDir_t dir;     // Направление
} MotorCommand_t;

extern osMessageQueueId_t motorQueueHandle;
extern const osThreadAttr_t motorTask_attributes;

void MotorTask(void *argument);

#endif //ETHTEST_1_MOTOR_H