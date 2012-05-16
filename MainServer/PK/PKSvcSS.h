// S_S ҵ����  pk
//

#ifndef PKSVCSS_H
#define PKSVCSS_H

#include "SSServer.h"
#include "ConnectionPool.h"
#include <list>
#include "List.h"
#include "ArchvPK.h"
#include "ArchvMap.h"

class MainSvc;
class ArchvPKEndInfo;
class ArchvCreatureStatus;
class ArchvPKReward;
class PKEndInfo;

class PKSvcSS
{
public:

	PKSvcSS(MainSvc * mainSvc, ConnectionPool *cp );
	~PKSvcSS();

	void OnProcessPacket(Session& session,Packet& packet);


protected:

	void ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode);

	//------------S_S ����-----------------------------------------

	//msgtype 201 pk����
	void	ProcessPKEnd(Session& session,Packet& packet);

	void UpdateThedurability(list<RoleEquipToPk> roletype);//�����;ö�

	void UpdatePet(list<PetInfoToPk> petinfo);//�仯��Hp��MP

	//pkʤ������ɫ�Ƿ��� vip,����1 �ǣ�0 ����
	int VipExtraExpAndMoney(UInt32 roleID,UInt32 &Exp,UInt32 &Money,List<VipRole>licRoleVip);

	//------------��ҵ���� -----------------------------------------

private:
	void DealKillMonsterTask( list<UInt32>& roleID,UInt32 creaturetype,UInt32 num);
	void GetInfoFromPkEnd2( List<ArchvPKEndInfo>& lei, List<ArchvCreatureStatus>& lcs, list<PKEndInfo>& lpkei ,UInt32 mapID,UInt32 pkID);
	void GetInfoFromPkEnd1( List<ArchvPKEndInfo>& lei,List<ArchvCreatureStatus>& lcs,list<PKEndInfo>& lpkei,UInt32 mapID,UInt32 pkID);
	void GetItemFromPkEnd(list<UInt32>& roleIDs,List<ArchvPKEndInfo>& lei,list<SenceMonster>& monsters,list<PKEndInfo>& lpkei,UInt32 mapID);
	void AddExpByExpRune(List<ArchvPKEndInfo>::iterator & itor, PKEndInfo & lrpk);

	UInt32 findtheLev(list<RoleInfopk>& lis,UInt32 roleID);

	void SetPetHPMPFullAfterPk(int petID);
	void WinPetAddExp(const list<UInt32> &winPets, const list<PKEndInfo> &lpkei);
	void PetAddExp(int petID, int exp, int maxLevel);
private:
	MainSvc * _mainSvc;
	ConnectionPool *_cp;
};


#endif

