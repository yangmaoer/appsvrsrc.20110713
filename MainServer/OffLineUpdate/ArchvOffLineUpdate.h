/**
 *	OffLineUpdate ���л��õ���
 *	
 */

#ifndef ARCHVOFFLINEUPDATE_H
#define ARCHVOFFLINEUPDATE_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "DebugData.h"
#include "List.h"


/*
class ArchvOffLineUpdateItem
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvOffLineUpdateItem()
	:UpdateID(0),RoleID(0),IsFinish(0),MoneyType(0),LockMoney(0),
	BegTime(0),DesSec(0),DesExp(0),DesLevel(0),DesNum(0),
	BilledNum(0),BilledMoney(0),FinishedSec(0),SpeedUpSec(0)
	{}
	
public:
	//��Ա����
	UInt32 UpdateID;
	UInt32 RoleID;
	string BegDate;
	Byte   IsFinish;
	Byte   MoneyType;
	UInt32 LockMoney;
	UInt32 BegTime;
	UInt32 DesSec;
	UInt32 DesExp;
	UInt32 DesLevel;
	UInt32 DesNum;
	UInt32 BilledNum;
	double BilledMoney;
	UInt32 FinishedSec;
	UInt32 SpeedUpSec;
	
//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
	SERIAL_ENTRY(UpdateID)
	SERIAL_ENTRY(RoleID)
	SERIAL_ENTRY(BegDate)
	SERIAL_ENTRY(IsFinish)
	SERIAL_ENTRY(MoneyType)
	SERIAL_ENTRY(LockMoney)
	SERIAL_ENTRY(BegTime)
	SERIAL_ENTRY(DesSec)
	SERIAL_ENTRY(DesExp)
	SERIAL_ENTRY(DesLevel)
	SERIAL_ENTRY(DesNum)
	SERIAL_ENTRY(BilledNum)
	SERIAL_ENTRY(BilledMoney)
	SERIAL_ENTRY(FinishedSec)
	SERIAL_ENTRY(SpeedUpSec)	
	END_SERIAL_MAP()

};

*/


#endif

