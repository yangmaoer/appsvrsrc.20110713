#include "TimerManager.h"
#include <sys/select.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "DebugData.h"
#include <string.h>
#include "Log.h"
#include "NewTimer.h"
#include <algorithm>

using namespace std;


bool TimerPool::init(int timerCount)
{
	NewTimer* timers = new NewTimer[timerCount];

	for (int i = 0; i < timerCount; i++)
	{
		_freeTimerList.push_back(&timers[i]);
	}

	return true;
}

TimerPool::~TimerPool()
{
	std::list<NewTimer*>::iterator timerItor = _freeTimerList.begin();
	for (; timerItor != _freeTimerList.end(); ++timerItor)
	{
		delete *timerItor;
	}
	_freeTimerList.clear();
	
	timerItor = _usedTimerList.begin();
	for (; timerItor != _usedTimerList.end(); ++timerItor)
	{
		delete *timerItor;
	}
	_usedTimerList.clear();
}

NewTimer* TimerPool::newTimer()
{
	MutexLockGuard lock(timerMutex_);
	if (!_freeTimerList.empty())
	{
		NewTimer* pTimer = _freeTimerList.front();
		_usedTimerList.push_back(pTimer);
		_freeTimerList.pop_front();
		return pTimer;
	}
	//todo:��ʱ�������õ�ʱ��Ӧ���䶨ʱ��
	return 0;
}

void TimerPool::delTimer(NewTimer* timer)
{
	MutexLockGuard lock(timerMutex_);
	std::list<NewTimer*>::iterator iter = _usedTimerList.begin();
	
	if (std::find(_usedTimerList.begin(), _usedTimerList.end(), timer) !=_usedTimerList.end())
	{
		_usedTimerList.remove(timer);
		_freeTimerList.push_back(timer);
	}
}


int Slot::AddTimer(NewTimer* timer)
{
	MutexLockGuard lock(mutex_);
	_mapTimer.insert( make_pair(timer->ID(), timer) );
	return 1;
}

bool Slot::DelTimer(NewTimer* timer)
{
	MutexLockGuard lock(mutex_);
	map<UInt32, NewTimer*>::iterator iter = _mapTimer.find(timer->ID());
	if (iter != _mapTimer.end())
	{
		_mapTimer.erase(iter);
		return true;
	}
	return false;
}

//@brief	ʱ�������¼�����
//@return	�ѳ�ʱ��ѭ����ʱ���б�
list<NewTimer*> Slot::OnTick()
{
	MutexLockGuard lock(mutex_);
	int iTimeout = 0;
	list<NewTimer*>	lTimeoutTimerAll;				//�����ѳ�ʱ�Ķ�ʱ��
	list<NewTimer*> lTimeoutTimerLoop;			//�ѳ�ʱ��ѭ����ʱ��
	map<UInt32,NewTimer*>::iterator itMap;

	//��ʱ����
	for( itMap = _mapTimer.begin(); itMap != _mapTimer.end(); itMap++ )
	{
		//��ʱ��ʱ���Ĵ���
		NewTimer* pTimer = itMap->second;
		if (pTimer)
		{
			iTimeout = pTimer->OnTick();
			if(iTimeout)
			{
				//��ʱ�Ķ�ʱ��
				lTimeoutTimerAll.push_back(pTimer);

				//��ʱ��ѭ����ʱ��
				if(NewTimer::LoopTimer == pTimer->Type())
				{
					lTimeoutTimerLoop.push_back(pTimer);
				}
			}
		}
	}

	//�������г�ʱ�Ķ�ʱ��
	std::list<NewTimer*>::iterator itList;
	for( itList = lTimeoutTimerAll.begin(); itList != lTimeoutTimerAll.end(); itList++ )
	{
		itMap = _mapTimer.find((*itList)->ID());
		if( itMap != _mapTimer.end() )
		{
			_mapTimer.erase(itMap);
		}
	}

	//����ѭ����ʱ���б�
	return lTimeoutTimerLoop;	
}

TimerManager::TimerManager():
_lastTimerID(0),
_isStop(0),
_currSlot(0)
{
	pthread_t tickThreadID;
	if( pthread_create(&tickThreadID,NULL, tick_thread_main,this))
	{
		LOG (LOG_ERROR, __FILE__, __LINE__, "create threads occurr error.");
	}

}

TimerManager::~TimerManager()
{
}

void* TimerManager::tick_thread_main(void * p)
{
	TimerManager * tm = static_cast<TimerManager*>(p);
	tm->TickThreadProcess();

	return NULL;
}

//@brief	ʱ������ķ���
void TimerManager::TickThreadProcess()
{
	struct timeval tv;

	while(0 == _isStop)
	{
		tv.tv_sec = TIMERTIMESLOT;
		tv.tv_usec = 0;
		select(0,NULL,NULL,NULL, &tv);

//DEBUG_PRINTF1( "TimerManager::TickThreadProcess().... .... _currSlot[%d] ", _currSlot );
		OnTick();
	}
}

//@brief	TimerManager �Ƿ�ֹͣʱ������
Byte TimerManager::IsStop()
{
	return _isStop;
}


//@brief	ֹͣ TimerManager ʱ������
void TimerManager::Stop()
{
	_isStop = 1;
}

//@brief	ʱ�������¼�����
//	currSlot++
//	������ǰ Slot ��Tick�¼�
void TimerManager::OnTick()
{
	//ʱ���ֲ۵�ʱ�������¼�����
	list<NewTimer*> lRetsetTimer;
	lRetsetTimer = _slots[_currSlot].OnTick();

	//���г�ʱ��ѭ����ʱ��,�ҵ��µ�ʱ���ֲ�
	list<NewTimer*>::iterator it;
	for( it = lRetsetTimer.begin(); it != lRetsetTimer.end(); it++ )
	{
		AttachTimer( *it, 0 );
	}

	//ʱ��ָ����һ��ʱ���ֲ�
	//	������Խ�� MAXTIMERNUM
	if( ++_currSlot >= MAXTIMERNUM )
		_currSlot = 0;
	
}


//@brief	��Ӷ�ʱ��
//@return 0 �ɹ�  ��0  ʧ��
int TimerManager::AddTimer(NewTimer* timer, int isNewTimer)
{
	if (timer == 0)
		return -1;
	//��ʱ����������У��
	if( _lastTimerID >= MAXTIMERNUM )
	{
		LOG (LOG_ERROR, __FILE__, __LINE__, "max timer num limit!!!!!!!");
		return -1;
	}
	
	//��ʱ���У��
	if(timer->_interval <= 0)
	{
		LOG (LOG_ERROR, __FILE__, __LINE__, "error param!, timer._interval[%d]", timer->_interval );
		return -1;
	}

	//�ҽӶ�ʱ��
	AttachTimer(timer,isNewTimer);
	
	return 0;
}

int TimerManager::DelTimer(NewTimer* timer)
{
	for (int i = 0; i< MAXSLOTNUM; i++)
	{
		if (_slots[i].DelTimer(timer))
			break;
	}
	return 1;
}


//@brief	�ҽӶ�ʱ����ָ��slot
//@param	timer	��ʱ��
//@param	isnewTimer	�Ƿ��¶�ʱ�� 0 ��   1 ��
void TimerManager::AttachTimer(NewTimer* timer, int isNewTimer )
{
	//��ʱ��ID
	//	�¶�ʱ��,��Ҫ���� ID
	//	����ʹ�� �ɵ� ID
	if (timer == 0)
		return;
	if(isNewTimer)
		timer->_ID = ++_lastTimerID;
		
//	timer->_maxTimerNum = MAXTIMERNUM;				//���ʱ������
//	timer->_timeSlot = TIMERTIMESLOT;				//ʱ��Ƭʱ�� ��λ ��
	timer->_ticks = timer->_interval/TIMERTIMESLOT;			//ʱ��Ƭ��
	timer->_round = timer->_ticks/MAXSLOTNUM;			//ʱ����Ȧ��
	int slotNum = (_currSlot + (timer->_ticks%MAXSLOTNUM))%MAXSLOTNUM;		//ʱ���ֲۺ�
	timer->_tm = this;
/*
DEBUG_PRINTF( "-----------------------" );
DEBUG_PRINTF1( "timer._ID[%d]", timer._ID );
DEBUG_PRINTF1( "timer._type[%d]", timer._type);
DEBUG_PRINTF1( "timer._maxTimerNum[%d]", timer._maxTimerNum );
DEBUG_PRINTF1( "timer._timeSlot[%d]", timer._timeSlot );
DEBUG_PRINTF1( "timer._ticks[%d]", timer._ticks );
DEBUG_PRINTF1( "timer._round[%d]", timer._round );
DEBUG_PRINTF1( "_currSlot[%d]", _currSlot );
DEBUG_PRINTF1( "timer._slotNum[%d]", timer._slotNum );
DEBUG_PRINTF1( "timer._callBack[%d]", timer._callBack);
DEBUG_PRINTF1( "timer._obj[%d]", timer._obj);
DEBUG_PRINTF1( "timer._arg[%d]", timer._arg);
DEBUG_PRINTF1( "timer._argLen[%d]", timer._argLen);
DEBUG_PRINTF( "========================" );
*/
//LOG(LOG_ERROR,__FILE__,__LINE__,"timer: ID[%d],slotNum[%d]", timer._ID, timer._slotNum);

	//ʱ���ֲ���Ӷ�ʱ��
	_slots[slotNum].AddTimer(timer);
}

