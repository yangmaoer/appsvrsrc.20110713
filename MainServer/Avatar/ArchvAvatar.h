/**
 *	��װ ���л��õ���
 *	
 */

#ifndef ARCHVAVATAR_H
#define ARCHVAVATAR_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "DebugData.h"
#include "List.h"



//avatar��Ϣ
class ArchvAvatarDesc
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvAvatarDesc()
	:roleId(0),equipIndex(0),itemID(0),entityID(0),durability(0),bindStatus(0)
	{}	
public:
	//��Ա����
	UInt32 roleId;
	Byte equipIndex;
	UInt32 itemID;
	UInt32 entityID;
	UInt16 durability;
	Byte bindStatus;

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(roleId)
		SERIAL_ENTRY(equipIndex)
		SERIAL_ENTRY(itemID)
		SERIAL_ENTRY(entityID)
		SERIAL_ENTRY(durability)
		SERIAL_ENTRY(bindStatus)
	END_SERIAL_MAP()

};

//��Ҫavatar��Ϣ
class ArchvAvatarDescBrief
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvAvatarDescBrief()
	:roleId(0),proID(0),wpnItemID(0)
	,coatID(0)
	{}
	
public:
	//��Ա����
	UInt32 roleId;
	Byte proID;
	UInt32 wpnItemID;		//���� ItemID
	UInt32 coatID;					//

	
//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(roleId)
		SERIAL_ENTRY(proID)
		SERIAL_ENTRY(wpnItemID)
		SERIAL_ENTRY(coatID)
	END_SERIAL_MAP()

};
class ArchvroleEuip
	:public BaseArchive
{
	public:

	ArchvroleEuip()
		:BonusAttrID(0),Type(0),Value(0)
		{}
	public:
		UInt16 BonusAttrID;
		UInt16 Type;
		UInt32 Value;
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(BonusAttrID)
		SERIAL_ENTRY(Type)
		SERIAL_ENTRY(Value)
	END_SERIAL_MAP()
		
	
};
class ArchvroleBonuschange
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvroleBonuschange()
	:BonusAttrID(0),Num(0)
	{}
	
public:
	//��Ա����
	UInt16 BonusAttrID;
	UInt32 Num;

	
//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(BonusAttrID)
		SERIAL_ENTRY(Num)
	END_SERIAL_MAP()

};



#endif

