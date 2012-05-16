//		һ��������
#ifndef ATTRGENERAL_H
#define ATTRGENERAL_H

#include "OurDef.h"
#include "ArchvMap.h"
#include <string>


using namespace std;

class AttrGeneral
{
public:
	friend class Creature;
	
public:
	AttrGeneral()
	:_ID(0),
	_flag(0),
	_type(0),
	_camp(0),
	_mapID(0),
	_status(0),					
	_level(0),
	_exp(0),
	_maxExp(0),
	_addPoint(0),
	_strength(0),
	_intelligence(0),
	_agility(0),
	_moveSpeed(541)
	{
		_direct = GetRandDirect();
		_statusChangeTime = time(NULL);
	}

	~AttrGeneral()
	{
	}

	static Byte GetRandDirect()
	{
		UInt32 uiTime = time(NULL);
		srand( uiTime );
		return (rand()%(MAXROLEDIRECT)) + 1;
	}

	//--------------ҵ����-----------

private:
	//ID
	UInt32 _ID;

	//����
	string _name;

	//�����־	1 ��ɫ  2 ��   3 npc  4 ����
	Byte _flag;

	//��������	��ɫ��proID ���߹�Type ���� NpcType  ���߳���Type
	UInt32 _type;

	//��Ӫ  1 ���� 2 ����  3 ���� 4 ȫ�жԹ�ϵ  5 ȫ��ƽ
	//
	Byte	_camp;

	//��ͼID 0 ��ʾδ�����ͼ
	UInt32 _mapID;

	//�ƶ�·��
	ArchvRoute  _route;

	//��ǰ����
	Byte _direct;

	//��ǰλ��
	ArchvPosition _pos;

	//����״̬	//	0 �� 1 ����δս��  2 ս��
	Byte _status;

	//����״̬���ʱ��
	//	��¼���һ������״̬���ʱ��
	UInt32 _statusChangeTime;

	//��Ϸ����
	UInt32 _level;

	//����
	UInt32 _exp;

	//�����ֵ
	UInt32 _maxExp;

	//�������Ե�
	UInt32 _addPoint;

	//����
	UInt32 _strength;

	//����
	UInt32 _intelligence;

	//����
	UInt32 _agility;

	//�ƶ��ٶ�, ��ֹ20101225û��ʹ��,����
	UInt32 _moveSpeed;

};


#endif

