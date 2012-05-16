/**
 *	��ͼ ���л��õ���
 *	
 */

#ifndef ARCHVMAP_H
#define ARCHVMAP_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "DebugData.h"
#include "List.h"


//����
class ArchvPosition
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvPosition():X(0), Y(0)
	{}

	ArchvPosition(UInt16 inputX, UInt16 inputY)
	:X(inputX), Y(inputY)
	{}
 
public:
	//��Ա����
	UInt16	X;
	UInt16	Y;

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(X)
		SERIAL_ENTRY(Y)
	END_SERIAL_MAP()

};

//��ɫ�ƶ�·��
class ArchvRoute
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvRoute():time(0)
	{
	}

	ArchvRoute(UInt32 uTime, ArchvPosition &pos)
	:time(uTime)
	{
		listPos.push_back(pos);
	}
	
public:
	//��Ա����
	UInt32	time;							//�ƶ�·����ʼʱ��
	List<ArchvPosition> listPos;		//�û��ƶ����꼯��
	
//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(time)
		SERIAL_ENTRY(listPos)
	END_SERIAL_MAP()

};

//��ɫ�ƶ�����
class ArchvRoleMoveDesc
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvRoleMoveDesc():roleID(0), speed(0)
	{
	}
	
public:
	//��Ա����
	UInt32				roleID;
	UInt32				speed;				//�ƶ��ٶ�
	ArchvRoute		route;				//�û��ƶ�·��
	
	
//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(roleID)
		SERIAL_ENTRY(speed)
		SERIAL_ENTRY(route)
	END_SERIAL_MAP()

};



//��������(��ɫ���֡�����)״̬����
class ArchvCreatureStatus
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvCreatureStatus():creatureFlag(0), creatureType(0), ID(0), status(0)
	{
	}
	
public:
	//��Ա����
	Byte 					creatureFlag;
	UInt32 				creatureType;
	UInt32				ID;						//ID����ɫID����ID
	Byte					status;				//״̬��0 �� 1 ����δս�� 2 ս��
	
	
//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(creatureFlag)
		SERIAL_ENTRY(creatureType)
		SERIAL_ENTRY(ID)
		SERIAL_ENTRY(status)
	END_SERIAL_MAP()

};

class SenceMonster
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	SenceMonster(): MonsterType(0), num(0)
	{
	}
	
public:
	UInt32 MonsterType;
	UInt16 num;//1���,2ս���У�4����
	


};


class SenceMonsterAll
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	SenceMonsterAll():X(0), Y(0), MonsterID(0), MonsterType(0),Status(0),dieTime(0)
	{
	}
	
public:
	UInt16 X;
	UInt16 Y;
	UInt32 MonsterID;
	UInt32 MonsterType;
	Byte Status;//1���,2ս���У�4����
	UInt32 dieTime;
	list<SenceMonster> Monsterp;
	
	


};



#endif
