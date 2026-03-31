//
// Created by msol0v on 30.03.2026.
//

#include "motor.h"

osMessageQueueId_t motorQueueHandle;

const osThreadAttr_t motorTask_attributes = {
    .name = "motorTask",
    .stack_size = 256 * 4,
    .priority = (osPriority_t) osPriorityAboveNormal,
  };

void MotorTask(void *argument) {
    MotorCommand_t rec_cmd;

    for(;;) {
        // Ждем команду из очереди бесконечно (osWaitForever)
        osStatus_t status = osMessageQueueGet(motorQueueHandle, &rec_cmd, NULL, osWaitForever);

        if (status == osOK) {
            // Здесь ваша основная логика движения
            printf("Executing move: Motor %d, Steps %ld\n", rec_cmd.motor_id, rec_cmd.steps);

            // Пример: запуск генерации PWM для шаговика
            // StartStepper(rec_cmd.motor_id, rec_cmd.steps, rec_cmd.dir);

            // Имитация работы (замените на реальное ожидание конца движения)
            osDelay(500);
        }
    }
}