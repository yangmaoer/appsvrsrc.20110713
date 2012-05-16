/**
 *	Friend ���л��õ���
 *	
 */

#ifndef ARCHVFRIEND_H
#define ARCHVFRIEND_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "DebugData.h"
#include "List.h"


//
class ArchvFriend
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvFriend():friendType(0),IsOnline(0)
	{}
	
public:
	//��Ա����
	string friendName;
	UInt32 friendRoleID;
	Byte friendType; // 1 ���ѣ�2 ���ˣ�3 ������
	Byte IsOnline;   //�Ƿ����� 1 �ǣ�0 ��

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
	SERIAL_ENTRY(friendName)
	SERIAL_ENTRY(friendRoleID)
	SERIAL_ENTRY(friendType)
	SERIAL_ENTRY(IsOnline)
	END_SERIAL_MAP()

};
class ArchvFriendAddPro
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvFriendAddPro():friendType(0),IsOnline(0),ProID(0)
	{}
	
public:
	//��Ա����
	string friendName;
	UInt32 friendRoleID;
	Byte friendType; // 1 ���ѣ�2 ���ˣ�3 ������
	Byte IsOnline;   //�Ƿ����� 1 �ǣ�0 ��
	Byte ProID;

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
	SERIAL_ENTRY(friendName)
	SERIAL_ENTRY(friendRoleID)
	SERIAL_ENTRY(friendType)
	SERIAL_ENTRY(IsOnline)
	SERIAL_ENTRY(ProID)
	END_SERIAL_MAP()

};


#endif

