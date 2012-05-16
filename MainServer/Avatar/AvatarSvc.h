//ҵ���� server  avatar���

#ifndef AVATARSVC_H
#define AVATARSVC_H

#include "GWProxy.h"
#include "IniFile.h"
#include "ConnectionPool.h"
#include "LuaState.h"
#include "ArchvAvatar.h"
#include "ArchvPK.h"
#include "ArchvBagItemCell.h"
#include "Role.h"




class MainSvc;
class AvatarSvc
{
public:
	AvatarSvc(void* service, ConnectionPool * cp);
	~AvatarSvc();
 	void OnProcessPacket(Session& session,Packet& packet);

	void ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode);

	int CheckEquipType( int input );


	//------------S_C ����-----------------------------------------
	//msgtype 501 Avatar���£�֪ͨ������
	void NotifyAvatarUpdate( UInt32 roleID ,UInt32 wepID,UInt32 coatID);
	//msgtype502 װ������֪ͨ
	void NotifyEquipUpdate(UInt32 RoleID,List<ItemCell>& lic);
	void NotifyBonus(UInt32 RoleID,List<ArchvroleBonuschange> &k);

	void NotifySingleBonus(UInt32 RoleID, UInt16 bonusAttrID, UInt32 num);


	//------------C_S ����-----------------------------------------

	void  ProcessGetRolesAvatar(Session& session,Packet& packet);//msgtype 501 ��ȡ�����ɫ avatar


	void  ProcessUpdateAvatar(Session& session,Packet& packet);//msgtype 502 ���� avatar


	void  ProcessGetRolesBriefAvatar(Session& session,Packet& packet);//msgtype 503 ��ȡ�����ɫ��Ҫ avatar


	void  ProcessGetRolesEquipPos(Session& session,Packet& packet);//msgtype 504 װ���ܷ�λ����ʾ


	void  ProcessRolesEquipGetoff(Session& session,Packet& packet);//506 ���ŵ�װ������


	void ProcessTestByWangLian(Session& session,Packet& packet);  //�������˲��ԣ��������ã�ɾ���������������Ӱ��

	//------------��ҵ���� -----------------------------------------

	void GetRoleSkill(UInt32 roleID,List<ArchvSkill>& sk);//װ����Ҫ��Ϣ,����0���ɹ�������-1ʧ��

	int GetEquipBrief(List<UInt32>& listRoleID , List<ArchvAvatarDescBrief>& ladb);

	//int GetItemBonus(UInt32 RoleID,List<UInt32>& l,List<ArchvroleBonuschange>& Bon,List<ArchvroleEuip>& Ebon);//�ж��Ƿ���Ҫ��������������Ƿ��������ӳɵ�װ��������1����Ҫ������0����Ҫ

	int IfneedToGet(UInt32 RoleID,List<UInt32>& l);


	int GetJustItemBonus(UInt32 ItemID,List<ArchvroleBonuschange>& Bon,List<UInt32>& l);//��ɫ����װ����������������֮���ֵ

	void GetRoleBonusoff(UInt32 RoleID,List<ArchvroleBonuschange>& Bon);//��ɫ����װ�����������ⷽʽ��ֵ

	void GetRoleBonusin(UInt32 RoleID,List<ArchvroleBonuschange>& Bon);//��ɫ������ֵ�Ķ�Ӧ��������

	void RoleBonus(UInt32 RoleID,UInt32 BonusAttrID,Int32 num);//��ɫ�ٷְټӳɣ�����������ʽ

	void RoleOtherBonus(UInt32 RoleID,List<UInt32>& L);

	UInt32 RoleGetNewBonus(UInt32 RoleID,UInt32 l,RolePtr& role);

	UInt32 RoleGetinfo(UInt32 RoleID,UInt32 l);

	Int32 Updatebind(UInt32 RoleID,UInt32 EntityID);

	UInt32 UseItem(UInt32 roleID,UInt32 ItemID,UInt32 EntityID,UInt32 EquipPos,UInt16 celIndex);


    //װ���;ö����
	int OnEquipDurabilityLoss(UInt32 roleID,UInt32 lossDurability);


//	Int32 CheckifhavaGet(UInt32 l,List <ArchvroleBonuschange>& bon);
	//Int32 GetBonusList(UInt32 RoleID,UInt32 l,List<ArchvroleBonuschange>& bon);
//private:


private:
	MainSvc * _mainSvc;
	IniFile _file;
	ConnectionPool *_cp;

	LuaState _luaState;

};


#endif

