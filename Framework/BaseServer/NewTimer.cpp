#include "NewTimer.h"
#include <sys/select.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include "DebugData.h"
#include <string.h>
#include "Log.h"
#include "TimerManager.h"

using namespace std;

SelfDeleteCallback::SelfDeleteCallback(TIMER_CALLBACK_FUN callBack, void * obj, void* arg, int argLen ):
_callBack(callBack),
_obj(obj),
_argLen(argLen)
{
	memcpy( _arg, arg, _argLen );
}
SelfDeleteCallback::~SelfDeleteCallback()
{
}

void SelfDeleteCallback::NewThreadCallBack()
{
	pthread_t	threadID;
	int iRet = 0;
	
	//���Լ���,ȷ���߳�����
	for(int i = 0; i<5; i++)
	{
		usleep(1000);
		iRet = pthread_create(&threadID, NULL, detach_thread_main, this);
		if(0==iRet)
			break;
	}
	if(iRet)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"SelfDeleteCallback pthread_create error!!") ;
	}

	//�����߳�
	iRet = pthread_detach(threadID);
	if(iRet)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"SelfDeleteCallback pthread_detach error!!") ;
	}
}

void * SelfDeleteCallback::detach_thread_main(void * p)
{
	(static_cast<SelfDeleteCallback*>(p))->DetachProcess();
}

//@brief	�����̵߳Ĵ����߼�
//				ִ�лص�����
void SelfDeleteCallback::DetachProcess()
{
//	DEBUG_PRINTF4( "..............DetachProcess........._callBack[%d], _obj[%d], _arg[%d], _argLen[%d] ",
//		_callBack, _obj, _arg, _argLen );
	
	
	if( NULL != _callBack && NULL != _obj )
		(*_callBack)( _obj, _arg, _argLen );

	//������ɺ�,���� delete
	delete this;
}



NewTimer::NewTimer():
_callBack(NULL),
_obj(NULL),
_argLen(0),
_ID(0),
//_maxTimerNum(0),
//_timeSlot(0),
_interval(0),
_ticks(0),
_round(0),
//_slotNum(0),
_type(OnceTimer),
_tm(0)
{	
}

NewTimer::~NewTimer()
{
}

//@brief	���ûص�����
//@param	fun	�ص�����
//@param	obj	�����ַ
//@param	arg	�ص������Ĳ�����ַ
//@param	argLen	�ص������Ĳ�������
//@return	0 �ɹ�  ��0 ʧ��
int NewTimer::SetCallbackFun( TIMER_CALLBACK_FUN fun, void * obj,  void * arg, int argLen )
{
	_callBack = fun;
 	_obj = obj;
 	
 	//�ص������������
	if( argLen < 0 || argLen > MAXARGLEN )
	{
		return -1;
	}
	else
	{
		_argLen = argLen;
		memcpy( _arg, arg, _argLen );
	}

	return 0;
}

void NewTimer::Interval(int input)
{
	_interval = input;
}

UInt32 NewTimer::ID()
{
	return _ID;
}

NewTimer::TimerType NewTimer::Type()
{
	return _type;
}

void NewTimer::Type(TimerType input)
{
	_type = input;
}

bool NewTimer::cancel()
{
	//todo:������ڴ������Ƿ����cancel?
	if (_tm)
	{
		bool bCancel = _tm->DelTimer(this);
		if (bCancel)
		{
			_tm = 0;
		}
		
		return bCancel;
	}
	return true;
}

bool NewTimer::start(TimerManager* tm)
{
	if (tm)
	{
		int bNewTimer = (ID() == 0);
		tm->AddTimer(this, bNewTimer);
	}
}

bool NewTimer::reStart()
{
	if (_tm)
	{
		bool bCancel = _tm->DelTimer(this);
		if (bCancel)
		{
			return _tm->AddTimer(this, 0);
		}
	}
	return false;
}


//@brief	ʱ���¼�����
//@return ��ʱ���Ƿ�ʱ	0 ��	��0 �ѳ�ʱ
int NewTimer::OnTick()
{
	int iTimeout = 0;

	//ʱ����Ȧ�� Ϊ0 ������ʱ����,
	if(--_round <= 0)
	{
		iTimeout = 1;
		OnTimeout();
	}
	
	return iTimeout;
}


//@brief	��ʱ����ʱ����
void NewTimer::OnTimeout()
{
	SelfDeleteCallback * callBack = new SelfDeleteCallback(_callBack, _obj, _arg, _argLen);
	callBack->NewThreadCallBack();
	_tm = 0;
}


