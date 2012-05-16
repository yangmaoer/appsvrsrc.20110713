/**
 *	���� ���л��õ���
 *	
 */

#ifndef ARCHVTRADE_H
#define ARCHVTRADE_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "DebugData.h"
#include "List.h"
#include "ArchvBagItemCell.h"

//��Ҫ������Ϣ
class ArchvTrade
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvTrade():RoleID(0),Money(0),Gold(0)
	{}
	
public:
	//��Ա����
	UInt32	RoleID;
	UInt32 Money;
	UInt32 Gold;
	List<UInt16> lic;

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(RoleID)
		SERIAL_ENTRY(Money)
		SERIAL_ENTRY(Gold)
		SERIAL_ENTRY(lic)		
	END_SERIAL_MAP()

};

class TradeItem
	  :public BaseArchive
{
 public:
  	
  TradeItem():isOnTrade(0),tradeRoleID(0),isLockTrade(0),isTrade(0),tradeMoney(0),tradeGold(0)
  {}

  ~TradeItem()
  {}

 public:
  	
  Byte         isOnTrade;   //��ɫ�Ƿ����ڽ��� 0 ���ڽ��ף� 1 ���ڽ���
  UInt32       tradeRoleID; //���׵Ľ�ɫ(��˭����)
  Byte         isLockTrade; //�Ƿ���������  0 δ����� 1 ����
  Byte         isTrade;     //�Ƿ����˽��� 0 δ���ף�1 ����
  UInt32       tradeMoney;  //���׵ķǰ�����
  UInt32       tradeGold;   //���׵Ľ��
  List<UInt16> tradeCellIndex; //��ɫ���׵ı�����Ԫ��λ��

 public:
 	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(isOnTrade)
		SERIAL_ENTRY(tradeRoleID)
		SERIAL_ENTRY(isLockTrade)
		SERIAL_ENTRY(isTrade)
		SERIAL_ENTRY(tradeMoney)
		SERIAL_ENTRY(tradeGold)
		SERIAL_ENTRY(tradeCellIndex)
	END_SERIAL_MAP()
  
};


#endif

