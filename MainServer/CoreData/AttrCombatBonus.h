//		�ӳ�ս��������

#ifndef ATTRCOMBATBONUS_H
#define ATTRCOMBATBONUS_H

#include "OurDef.h"


class AttrCombatBonus
{
public:
	friend class Creature;

public:
	AttrCombatBonus()
	:_maxHpBonus(0),
	_maxMpBonus(0),
	_hpRegenBonus(0),
	_mpRegenBonus(0),
	_attackPowerHighBonus(0),
	_attackPowerLowBonus(0),
	_attackScopeBonus(0),
	_attackSpeedBonus(0),
	_bulletSpeedBonus(0),
	_defenceBonus(0),
	_mDefenceBonus(0),
	_critRateBonus(0),
	_hitRateBonus(0),
	_dodgeRateBonus(0),
	_strengthBonus(0),
	_intelligenceBonus(0),
	_agilityBonus(0),
	_movespeedBonus(0)
	{}
	~AttrCombatBonus(){}


private:
	//�ӳɺ�����
	UInt32 _maxHpBonus;

	//�ӳ�������
	UInt32 _maxMpBonus;

	//�ӳɻغ��ٶ�
	UInt32 _hpRegenBonus;

	//�ӳɻ����ٶ�
	UInt32 _mpRegenBonus;

	//�ӳɹ���������
	UInt32 _attackPowerHighBonus;

	//�ӳɹ���������
	UInt32 _attackPowerLowBonus;

	//�ӳɹ�����Χ
	UInt32 _attackScopeBonus;

	//�ӳɹ����ٶ�
	UInt32 _attackSpeedBonus;

	//�ӳ��ӵ��ٶ�
	UInt32 _bulletSpeedBonus;

	//�ӳɷ�����
	UInt32 _defenceBonus;

	//�ӳ�ħ��,ħ��������
	UInt32 _mDefenceBonus;

	//�ӳɱ�����
	UInt32 _critRateBonus;

	//�ӳ�������
	UInt32 _hitRateBonus;

	//�ӳ�������
	UInt32 _dodgeRateBonus;
	//�����ӳ�
	UInt32 _strengthBonus;
	//�����ӳ�
	UInt32 _intelligenceBonus;
	//���ݼӳ�
	UInt32 _agilityBonus;
	//�ٶȼӳ�
	UInt32 _movespeedBonus;
	
};


#endif

