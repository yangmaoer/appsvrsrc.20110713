#ifndef TIMER_H
#define TIMER_H 

#include <pthread.h>
#include "OurDef.h"


typedef void (*CALLBACK_FUN) ( void * obj, UInt32 arg );

class Timer
{
public:	
	Timer(unsigned int input);
	virtual ~Timer();
	void CountDown(long second);
	void AddSec(long second);
	 	
	void SetCallbackFun( CALLBACK_FUN fun, void * obj,  UInt32 arg );

	long GetTimeElapse();
	long GetCountDownSec();
	
private:
	void ThreadProcess();
	void HandleTimeout();
	static void * thread_main(void *p)
	{
		(static_cast<Timer*>(p))->ThreadProcess();
	}
	
private:
	//���߳�ID
	pthread_t _threadId;
	
	//�߳���
	pthread_mutex_t	_mutexSleep;
	//��������
	pthread_cond_t _condSleep;
	
	//�߳���
	pthread_mutex_t	_mutex;
	
	//��������
	pthread_cond_t _cond;

	//�ص�����1
	CALLBACK_FUN _fun;
	
	//�ص�����2
	void * _obj;
	
	//�ص�����3
	UInt32 _roleID;
	char _userID[32];

	int _stop;
    
private:
	//��ʱ��ʼʱ��
	long _initTime;
	
 	//��ʱ������ 0 ʱ�䵽����������Timer����    1 ʱ�䵽�����ҴݻٵĶ���
	unsigned int _type;
	
	//����ʱ������
	long _secCountDown;
    
	

};

#endif
