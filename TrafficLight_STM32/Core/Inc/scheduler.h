/*
 * scheduler.h - Cooperative Scheduler O(1) (Delta-Delay Queue)
 */

#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdint.h>

#define SCH_MAX_TASKS    40
#define NO_TASK_ID       0

#define RETURN_ERROR     0
#define RETURN_NORMAL    1

#define SCH_TICK_MS      10  // Chu kỳ ngắt TIM2 (ms)

typedef struct {
    void     (*pTask)(void);
    uint32_t Delay;           // Delay tương đối so với task đứng trước (tick)
    uint32_t Period;          // Chu kỳ lặp lại (0 = chạy 1 lần rồi tự xóa)
    uint8_t  RunMe;
    uint32_t TaskID;
} sTask;

void     SCH_Init(void);
uint32_t SCH_Add_Task(void (*pFunc)(void), uint32_t DELAY, uint32_t PERIOD);
uint8_t  SCH_Delete_Task(uint32_t taskID);
void     SCH_Update(void);          // Gọi trong ngắt TIM2 - O(1)
void     SCH_Dispatch_Tasks(void);
uint32_t SCH_Get_Task_Count(void);

#endif /* INC_SCHEDULER_H_ */
