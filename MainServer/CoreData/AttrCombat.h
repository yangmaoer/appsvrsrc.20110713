//		����ս��������

#ifndef ATTRCOMBAT_H
#define ATTRCOMBAT_H

#include "OurDef.h"

class AttrCombat
{
public:
	friend class Creature;
	
public:
	AttrCombat()
	:_hp(0),
	_mp(0),
	_maxHp(0),
	_maxMp(0),
	_hpRegen(0),
	_mpRegen(0),
	_attackPowerHigh(0),
	_attackPowerLow(0),
	_attackScope(0),
	_attackSpeed(0),
	_bulletSpeed(0),
	_defence(0),
	_mDefence(0),
	_critRate(0),
	_hitRate(0),
	_dodgeRate(0)
	{}
	~AttrCombat(){}

private:
	//��
	UInt32 _hp;

	//��
	UInt32 _mp;

	//������
	UInt32 _maxHp;

	//������
	UInt32 _maxMp;

	//�غ��ٶ�
	UInt32 _hpRegen;

	//�����ٶ�
	UInt32 _mpRegen;

	//����������
	UInt32 _attackPowerHigh;

	//����������
	UInt32 _attackPowerLow;

	//������Χ
	UInt32 _attackScope;

	//�����ٶ�
	UInt32 _attackSpeed;

	//�ӵ��ٶ�
	UInt32 _bulletSpeed;

	//������
	UInt32 _defence;

	//ħ��,ħ��������
	UInt32 _mDefence;

	//������
	UInt32 _critRate;

	//������
	UInt32 _hitRate;

	//������
	UInt32 _dodgeRate;
	
};


#endif

