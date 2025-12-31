// #include "FreeRTOS.h"
// #include "task.h"
// #include "pico/stdlib.h"
// #include "pico/cyw43_arch.h"

// void vBlinkTask(void *pvParameters) {
//     while (1) {
//         cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
//         vTaskDelay(pdMS_TO_TICKS(500));
//         cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
//         vTaskDelay(pdMS_TO_TICKS(500));
//     }
// }

// int main(void) {
//     stdio_init_all();
    
//     if (cyw43_arch_init()) {
//         return -1;  // Init failed
//     }
    
//     xTaskCreate(vBlinkTask, "Blink", 256, NULL, 1, NULL);
//     vTaskStartScheduler();
    
//     while (1);
// }










// // This program turns the led off and onn, also pushes the updated value of led to a queue. (Thread 1)
// // The value is then read by other thread and then printed on the serial monitor. (Thread 2)
// #include<stdio.h>
// #include "pico/stdlib.h"
// #include "pico/cyw43_arch.h"
// #include "FreeRTOS.h"
// #include "task.h"
// #include "queue.h"

// static QueueHandle_t queue = NULL;

// void led_task(void * pvParameters) // inverts led and sends data to queue (every 2 sec)
// {
//     cyw43_arch_init();
//     uint led_state = 1;

//     while(true)
//     {
//         led_state = !led_state;
//         cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN , led_state);
//         xQueueSend(queue , &led_state , 0);
//         vTaskDelay(pdMS_TO_TICKS(2000));
//     }
// }

// void print_led_state(void * pvParameters) // reads led state out of queue every 2 sec
// {
//     uint led_state = 0;
    
//     while(true)
//     {
//         xQueueReceive(queue, &led_state , portMAX_DELAY); // portMAX_DELAY : waits for resource to be avaliable in the queue
//         printf("LED state : %d\n" , led_state);
//         vTaskDelay(pdMS_TO_TICKS(2000));
//     }
// }

// int main()
// {
//     stdio_init_all();

//     printf("PROCESS STARTED\n");

//     queue = xQueueCreate(1, sizeof(uint));

//     xTaskCreate(led_task , "LED_TASK" , 256 , NULL , 1 , NULL);
//     xTaskCreate(print_led_state, "READ_LED" , 256 , NULL , 1 , NULL);
    
//     vTaskStartScheduler();

//     return 0;
// }




























// This prog demonstrate states of a tasks as well as touches on block waiting and non block waiting
// 4 tasks states : Running, Ready, Blocked, Suspended
// Running : Task currently running on the CPU
// Ready : Task ready to be scheduled
// Blocked : Task waiting for external resource / event. vTaskDelay() blocks task for specefied cpu ticks.
// Suspended : Task in the suspended state can not go to running / ready state. But they are also not waiting for a event. To wake them up you have to use vTaskResume() and to suspend them you have to use vTaskSuspend()
// #include<stdio.h>
// #include"FreeRTOS.h"
// #include"task.h"
// #include"pico/stdlib.h"

// void task1(void * pvParameters) // this task should not run as task2 is higher priority and is not being blocked. (in case if we are just using 1 cpu out of 2)
// {
//     while(true)
//     {
//         printf("Task 1\n");
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

// void task2(void * pvParameters)
// {
//     while(true)
//     {
//         printf("Task 2\n");
//         for(int i = 0 ; i < 15000000  ; i++){} // non blocking waiting
//     }
// }

// int main()
// {
//     stdio_init_all();

//     xTaskCreate(task1, "task1", 256, NULL, 1, NULL);
//     xTaskCreate(task2, "task2", 256, NULL, 2, NULL);
//     vTaskStartScheduler();

//     return 0;
// }



































// This example focuses on mutex, which prvodes mutual exclusion and helps reducing race conditions on shared resources
// xSemaphoreTake(mutex, 0) -> tries to acquire the mutex and if not found then continues without delay (0 delay). Inplace of 0 if you use "portMAX_DELAY" the task will be sent to blocked state until mutex is released and then as soon as mutex is released the tasks acquires it automatically
// Expected output : 111
//                   222
//                   222
//                   111
// #include <stdio.h>
// #include <pico/stdlib.h>
// #include "FreeRTOS.h"
// #include "task.h"
// #include "semphr.h"

// static SemaphoreHandle_t mutex;

// void task1(void * pvParameters)
// {
//     char print = '1';
//     while (true)
//     {
//         if(xSemaphoreTake(mutex, 0) == pdTRUE)
//         {
//             for (int i = 0; i < 3; i++)
//             {
//                 printf("%c", print);
//             }
//             printf("\n");
//             xSemaphoreGive(mutex);
//         }
//     }
// }

// void task2(void * pvParameters)
// {
//     char print = '2';
//     while (true)
//     {
//         if(xSemaphoreTake(mutex, 0) == pdTRUE)
//         {
//             for (int i = 0; i < 3; i++)
//             {
//                 printf("%c", print);
//             }
//             printf("\n");
//             xSemaphoreGive(mutex);
//         }
//     }
// }

// int main()
// {
//     stdio_init_all();

//     mutex = xSemaphoreCreateMutex();

//     xTaskCreate(task1, "Task 1", 256, NULL, 1, NULL);
//     xTaskCreate(task2, "Task 2", 256, NULL, 1, NULL);

//     vTaskStartScheduler();

//     return 0;
// }




























// This code demonstrates SMP (Symmetric Multiprocessing) with freeRtos
// SMP means using more then 1 cores, and concurrency
// vTaskCoreAffinitySet(task, core) this func pins task to a specefied core
// You can also enable/disable preemption for a specefic tasks using vTaskPreemptionEnable() and vTaskPreemptionDisable()
// Expected output: Task 1 running on core : 0
//                  Task 2 running on core : 1
#include "FreeRTOS.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "pico/multicore.h"
#include "semphr.h"
#include "task.h"

static SemaphoreHandle_t mutex;
TaskHandle_t task1_handle;
TaskHandle_t task2_handle;

void print(uint taskNum) // making sure that 1 printf is done at a time on serial monitor
{
    xSemaphoreTake(mutex , portMAX_DELAY);
    printf("Task %d running on core : %d\n", taskNum, get_core_num());
    xSemaphoreGive(mutex);
}    

void task1 (void * pvParameters)
{
    while (true)
    {
        print(1);
        vTaskDelay(1000);
    }    
}    

void task2 (void * pvParameters)
{
    while (true)
    {
        print(2);
        vTaskDelay(1000);
    }    
}

int main()
{
    stdio_init_all();

    mutex = xSemaphoreCreateMutex();

    xTaskCreate(task1, "Task 1", 256, NULL, 1, &task1_handle);    
    xTaskCreate(task2, "Task 2", 256, NULL, 1, &task2_handle);   

    vTaskCoreAffinitySet(task1_handle, (1 << 0));
    vTaskCoreAffinitySet(task2_handle, (1 << 1));
    
    vTaskStartScheduler();

    return 0;
}