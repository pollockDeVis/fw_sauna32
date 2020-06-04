/*
 *	MessageQueue.c
 *
 *  Created on: 02 Feb 2020
 *      Author: Sadequr Rahman Rabby
 * 
 * Description:
 *
 */

#include "MessageQueue.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "custom_list.h"
#include <string.h>

/*

*/

#define MESSAGE_QUEUE_STACK 		(1024*8)
static void msgQueueTask(void *pvParameters);
static void vProcessMsg(msg_src_t src, void *msg);
/*

*/
static QueueHandle_t msgQueue;
static TaskHandle_t msgQueueTaskHandler = NULL;
static const char *module_Name = "MessageQueue";
static list_t *recevierList = NULL;
const TickType_t xTicksToWait = pdMS_TO_TICKS(100UL);


mStatus_t MessageQueue_Init(int _mSize){
	if (_mSize == 0 || _mSize > MAX_MSG)
		return mInvalidSize;

	msgQueue = xQueueCreate(_mSize, sizeof(msg_t));
	if (msgQueue == NULL)
		return mErr;


	xTaskCreate(msgQueueTask, module_Name, MESSAGE_QUEUE_STACK , NULL, configMAX_PRIORITIES, &msgQueueTaskHandler);
	
	recevierList = (list_t*) heap_caps_malloc(MAX_MSG_SRC * sizeof(list_t), MALLOC_CAP_DEFAULT); //malloc(MAX_MSG_SRC * sizeof(list_t));

	for (unsigned int i = 0; i < MAX_MSG_SRC; i++){
		memset((void*)&recevierList[i], 0, sizeof(list_t));
	}
	return mPass;
}

mStatus_t MessageQueue_Send(msg_t *msg)
{
	if(msg->src >= MAX_MSG_SRC)
		return mInvalidSrc;
	if (msgQueue == NULL)
		return mErr;
	portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
	vTaskEnterCritical(&myMutex);
	unsigned int len = recevierList[msg->src].len;
	vTaskExitCritical(&myMutex);
	if (len == 0)
		return mNoRecDefine;

	if(pdPASS == xQueueSendToBack(msgQueue, msg, xTicksToWait))
		return mPass;
	
	return mErr;
}

mStatus_t MessageQueue_IsValid(void){
	if(msgQueueTaskHandler == NULL)
		return mInvalid;

	return mValid;
}

mStatus_t MessageQueue_RegisterMsg(msg_src_t src, msgQueueCallback_t mCallback)
{
	if (recevierList == NULL)
		return mErr;
	for (unsigned int i = 0; i < MAX_MSG_SRC; i++) {
		if (i == src) {
			list_node_t *node = list_node_new((void*)mCallback);
			portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
			vTaskEnterCritical(&myMutex);
			custom_list_lpush(&recevierList[i], node);
			vTaskExitCritical(&myMutex);
			return mPass;
		}
	}
	return mInvalidSrc;
}

mStatus_t MessageQueue_DeregisterMsg(msgQueueCallback_t mCallback)
{
	if (recevierList == NULL)
		return mErr;
	for (unsigned int i = 0; i < MAX_MSG_SRC; i++){
		list_node_t *node = custom_list_find(&recevierList[i], mCallback);
		if (node != NULL) {
			portMUX_TYPE myMutex = portMUX_INITIALIZER_UNLOCKED;
			vTaskEnterCritical(&myMutex);
			custom_list_remove(&recevierList[i], node);
			vTaskExitCritical(&myMutex);
			return mPass;
		}
	}
	return mInvalidSrc;
}

void msgQueueTask(void *pvParameters) {
	BaseType_t mStatus;
	msg_t mReceivedMsg;
	memset((void*)&mReceivedMsg, 0, sizeof(msg_t));
	while (1){
		mStatus = xQueueReceive(msgQueue, &mReceivedMsg, portMAX_DELAY);
		if (mStatus == pdPASS) {
			vProcessMsg(mReceivedMsg.src, mReceivedMsg.msg);
		}
	}
}

void vProcessMsg(msg_src_t src, void *msg) {
	if (src >= MAX_MSG_SRC)
		return;
	list_iterator_t *it = list_iterator_new(&recevierList[src], LIST_TAIL);
	list_node_t *eL = list_iterator_next(it);
	msgQueueCallback_t callback;
	while (eL) {
		callback = (msgQueueCallback_t)eL->val;
		callback(msg);
		eL = list_iterator_next(it);
	}

	list_iterator_destroy(it);
}
