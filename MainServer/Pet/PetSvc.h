#ifndef PETSVC_H
#define PETSVC_H
#include "GWProxy.h"
#include "ConnectionPool.h"

#include "Serializer.h"
#include "../Avatar/ArchvAvatar.h"
#include "ArchvPK.h"
#include "ArchvBagItemCell.h"
#include "ArchvPet.h"
#include "ArchvRole.h"
#include "Pet.h"



#ifndef PET_MAX_LEVEL
#define PET_MAX_LEVEL 70   //�������ȼ�
#endif

#ifndef PET_MAX_NUM 
#define PET_MAX_NUM 6
#endif


class PetSvc
{
public:
	//���캯��	
	PetSvc(void *sever,ConnectionPool *cp);
	
	//��������
	~PetSvc();

	void OnProcessPacket(Session& session,Packet& packet);
	//�����
	void ClientErrorAck(Session& session, Packet& packet, UInt32 RetCode);
	
	void ProcessPacket(Session& session, Packet& packet);

	//�ͻ��˴���Ӧ��s
	void ClientErrorAck(Session& session, Packet& packet);


	//========================== c-s ack ===================================

	//[msgtype:1002] ��ѯ������ϸ��Ϣ;
	void ProcessPetDetailInfo(Session& session, Packet& packet);

	//[mystype:1005]����ж��װ�� 
	void ProcessPetEquipGetOff(Session& session, Packet& packet);

	//[mystype:1006] ���ﴩ��װ��
	void ProcessPetEquipPutOn(Session& session, Packet& packet);

	//����ϳ� ������0 �ɹ��� ��0 ʧ��
	int OnPetComposite(UInt32 &roleID, UInt32 &petType);
	
	//���ﶪ�������� 0 �ɹ����� 0 ʧ��
	int OnPetAbandon(UInt32 &roleID,UInt32 &petID);
	
	
	//�ӳɼ�С
	void GetPetBonusoff(UInt32 petID,List<ArchvroleBonuschange>& Bon,Pet& pet);

	//�ӳ�����
	void GetPetBonusin(UInt32 petID,List<ArchvroleBonuschange>& Bon,Pet& pet);

	//�ӳɼ���
	void PetBonusFunc(UInt32 petID,UInt32 BonusAttrID,Int32 num,Pet& pet);

	//��üӳ�ֵ
	UInt32 PetGetNewBonus( Pet& pet, UInt32 l);

	void GetPetItemBonus(UInt32 ItemID,List<ArchvroleBonuschange>& Bon);

	//========================== c-s �����ù�ack ===================================

	//[msgtype:2302]���ǰ����
	void ProcessPetIsUse(Session& session, Packet& packet);



	//============================ S-C  Ack =========================
	
	//[MsgType:1001]����������Ա��
	void NotifyPetAttrChange(UInt32 RoleID);

	//[MsgType:1002]����ӳ����Ա��
	void NotifyPetBonus(UInt32 RoleID,List<ArchvroleBonuschange> &k);

	//[MsgType:1003]����װ���ı�
	void NotifyPetEquipChange(UInt32 RoleID,List<ItemCell>& lic);

	//[MsgType:1004]���＼�ܱ��
	void NotifyPetSkillChange(UInt32 RoleID,List<ArchvSkill>&lskill);

	//[MsgType:1006]���Ｄ��
	void NotifyPetOut(UInt32 &RoleID,PetbrifInfo& petinfo);


    // ��ʱ��Ϊ:MsgType[1007]
	/////////////////////////���Ӿ���֪ͨ����Ϣ���ʹ���
	void NotifyPetAddExp(UInt32 &roleID,UInt32 petID,UInt32 &curExp);

	//============================ S-C  �����ù�Ack =========================

	//[MsgType:2101]����Ļ��
	void NotifyGetPet(UInt32 &roleID,PetBrief petBrief);

private:
	
	MainSvc * _mainSvc;
	//IniFile _file;
	ConnectionPool *_cp;
};


#endif

