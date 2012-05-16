#ifndef ARCHVSHOP_H
#define ARCHVSHOP_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "DebugData.h"
#include "List.h"


//��Ҫ������Ϣ
class ArchvShopItem
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvShopItem():ItemID(0),Category(0),NowPrice(0),Num(0)
	{}
	
public:
	//��Ա����
	UInt32	ItemID;
	Byte		Category;
	UInt16		NowPrice;
	UInt16      Num;

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(ItemID)
		SERIAL_ENTRY(Category)
		SERIAL_ENTRY(NowPrice)
		SERIAL_ENTRY(Num)
		
	END_SERIAL_MAP()

};

class SpecialItem 
	:public BaseArchive

{
	public:
		SpecialItem():ItemID(0),NowPrice(0),Num(0)
			{}
	public:
		UInt32 ItemID;
		UInt16 NowPrice;
		UInt16 Num;
		
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(ItemID)
		SERIAL_ENTRY(NowPrice)
		SERIAL_ENTRY(Num)
		
	END_SERIAL_MAP()
};





#endif

