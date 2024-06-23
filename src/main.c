#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "diag/trace.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include <time.h>

#define CCM_RAM __attribute__((section(".ccmram")))
#define QUEUE_SIZE 3
#define TASK_STACK_SIZE 200

// global variables
uint32_t sum1 = 0, sum2 = 0, sum3 = 0;
uint32_t M1 = 0, M2 = 0, M3 = 0;
uint32_t random_value;

#define INITIAL_LOWER_BOUND 50
#define INITIAL_UPPER_BOUND 150
#define RECEIVER_PERIOD 100 // 100 msec

// Variables for message counts
static uint32_t sentCount[3] = { 0 };
static uint32_t blockedCount[3] = { 0 };
static uint32_t receivedCount = 0;

// uniform distribution bounds
static uint32_t currentLowerBound = INITIAL_LOWER_BOUND;
static uint32_t currentUpperBound = INITIAL_UPPER_BOUND;
static const uint32_t lowerBounds[] = { 50, 80, 110, 140, 170, 200 };
static const uint32_t upperBounds[] = { 150, 200, 250, 300, 350, 400 };
static uint32_t iteration = 0;

QueueHandle_t xQueue;
SemaphoreHandle_t senderSemaphores[3];
SemaphoreHandle_t receiverSemaphore;
TimerHandle_t senderTimers[3];
TimerHandle_t receiverTimer;

// Function prototypes
void vSenderTask1(void *pvParameters);
void vSenderTask2(void *pvParameters);
void vSenderTask3(void *pvParameters);
void vReceiverTask(void *pvParameters);
void vSenderTimerCallback(TimerHandle_t xTimer); //release semaphore
void vReceiverTimerCallback(TimerHandle_t xTimer);
void vResetSystem(void);
void intialvResetSystem(void);
uint32_t getRandomTime(uint32_t lower, uint32_t upper);

// Main function
int main(void) {
	srand(time(NULL)); // Seed the random number generator

	xQueue = xQueueCreate(QUEUE_SIZE, sizeof(char[20]));
	for (int i = 0; i < 3; i++) {
		senderSemaphores[i] = xSemaphoreCreateBinary();
	}
	receiverSemaphore = xSemaphoreCreateBinary();

	xTaskCreate(vSenderTask1, "Sender1", TASK_STACK_SIZE, (void*) 0, 1, NULL);
	xTaskCreate(vSenderTask2, "Sender2", TASK_STACK_SIZE, (void*) 1, 1, NULL);
	xTaskCreate(vSenderTask3, "Sender3", TASK_STACK_SIZE, (void*) 2, 2, NULL);
	xTaskCreate(vReceiverTask, "Receiver", TASK_STACK_SIZE, NULL, 3, NULL);

	senderTimers[0] = xTimerCreate("SenderTimer1",
			pdMS_TO_TICKS(
					getRandomTime(currentLowerBound, currentUpperBound)),
			pdTRUE, (void*) 0, vSenderTimerCallback);
	senderTimers[1] = xTimerCreate("SenderTimer2",
			pdMS_TO_TICKS(
					getRandomTime(currentLowerBound, currentUpperBound)),
			pdTRUE, (void*) 1, vSenderTimerCallback);
	senderTimers[2] = xTimerCreate("SenderTimer3",
			pdMS_TO_TICKS(
					getRandomTime(currentLowerBound, currentUpperBound)),
			pdTRUE, (void*) 2, vSenderTimerCallback);
	receiverTimer = xTimerCreate("ReceiverTimer",
			pdMS_TO_TICKS(RECEIVER_PERIOD), pdTRUE, NULL,
			vReceiverTimerCallback);

	for (int i = 0; i < 3; i++) {
		xTimerStart(senderTimers[i], 0);
	}
	xTimerStart(receiverTimer, 0);

	intialvResetSystem(); // Initial reset

	vTaskStartScheduler();

	for (;;) {
	}
	return 0;
}

// Sender task1 implementation
void vSenderTask1(void *pvParameters) {
	uint32_t taskId = (uint32_t) pvParameters;
	char message[20];

	for (;;) {
		xSemaphoreTake(senderSemaphores[taskId], portMAX_DELAY);

		// Send message
		sprintf(message, "Time is %d", xTaskGetTickCount());
	    printf("Sender %d: Sending message: %s\n", taskId+1, message); // Debug print
		if (xQueueSend(xQueue, &message, 0) == pdPASS) {
			sentCount[taskId]++;
		} else {
			blockedCount[taskId]++;
		}

		vTaskDelay(
				pdMS_TO_TICKS(
						getRandomTime(currentLowerBound,
								currentUpperBound)));
		M1++;
		sum1 += random_value;
	}
}

// Sender task2 implementation
void vSenderTask2(void *pvParameters) {
	uint32_t taskId = (uint32_t) pvParameters;
	char message[20];

	for (;;) {
		xSemaphoreTake(senderSemaphores[taskId], portMAX_DELAY);

		// Send message
		sprintf(message, "Time is %d", xTaskGetTickCount());
		printf("Sender %d: Sending message: %s\n", taskId+1, message); // Debug print

		if (xQueueSend(xQueue, &message, 0) == pdPASS) {
			sentCount[taskId]++;
		} else {
			blockedCount[taskId]++;
		}

		vTaskDelay(
				pdMS_TO_TICKS(
						getRandomTime(currentLowerBound,
								currentUpperBound)));
		M2++;
		sum2 += random_value;
	}
}

// Sender task3 implementation
void vSenderTask3(void *pvParameters) {
	uint32_t taskId = (uint32_t) pvParameters;
	char message[20];

	for (;;) {
		xSemaphoreTake(senderSemaphores[taskId], portMAX_DELAY);

		// Send message
		sprintf(message, "Time is %d", xTaskGetTickCount());
	    printf("Sender %d: Sending message: %s\n", taskId+1, message); // Debug print

		if (xQueueSend(xQueue, &message, 0) == pdPASS) {
			sentCount[taskId]++;
		} else {
			blockedCount[taskId]++;
		}

		vTaskDelay(
				pdMS_TO_TICKS(
						getRandomTime(currentLowerBound,
								currentUpperBound)));
		M3++;
		sum3 += random_value;
	}
}
// Receiver task implementation
void vReceiverTask(void *pvParameters) {
	char message[20];

	for (;;) {
		xSemaphoreTake(receiverSemaphore, portMAX_DELAY);

		// Read message
		if (xQueueReceive(xQueue, &message, 0) == pdPASS) {
			receivedCount++;
			printf("Receiver: Received message: %s\n", message); // Debug print

		}

		if (receivedCount >= 1000) {
			vResetSystem();

		}
	}
}

// Sender timer callback function
void vSenderTimerCallback(TimerHandle_t xTimer) {
	uint32_t timerId = (uint32_t) pvTimerGetTimerID(xTimer);
	xSemaphoreGive(senderSemaphores[timerId]);
}

// Receiver timer callback function
void vReceiverTimerCallback(TimerHandle_t xTimer) {
	xSemaphoreGive(receiverSemaphore);
}

// Reset function
void vResetSystem(void) {
	printf("Total Sent Messages: %lu\n",
			sentCount[0] + sentCount[1] + sentCount[2]);
	printf("Total Blocked Messages: %lu\n",
			blockedCount[0] + blockedCount[1] + blockedCount[2]);
	printf("Total Received Messages: %lu\n", receivedCount);
	for (int i = 0; i < 3; i++) {
		printf("Sender %d: Sent = %lu, Blocked = %lu\n", i + 1, sentCount[i],
				blockedCount[i]);
	}
	sentCount[0] = sentCount[1] = sentCount[2] = 0;
	blockedCount[0] = blockedCount[1] = blockedCount[2] = 0;
	receivedCount = 0;

	xQueueReset(xQueue);

	uint32_t avg1, avg2, avg3;
	avg1 = sum1 / M1;
	avg2 = sum2 / M2;
	avg3 = sum3 / M3;
	printf("average1 = %lu,average2 = %lu,average3 = %lu\n", avg1, avg2, avg3);
	printf("total average = %lu\n", (avg1 + avg2 + avg3) / 3);
	sum1 = sum2 = sum3 = M1 = M2 = M3 = 0;
	printf("\n");

	iteration++;
	if (iteration >= sizeof(lowerBounds) / sizeof(lowerBounds[0])) {
		printf("Game Over\n");
		for (int i = 0; i < 3; i++) {
			xTimerDelete(senderTimers[i], 0);
		}
		xTimerDelete(receiverTimer, 0);
		vTaskSuspendAll();
		for (;;) {
		} // Infinite loop to stop execution
	} else {
		currentLowerBound = lowerBounds[iteration];
		currentUpperBound = upperBounds[iteration];
		for (int i = 0; i < 3; i++) {
			xTimerChangePeriod(senderTimers[i],
					pdMS_TO_TICKS(getRandomTime(currentLowerBound, currentUpperBound)),
					0);
		}
	}

}
void intialvResetSystem(void) {
	sentCount[0] = sentCount[1] = sentCount[2] = 0;
	blockedCount[0] = blockedCount[1] = blockedCount[2] = 0;
	receivedCount = 0;
	xQueueReset(xQueue);
	currentLowerBound = lowerBounds[iteration];
	currentUpperBound = upperBounds[iteration];
	for (int i = 0; i < 3; i++) {
		xTimerChangePeriod(senderTimers[i],
				pdMS_TO_TICKS(getRandomTime(currentLowerBound, currentUpperBound)),
				0);
	}

}

// Generate a random time within a range
uint32_t getRandomTime(uint32_t lower, uint32_t upper) {
	random_value = (rand() % (upper - lower + 1)) + lower;
	return random_value;
}

//----------------------------------------------------

void vApplicationMallocFailedHook(void) {
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	 free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	 internally by FreeRTOS API functions that create tasks, queues, software
	 timers, and semaphores.  The size of the FreeRTOS heap is set by the
	 configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
	for (;;)
		;
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
	(void) pcTaskName;
	(void) pxTask;

	/* Run time stack overflow checking is performed if
	 configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	 function is called if a stack overflow is detected. */
	for (;;)
		;
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void) {
	volatile size_t xFreeStackSpace;

	/* This function is called on each cycle of the idle task.  In this case it
	 does nothing useful, other than report the amout of FreeRTOS heap that
	 remains unallocated. */
	xFreeStackSpace = xPortGetFreeHeapSize();

	if (xFreeStackSpace > 100) {
		/* By now, the kernel has allocated everything it is going to, so
		 if there is a lot of heap remaining unallocated then
		 the value of configTOTAL_HEAP_SIZE in FreeRTOSConfig.h can be
		 reduced accordingly. */
	}
}

void vApplicationTickHook(void) {
}

StaticTask_t xIdleTaskTCB CCM_RAM;
StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE] CCM_RAM;

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
		StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize) {
	/* Pass out a pointer to the StaticTask_t structure in which the Idle task's
	 state will be stored. */
	*ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

	/* Pass out the array that will be used as the Idle task's stack. */
	*ppxIdleTaskStackBuffer = uxIdleTaskStack;

	/* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
	 Note that, as the array is necessarily of type StackType_t,
	 configMINIMAL_STACK_SIZE is specified in words, not bytes. */
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

static StaticTask_t xTimerTaskTCB CCM_RAM;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH] CCM_RAM;

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
 application must provide an implementation of vApplicationGetTimerTaskMemory()
 to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
		StackType_t **ppxTimerTaskStackBuffer, uint32_t *pulTimerTaskStackSize) {
	*ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
	*ppxTimerTaskStackBuffer = uxTimerTaskStack;
	*pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
