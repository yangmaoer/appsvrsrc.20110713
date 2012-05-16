#ifndef NEWTIMER_H
#define NEWTIMER_H 


#include <pthread.h>
#include "OurDef.h"


#define MAXARGLEN	128

typedef void (*TIMER_CALLBACK_FUN) ( void * obj, void * arg, int argLen );

class TimerManager;

//����delete �Ļص���������
class SelfDeleteCallback
{
public:
	friend class NewTimer;
	SelfDeleteCallback(TIMER_CALLBACK_FUN callBack, void * obj, void* arg, int argLen );
	~SelfDeleteCallback();
	
	void NewThreadCallBack();
	static void * detach_thread_main(void * p);
	void DetachProcess();
	

private:
	//---------------�ص��������----------------------------
	//�ص�������ַ
	TIMER_CALLBACK_FUN	_callBack;
	
	//�����ַ
	void * _obj;

	//�ص������Ĳ���
	char _arg[MAXARGLEN];

	//�ص������Ĳ�������
	int _argLen;
};

class NewTimer
{
public:
	friend class TimerManager;
	
public:
	enum TimerType
	{
		LoopTimer,
		OnceTimer,
		TimeTypeNum,
	};
	
	NewTimer();
	virtual ~NewTimer();
	int SetCallbackFun( TIMER_CALLBACK_FUN fun, void * obj,  void * arg, int argLen );
	void Interval(int input);
	UInt32 ID();
	TimerType Type();
	void Type(TimerType input);
	int OnTick();
	void OnTimeout();
	bool cancel();
	bool start(TimerManager* tm);
	bool reStart();
	bool IsRunning() {return _tm != 0;}  //_tm��Ϊ�ձ�ʾ����TimerManager���е���ʱ
	void* GetCallbackFunParam() {return (void*)_arg;}
private:
	//---------------�ص��������----------------------------
	//�ص�������ַ
	TIMER_CALLBACK_FUN	_callBack;
	
	//�����ַ
	void * _obj;

	//�ص������Ĳ���
	char _arg[MAXARGLEN];

	//�ص������Ĳ�������
	int _argLen;

   
private:
	//---------------��ʱ������----------------------------
	
	//��ʱ������ 0 ѭ����(ִ�����޴λص�)	  1 ����(ִ��һ�λص�)
	TimerType _type;

	//��ʱ�� ID
	UInt32 _ID;
 	
	//��� ��ʱ����
//	int _maxTimerNum;

	//ʱ��Ƭ ʱ��
//	int _timeSlot;
	
	//��ʱ�� ��ʱ���
	int _interval;

	//ʱ��Ƭ��
	//	��ʱ��� ����ʱ��Ƭ����
	//	ticks = interval / timeslot
	int _ticks;

	//ʱ����	Ȧ��
	//	round = ticks / maxTimerNum
	//	round Ϊ0 ��ʾ��ʱ��ʱ�䳬ʱ,
	//	currentSlot ����ʱ round--
	int _round;

	//ʱ���ֲۺ�
	//	ȡֵ��Χ	0~ (_maxTimerNum-1)
	//	slotNum = currentSlot + (ticks % maxTimerNum)
//	int _slotNum;

	TimerManager* _tm;

};

#endif
