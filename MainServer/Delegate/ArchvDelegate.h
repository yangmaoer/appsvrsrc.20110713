/**
 *	Delegate ���л��õ���
 *	
 */

#ifndef ARCHVDELEGATE_H
#define ARCHVDELEGATE_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "DebugData.h"
#include "List.h"


class ArchvRoleDelegateInfo
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvRoleDelegateInfo()
	:taskID(0),maxDelegateNum(0),delegateTime(0),
	delegateCost(0),isStarted(0),finishNum(0),elapseTime(0)
	{}
	
public:
	//��Ա����
	UInt32 taskID;
	UInt16 maxDelegateNum;
	UInt32 delegateTime;
	UInt32 delegateCost;
	Byte	 isStarted;
	UInt16 finishNum;
	UInt32 elapseTime;
	
//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
	SERIAL_ENTRY(taskID)
	SERIAL_ENTRY(maxDelegateNum)
	SERIAL_ENTRY(delegateTime)
	SERIAL_ENTRY(delegateCost)
	SERIAL_ENTRY(isStarted)
	SERIAL_ENTRY(finishNum)
	SERIAL_ENTRY(elapseTime)
	END_SERIAL_MAP()

};



#endif

