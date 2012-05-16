/**
 *	ItemCell
 *	
 */

#ifndef ARCHVBAGITEM_H
#define ARCHVBAGITEM_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "DebugData.h"
#include "List.h"


//������Ʒ��Ϣ
class ItemCell
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ItemCell():celIndex(0),ItemID(0),EntityID(0),cdTime(0),num(0),durability(0),bindStatus(0)
	{}
	
public:
	//��Ա����
	UInt16 celIndex;
	UInt32 ItemID;
	UInt32 	EntityID;
	UInt16 cdTime;
	UInt16 num;
	UInt16 durability;
	Byte bindStatus;
	

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(celIndex)
		SERIAL_ENTRY(ItemID)
		SERIAL_ENTRY(EntityID)
		SERIAL_ENTRY(cdTime)
		SERIAL_ENTRY(num)
		SERIAL_ENTRY(durability)	
		SERIAL_ENTRY(bindStatus)
	END_SERIAL_MAP()

};
class ItemList
	:public BaseArchive
{
	public:
		//��Ա��������ֵ
		ItemList():ItemID(0),num(0)
		{}
		
	public:
		//��Ա����
		UInt32 ItemID;
		UInt16 num;
	//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
		BEGIN_SERIAL_MAP()
			SERIAL_ENTRY(ItemID)
			SERIAL_ENTRY(num)
		END_SERIAL_MAP()

};




#endif
