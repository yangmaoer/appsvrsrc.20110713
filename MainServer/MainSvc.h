
//C_S,S_C ���ݷ������Ϣ���� server
//��ҵ���� server

#ifndef MAINSVC_H
#define MAINSVC_H

#include "GWProxy.h"
#include "IniFile.h"
#include "ConnectionPool.h"
#include "MainSvcSS.h"
#include "TimerManager.h"
#include "./Trade/TradeSvc.h"

class SSClientManager;
class MainSvcSS;
class CoreData;
class NpcSvc;
class TaskSvc;
class BagSvc;
class AvatarSvc;
class PKSvc;
class LoginSvc;
class MapSvc;
class RoleInfoSvc;
class CoreData;
class ChatSvc;
class PetSvc;
class StoreBagSvc;
class FriendSvc;
class MailSvc;
class TradeSvc;
class OffLineUpdateSvc;
class TeamSvc;
class ShopSvc;
class GuideSvc;
class DelegateSvc;
class MainSvc
{
public:
	GWProxy<MainSvc> * Service();
	CoreData * GetCoreData();
	SSClientManager * GetSSClientManager();

	TaskSvc * GetTaskSvc();
	NpcSvc * GetNpcSvc();
	BagSvc * GetBagSvc();
	AvatarSvc * GetAvatarSvc();
	PKSvc * GetPkSvc();
	LoginSvc * GetLoginSvc();
	MapSvc * GetMapSvc();
	RoleInfoSvc * GetRoleInfoSvc();
	ChatSvc * GetChatSvc();
	PetSvc * GetPetSvc();
	StoreBagSvc* GetStoreSvc();
	FriendSvc *GetFriendSvc();
	TradeSvc *GetTradeSvc();
	TeamSvc *GetTeamSvc();
	OffLineUpdateSvc *GetOffLineUpdateSvc();
	ShopSvc *GetShopSvc();
	GuideSvc *GetGuideSvc();
	DelegateSvc * GetDelegateSvc();
	MailSvc* GetMailSvc();
protected:
	MainSvc();
	~MainSvc();

	int OnInit(void* service);

	void OnStop();

	void OnConnected(Session& session);

	void OnProcessPacket(Session& session,Packet& packet);

	void ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode);


	//------------S_C ����-----------------------------------------

	//msgtype 9901 ����
	void NotifyScreenDisplay( UInt32 roleID, string &str );
	

	//------------C_S ����-----------------------------------------

	//msgtype 999 SS����˲��� 
	void	ProcessSSServerTest(Session& session,Packet& packet);

	//msgtype 9901  ���Բ��� 
	void  ProcessScreenDisplay(Session& session,Packet& packet);

	//------------��ҵ���� -----------------------------------------
	
	
public:
	TimerManager _tm;		//��ʱ��ϵͳ
	TimerPool _timerPool;
private:
	GWProxy<MainSvc>* _service;
	IniFile _file;
	ConnectionPool *_cp;

	CoreData * _coreData;

	SSServer<MainSvcSS> * _mainSvcSS;

	SSClientManager * _ssClientManager;

	TaskSvc * _taskSvc;

	NpcSvc * _npcSvc;

	BagSvc * _bagSvc;

	AvatarSvc * _avatarSvc;

	PKSvc * _pkSvc;

	LoginSvc * _loginSvc;

	MapSvc * _mapSvc;

	RoleInfoSvc * _roleInfoSvc;
	
	ChatSvc *_chatSvc;

	PetSvc *_petSvc;

	StoreBagSvc *_storeBagSvc;

	FriendSvc *_friendSvc;

	MailSvc *_mailSvc;

	TradeSvc *_tradeSvc;

	OffLineUpdateSvc *_offlineupdateSvc;

	TeamSvc *_teamsvc;

	ShopSvc * _shopSvc;

	GuideSvc * _guideSvc;

	DelegateSvc * _delegateSvc;

};


#endif

