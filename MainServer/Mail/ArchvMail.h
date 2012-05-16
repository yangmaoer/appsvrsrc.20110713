/**
 *	Mail ���л��õ���
 *	
 */

#ifndef ARCHVMAIL_H
#define ARCHVMAIL_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "DebugData.h"
#include "List.h"


//


class ArchvMailQueryItem
	  :public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvMailQueryItem()
	:MailID(0),IsRead(0),MailType(2),\
	Money(0),ItemID(0),Num(0),Durability(0)
	{}
	
public:
	//��Ա����
   UInt32	MailID;			//�ʼ�ID
   Byte     MailType;       //�ʼ����� 1 ϵͳ�ʼ���2 �û��ʼ�
   Byte     IsRead;         //�Ƿ��Ķ�  
   string   SendRoleName;	//������RoleID
   string	SendTime;		//����ʱ��
   string   Content;	    //�ʼ�����
   UInt32   Money;		    //������Ǯ��
   Byte     MoneyType;      //��Ǯ����
   UInt32   ItemID;			//��Ʒid
   UInt16   Num;			//��Ʒ����
   UInt16   Durability;		//�;ö�
  

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
	SERIAL_ENTRY(MailID)
	SERIAL_ENTRY(MailType)
	SERIAL_ENTRY(IsRead)
	SERIAL_ENTRY(SendRoleName)
	SERIAL_ENTRY(SendTime)
	SERIAL_ENTRY(Content)
	SERIAL_ENTRY(Money)
	SERIAL_ENTRY(MoneyType)
	SERIAL_ENTRY(ItemID)
	SERIAL_ENTRY(Num)
	SERIAL_ENTRY(Durability)
	END_SERIAL_MAP()

};

class ArchvSystemMailItem
	  :public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvSystemMailItem()
	:Money(0),MoneyType(0),ItemID(0),Num(0),EntityID(0)
	{}
	
public:
	//��Ա����
   string   RecvRoleName;   //�ռ�������
   string   Content;        //����
   UInt32   Money;		    //������Ǯ��
   Byte     MoneyType;      //��Ǯ���� 1 ���ң� 2 ���
   UInt32   ItemID;			//��Ʒid
   UInt16   Num;			//��Ʒ����
   UInt32   EntityID;       //ʵ��ID
  

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
	SERIAL_ENTRY(RecvRoleName)
	SERIAL_ENTRY(Content)
	SERIAL_ENTRY(Money)
	SERIAL_ENTRY(MoneyType)
	SERIAL_ENTRY(ItemID)
	SERIAL_ENTRY(Num)
	SERIAL_ENTRY(EntityID)
	END_SERIAL_MAP()

};

class ArchvMailItem
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvMailItem()
	:mailID(0),recvRoleID(0),sendRoleID(0),
	money(0),itemID(0),num(0),entityID(0)
	{}
	
public:
	//��Ա����
	UInt32 mailID;
	UInt32 recvRoleID;
	UInt32 sendRoleID;
	string sendTime;
	UInt32 money;
	UInt16 cellIndex;
	UInt32 itemID;
	UInt32 num;
	UInt32 entityID;
	string Content;

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
	SERIAL_ENTRY(mailID)
	SERIAL_ENTRY(recvRoleID)
	SERIAL_ENTRY(sendRoleID)
	SERIAL_ENTRY(sendTime)
	SERIAL_ENTRY(money)
	SERIAL_ENTRY(cellIndex)
	SERIAL_ENTRY(itemID)
	SERIAL_ENTRY(num)
	SERIAL_ENTRY(entityID)
	SERIAL_ENTRY(Content)
	END_SERIAL_MAP()

};




#endif

