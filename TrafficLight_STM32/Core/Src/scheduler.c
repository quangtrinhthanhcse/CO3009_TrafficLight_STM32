/*
 * scheduler.c - Cooperative Scheduler O(1) dùng mảng Delta-Delay
 * Hàng đợi luôn xếp tăng dần; mỗi task lưu Delay tương đối so với task đứng trước
 * để SCH_Update() chỉ cần giảm phần tử [0].
 */

#include "scheduler.h"
#include "stm32f1xx_hal.h"
#include <stddef.h>

static sTask SCH_tasks_G[SCH_MAX_TASKS]; // Dùng mảng tĩnh, tránh malloc gây phân mảnh RAM
static volatile uint32_t SCH_count = 0;
static uint32_t next_task_id = 1;

// Cờ hỗ trợ trường hợp task tự gọi SCH_Delete_Task() khi đang chạy
static uint32_t running_task_id = NO_TASK_ID;
static uint8_t  delete_running_task = 0;

static uint32_t sch_insert_task(void (*pFunc)(void), uint32_t delay, uint32_t period, uint32_t task_id) {
    if (pFunc == NULL || task_id == NO_TASK_ID) {
        return NO_TASK_ID;
    }

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    if (SCH_count >= SCH_MAX_TASKS) {
        __set_PRIMASK(primask);
        return NO_TASK_ID;
    }

    // Tìm vị trí chèn và tính độ trễ tương đối (remaining) so với các task đứng trước
    uint32_t pos = 0;
    uint32_t remaining = delay;

    while (pos < SCH_count && remaining >= SCH_tasks_G[pos].Delay) {
        remaining -= SCH_tasks_G[pos].Delay;
        pos++;
    }

    for (uint32_t i = SCH_count; i > pos; i--) {
        SCH_tasks_G[i] = SCH_tasks_G[i - 1];
    }

    SCH_tasks_G[pos].pTask  = pFunc;
    SCH_tasks_G[pos].Delay  = remaining;
    SCH_tasks_G[pos].Period = period;
    SCH_tasks_G[pos].RunMe  = (pos == 0 && remaining == 0) ? 1 : 0;
    SCH_tasks_G[pos].TaskID = task_id;

    SCH_count++;

    // Trừ bớt Delay của task đứng ngay sau để giữ đúng tổng thời gian chờ
    if (pos + 1 < SCH_count) {
        SCH_tasks_G[pos + 1].Delay -= remaining;
    }

    __set_PRIMASK(primask);
    return task_id;
}

void SCH_Init(void) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    for (uint32_t i = 0; i < SCH_MAX_TASKS; i++) {
        SCH_tasks_G[i].pTask  = NULL;
        SCH_tasks_G[i].Delay  = 0;
        SCH_tasks_G[i].Period = 0;
        SCH_tasks_G[i].RunMe  = 0;
        SCH_tasks_G[i].TaskID = NO_TASK_ID;
    }

    SCH_count = 0;
    next_task_id = 1;
    running_task_id = NO_TASK_ID;
    delete_running_task = 0;

    __set_PRIMASK(primask);
}

uint32_t SCH_Add_Task(void (*pFunc)(void), uint32_t DELAY, uint32_t PERIOD) {
    if (pFunc == NULL) {
        return NO_TASK_ID;
    }

    uint32_t task_id = next_task_id++;
    if (next_task_id == NO_TASK_ID) { // Tránh 0 vì dành cho NO_TASK_ID
        next_task_id = 1;
    }

    return sch_insert_task(pFunc, DELAY, PERIOD, task_id);
}

uint8_t SCH_Delete_Task(uint32_t taskID) {
    if (taskID == NO_TASK_ID) {
        return RETURN_ERROR;
    }

    if (taskID == running_task_id) {
        delete_running_task = 1;
        return RETURN_NORMAL;
    }

    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    for (uint32_t i = 0; i < SCH_count; i++) {
        if (SCH_tasks_G[i].TaskID == taskID) {
            uint8_t was_ready_head = (i == 0 && SCH_tasks_G[0].RunMe > 0) ? 1 : 0;

            // Cộng dồn Delay của task bị xóa vào task kế tiếp để không lệch lịch
            if (i + 1 < SCH_count) {
                SCH_tasks_G[i + 1].Delay += SCH_tasks_G[i].Delay;
            }

            for (uint32_t j = i; j + 1 < SCH_count; j++) {
                SCH_tasks_G[j] = SCH_tasks_G[j + 1];
            }

            SCH_count--;
            SCH_tasks_G[SCH_count].pTask  = NULL;
            SCH_tasks_G[SCH_count].Delay  = 0;
            SCH_tasks_G[SCH_count].Period = 0;
            SCH_tasks_G[SCH_count].RunMe  = 0;
            SCH_tasks_G[SCH_count].TaskID = NO_TASK_ID;

            if (was_ready_head && SCH_count > 0 && SCH_tasks_G[0].Delay == 0) {
                SCH_tasks_G[0].RunMe = 1;
            }

            __set_PRIMASK(primask);
            return RETURN_NORMAL;
        }
    }

    __set_PRIMASK(primask);
    return RETURN_ERROR;
}

void SCH_Update(void) {
    if (SCH_count == 0) {
        return;
    }

    if (SCH_tasks_G[0].Delay > 0) {
        SCH_tasks_G[0].Delay--;
    }

    if (SCH_tasks_G[0].Delay == 0) {
        SCH_tasks_G[0].RunMe = 1;
    }
}

void SCH_Dispatch_Tasks(void) {
    while (1) {
        sTask current;
        uint8_t has_task = 0;

        uint32_t primask = __get_PRIMASK();
        __disable_irq();

        if (SCH_count > 0 && SCH_tasks_G[0].RunMe > 0) {
            current = SCH_tasks_G[0];
            has_task = 1;

            for (uint32_t i = 0; i + 1 < SCH_count; i++) {
                SCH_tasks_G[i] = SCH_tasks_G[i + 1];
            }
            SCH_count--;
            SCH_tasks_G[SCH_count].pTask  = NULL;
            SCH_tasks_G[SCH_count].Delay  = 0;
            SCH_tasks_G[SCH_count].Period = 0;
            SCH_tasks_G[SCH_count].RunMe  = 0;
            SCH_tasks_G[SCH_count].TaskID = NO_TASK_ID;

            // Kích hoạt luôn nếu task kế tiếp có cùng thời điểm chạy (Delay == 0)
            if (SCH_count > 0 && SCH_tasks_G[0].Delay == 0) {
                SCH_tasks_G[0].RunMe = 1;
            }

            running_task_id = current.TaskID;
            delete_running_task = 0;
        }

        __set_PRIMASK(primask);

        if (!has_task) {
            break;
        }

        if (current.pTask != NULL) {
            (*current.pTask)();
        }

        // Task tuần hoàn (Period > 0) được nạp lại vào hàng đợi với TaskID cũ
        if (current.Period > 0 && !delete_running_task) {
            sch_insert_task(current.pTask, current.Period, current.Period, current.TaskID);
        }

        running_task_id = NO_TASK_ID;
        delete_running_task = 0;
    }
}

uint32_t SCH_Get_Task_Count(void) {
    return SCH_count;
}
