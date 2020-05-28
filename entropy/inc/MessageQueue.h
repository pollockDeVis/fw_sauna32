/*
 *	MessageQueue.h
 *
 *  Created on: 02 Feb 2020
 *      Author: Sadequr Rahman Rabby
 *
 * Description:
 *
 */
#ifndef __MESSAGE_QUEUE_H__
#define __MESSAGE_QUEUE_H__


#define MAX_MSG			200
#define	MSG_QUEUE_LEN	50

typedef void(*msgQueueCallback_t)(void *mMsg);

typedef enum {
	temp = 0,
	proximity,
	uart,
	bluetooth,
	MAX_MSG_SRC
}msg_src_t;

typedef struct {
	msg_src_t src;
	void *msg;
}msg_t;

typedef enum {
	mErr = 0,
	mPass,
	mInvalidSize,
	mInvalidSrc,
	mValid,
	mInvalid,
	mNoRecDefine,
	mUnkwn
}mStatus_t;


mStatus_t MessageQueue_Init(int _mSize);
mStatus_t MessageQueue_Send(msg_t *msg);
mStatus_t MessageQueue_IsValid(void);
mStatus_t MessageQueue_RegisterMsg(msg_src_t src, msgQueueCallback_t mCallback);
mStatus_t MessageQueue_DeregisterMsg(msgQueueCallback_t mCallback);


#endif
