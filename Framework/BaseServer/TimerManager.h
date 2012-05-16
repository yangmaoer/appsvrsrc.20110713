#ifndef TIMERMANAGER_H
#define TIMERMANAGER_H 
//	����ʱ�����㷨�Ķ�ʱ��
//	�㷨������� <���߳�Ӧ���еĶ�ʱ�������㷨>

#include <map>
#include <list>
#include "Mutex.h"
#include "OurDef.h"

#define	MAXTIMERNUM				3000						//���ʱ������
#define MAXSLOTNUM				3000						//���۸���
#define TIMERTIMESLOT			1							//ʱ��Ƭʱ��	1 ��

using namespace std;

class NewTimer;

class TimerPool
{
public:
	~TimerPool();
	bool			init(int timerCount = MAXTIMERNUM);
	NewTimer*		newTimer();
	void 			delTimer(NewTimer* timer); //����ӿ���ʱû�õ���Ҳ����˵timerһֱռ��
private:
	std::list<NewTimer*>    _freeTimerList; //Guarded by timerMutex_
	std::list<NewTimer*>    _usedTimerList;//Guarded by timerMutex_
	mutable MutexLock 		timerMutex_;
};


//ʱ���ֵ� �ֲ�
class Slot
{
public:
	int	AddTimer(NewTimer* timer);
	bool DelTimer(NewTimer* timer);
	list<NewTimer*> OnTick();

private:
	//�߳���
	mutable MutexLock mutex_;

	//��ʱ������
	map<UInt32, NewTimer*> _mapTimer; //Guarded by mutex_
};

//��ʱ��������
class TimerManager
{
public:
	TimerManager();
	~TimerManager();
	static void * tick_thread_main(void *);
	void TickThreadProcess();
	Byte IsStop();
	void Stop();
	void OnTick();
	int AddTimer(NewTimer* timer, int isNewTimer = 1);
	int DelTimer(NewTimer* timer);

private:	
	void AttachTimer( NewTimer* timer, int isNewTimer );

private:
	//ʱ��������
	Slot _slots[MAXSLOTNUM];
	
	//��¼���һ�� timer��ID
	//		ȡֵ��Χ 1 ~ MAXTIMERNUM
	UInt32	_lastTimerID;

	//��ʱ����ֹͣ��־
	//	0 δֹͣ  1 ֹͣ
	Byte	_isStop;

	//��ǰʱ��Ƭָ��
	//	ȡֵ��Χ	0 ~ (MAXTIMERNUM-1)
	UInt32	_currSlot;

	

};

#endif
