/**
 *	���� ���л��õ���
 *	
 */

#ifndef DBBAG_H
#define DBBAG_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "DebugData.h"
#include "List.h"


//������Ʒ��Ϣ
class DBBagItem:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	DBBagItem():RoleID(0),CellIndex(0),CellType(0),ItemType(0),ItemID(0),EntityID(0),NUM(0)
	{}
	
	virtual ~DBBagItem(){}

public:
	//��Ա����
	UInt32 RoleID;
	UInt16 CellIndex;
	UInt16 CellType;
	UInt16 ItemType;
	UInt32 ItemID;
	UInt32 	EntityID;
	UInt32 NUM;
	
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(RoleID)
		SERIAL_ENTRY(CellIndex)
		SERIAL_ENTRY(CellType)
		SERIAL_ENTRY(ItemID)
		SERIAL_ENTRY(EntityID)
		SERIAL_ENTRY(NUM)
	END_SERIAL_MAP()
	

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�

};

class ItemMapTravelPoint
{
	public:
		ItemMapTravelPoint():MapID(0),X(0),Y(0)
		{}
	public:
		UInt32 MapID;
		UInt16 X;
		UInt16 Y;
		
};
	
class EquipItem
{
	public:
		EquipItem():ItemID(0),EntityID(0)
				{}
	public:
		UInt32 ItemID;
		UInt32 EntityID;
};


//װ���;öȱ仯(װ�����)
class ArchvEquipDurability:public BaseArchive
{

  public:
  	 ArchvEquipDurability():EquipIndex(0),ItemID(0),EntityID(0),Durability(0)
  	 {}

  	virtual ~ArchvEquipDurability(){}
	 
  public:
  	 Byte   EquipIndex;
	 UInt32 ItemID;
	 UInt32 EntityID;
	 UInt32 Durability;


	 BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(EquipIndex)
		SERIAL_ENTRY(ItemID)
		SERIAL_ENTRY(EntityID)
		SERIAL_ENTRY(Durability)
	END_SERIAL_MAP()
};

#endif
