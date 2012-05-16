/**
 *	Pet ���л��õ���
 *	
 */

#ifndef ARCHVPET_H
#define ARCHVPET_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "DebugData.h"
#include "List.h"


//���� 
class PetBrief
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	PetBrief()
	:petId(0),petType(0),petLevel(0)
	{}
	
public:
	//��Ա����
	UInt32 petId;
	UInt16 petType;
	string petName;
	UInt32 petLevel;

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
	SERIAL_ENTRY(petId)
	SERIAL_ENTRY(petType)
	SERIAL_ENTRY(petName)
	SERIAL_ENTRY(petLevel)
	END_SERIAL_MAP()

};


class PetEquip
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	PetEquip():EquipIndex(0),ItemID(0),EntityID(0),Durability(0),BindState(0)
	{}
	
public:
	//��Ա����
	Byte EquipIndex;   //װ�����
	UInt32 ItemID;     //��ƷID
	UInt32 EntityID;   //ʵ��ID
	UInt16 Durability;  //�;ö�
	Byte BindState;    //��״̬

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
	SERIAL_ENTRY(EquipIndex)
	SERIAL_ENTRY(ItemID)
	SERIAL_ENTRY(EntityID)
	SERIAL_ENTRY(Durability)
	SERIAL_ENTRY(BindState)
	END_SERIAL_MAP()

};




class PetBonus
	:public BaseArchive
{
public:
	PetBonus():MPRegen(0), HPRegen(0), MaxHP(0), MaxMP(0), MoveSpeed(0), Agility(0),
		Intelligence(0), Strength(0), attackPower(0), 
		AttackSpeed(0), Defence(0), MDefence(0), CritRate(0) ,HitRate(0),DodgeRate(0)
		{}
public:

	UInt32 MaxHP;
	UInt32 MaxMP;
	UInt32 attackPower;
	UInt32 Defence;
	UInt32 MDefence;
	UInt32 CritRate;
	UInt32 Strength;
	UInt32 Intelligence;
	UInt32 Agility;	
	UInt32 MoveSpeed;
	UInt32 HitRate;
	UInt32 DodgeRate;
	UInt32 AttackSpeed;
	UInt32 HPRegen;
	UInt32 MPRegen;
	
	
	BEGIN_SERIAL_MAP()
	SERIAL_ENTRY(MaxHP)
	SERIAL_ENTRY(MaxMP)
	SERIAL_ENTRY(attackPower)
	SERIAL_ENTRY(Defence)
	SERIAL_ENTRY(MDefence)
	SERIAL_ENTRY(CritRate)
	SERIAL_ENTRY(Strength)
	SERIAL_ENTRY(Intelligence)
	SERIAL_ENTRY(Agility)
	SERIAL_ENTRY(MoveSpeed)
	SERIAL_ENTRY(HitRate)
	SERIAL_ENTRY(DodgeRate)
	SERIAL_ENTRY(AttackSpeed)
	SERIAL_ENTRY(HPRegen)
	SERIAL_ENTRY(MPRegen)
	END_SERIAL_MAP()
	
};

class PetInfo
	:public BaseArchive
{
public:
	PetInfo()
	:  Exp(0), MaxExp(0), AddPoint(0),
	Strength(0), Intelligence(0),
	Agility(0), MoveSpeed(0), HP(0), MP(0), MaxHP(0), MaxMP(0), HPRegen(0), MPRegen(0),
	AttackPowerHigh(), AttackPowerLow(),
	  AttackSpeed(0), Defence(0), MDefence(0), CritRate(0), HitRate(0),
	DodgeRate(0), Level(0){}
public:
	
	UInt32 Exp     ;                        //����ֵ
	UInt32 MaxExp   ;                       //�õȼ������
	UInt32 HP       ;                       //����ֵ����
	UInt32 MP       ;                       //ħ��ֵ����
	UInt32 MaxHP     ;                      //����
	UInt32 MaxMP    ;                       //�����
	UInt32 AttackPowerHigh  ;               //��������������
	UInt32 AttackPowerLow  ;                //��������������
	UInt32 Defence    ;                     //�����������������
	UInt32 MDefence   ;                     //ħ����ħ�������� 
	UInt32 CritRate   ;                     //������
	UInt32 AddPoint ;                       //�������Ե�
	UInt32 Strength  ;                      //����
	UInt32 Intelligence   ;                //֮��
	UInt32 Agility    ;                     //���ݶ�
	UInt32 MoveSpeed   ;                    //�ƶ��ٶ�
	UInt32 HitRate    ;                     //������
	UInt32 DodgeRate  ;                      //������
	UInt32 AttackSpeed   ;                  //�����ٶ�
	UInt32 HPRegen  ;                       //�غ��ٶ�
	UInt32 MPRegen   ;                      //�����ٶ�
	UInt32 Level;                           //����
	string PetName;                      //����

	BEGIN_SERIAL_MAP()
	SERIAL_ENTRY(Exp)
	SERIAL_ENTRY(MaxExp)
	SERIAL_ENTRY(HP)
	SERIAL_ENTRY(MP)
	SERIAL_ENTRY(MaxHP)
	SERIAL_ENTRY(MaxMP)
	SERIAL_ENTRY(AttackPowerHigh)
	SERIAL_ENTRY(AttackPowerLow)
	SERIAL_ENTRY(Defence)
	SERIAL_ENTRY(MDefence)
	SERIAL_ENTRY(CritRate)
	SERIAL_ENTRY(AddPoint)
	SERIAL_ENTRY(Strength)
	SERIAL_ENTRY(Intelligence)
	SERIAL_ENTRY(Agility)
	SERIAL_ENTRY(MoveSpeed)
	SERIAL_ENTRY(HitRate)
	SERIAL_ENTRY(DodgeRate)
	SERIAL_ENTRY(AttackSpeed)
	SERIAL_ENTRY(HPRegen)
	SERIAL_ENTRY(MPRegen)
	SERIAL_ENTRY(Level)
	SERIAL_ENTRY(PetName)
	END_SERIAL_MAP()

	
};

class PetbrifInfo
	:public BaseArchive
{
	public:    
	PetbrifInfo():petID(0),PetType(0),Level(0),IsOut(0)
	{}
	public:
		UInt32 petID;
		UInt32 PetType;
		string PetName;
		UInt32 Level;
		Byte IsOut;
	public:
		BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(petID)
		SERIAL_ENTRY(PetType)
		SERIAL_ENTRY(PetName)
		SERIAL_ENTRY(Level)
		SERIAL_ENTRY(IsOut)
		END_SERIAL_MAP()
};

#endif

