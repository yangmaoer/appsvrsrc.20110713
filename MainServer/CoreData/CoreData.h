//��������

#ifndef COREDATA_H
#define COREDATA_H
#include <string>
#include <map>
#include <vector>
#include "Mutex.h"
#include "OurDef.h"
#include "GWProxy.h"
#include "ConnectionPool.h"
#include "Map.h"
#include  "Account.h"
#include "Team.h"
#include "ArchvPK.h"
#include "./Friend/FriendSvc.h"
#include "./Trade/ArchvTrade.h"

using namespace std;


class CoreData
{
public:
	typedef map<UInt32, UInt32> MAPROLEMAPID;
	typedef map<UInt32, Account> MAPAccount;
	typedef map<UInt32,Team> MAPROLETEAM;
	typedef map<UInt32,UInt32> MAPROLETEAMID;
public:
	CoreData(ConnectionPool * cp );
	~CoreData();
	void StopAndWait();
	int Init(void * service);
	
	void ProcessIsAdult();
	//------------S_C ����-----------------------------------------
	void NotifyNpcStatus( UInt32 roleID );

	void NotifyAllRoleInfo(UInt32 roleID);

	void NotifyRoleAddLevl(UInt32 roleID);//[MsgType:206]��ɫ����֪ͨ
	
	//----ҵ����-------------------------------------------------
	int ProcessRoleLogin( UInt32 roleID, string passwd, UInt64 clientConnID );
	int ProcessRoleLogout( UInt32 roleID );
	RolePtr ProcessGetRolePtr( UInt32 roleID );

	int RoleTeamset(UInt32 roleID,UInt32 teamID,UInt32 leaderID,Byte flag);

	Int32 RoleExpAdd(UInt32 roleID,Int32 input);

	int TradeInfo(TradeItem tradeItem,UInt32 roleID);//������Ϣ
	Monster ProcessGetMonster( int mapID, UInt32 mID );//�������
	int ProcessRoleEnterMap( UInt32 roleID, UInt32 newMapID, UInt16 X, UInt16 Y);
	int ProcessRoleMove( UInt32 roleID, ArchvRoute &art );
	int ProcessCalcAllRolePos(UInt32 roleID );
	int ChangeCreatureStatus( int mapID, List<ArchvCreatureStatus>& lcs );//�������

	UInt32 FromNameToFindID(UInt32 RoleID,string Name);
	Byte GetAccount(UInt32 roleID,UInt32 & LastloginTime,UInt32 & TopTime);
	Byte GetAccountAccountID(UInt32 AccountID,UInt32 & LastloginTime,UInt32 & TopTime);
	int UpdateAccount(UInt32 roleID,Account& l);
	int AccountIsAdult( UInt32 accountID, Byte  isAdult ,string Name,string cardID);
	int AccountPlaytimeAccess( UInt32 accountID ); 

	ArchvPosition GetSenceMonster(UInt32 mapID,UInt32 MonsterID,list<SenceMonster>& Monsters);//��ȡ�־���״��,
	list<ItemList> GetPublicDrop(UInt32 mapID,char type);
	
	void updateToptime(UInt32 accountID);
	void NotifyCtAdult(List<UInt32> &it,Byte Type);
	void NotifyCtAdultTOflag(List<UInt32> &it);
	void NotifyRoleExp(UInt32 roleID,UInt32 input);
	void NotifyRoleLev(UInt32 roleID,List<UInt32>& it);
	void NotifyEnterMap(list<UInt32>& itor,UInt32 mapID,UInt16 x,UInt16 y);
	void Notifyinfo(UInt32 roleID);

	void GetRoleIDs( list<UInt32>& lrid );
	void GetRoleIDs( UInt32 roleID, list<UInt32>& lrid );
	void GetMapRoleIDs( UInt32 mapID, list<UInt32>& lrid );
	void GetMapRoleIDs( UInt32 mapID, UInt32 roleID, list<UInt32>& lrid );
	
		
	int GetRoleTeamID(UInt32 RoleID,UInt32& teamID);//return -1û���ҵ���0�ɹ�
	Team GetTeams(UInt32 teamID);//����team;
	Team GetTeamsFromRoleID(UInt32 RoleID);
	
	int AddTeamRole(UInt32 teamID,UInt32 RoleaID);//���ӳ�Ա
	int CreateTeam(UInt32 LeaderRoleID,UInt32 RoleID);//��������
	int DeleteTeam(UInt32 TeamID);//ɾ������
	int DeleteTeamRole(UInt32 teamID,UInt32 RoleID);//ɾ����Ա
	int ChangtoBeLeader(UInt32 teamID,UInt32 RoleID);//�����ӳ�
	int ChangtoFlag(UInt32 teamID,UInt32 roleID,Byte type);
	int LogoutToTeam(UInt32 teamID,UInt32 roleID);

	void AddPKBrif(UInt32 pkID,PKBriefList& pkinfo);
	int DeletePkBrif(UInt32 pkID);
	void AddRolePKID(UInt32 pkID,PKBriefList& pkinfo);
	void deleterolePKID(PKBriefList pkinfo);

	PKBriefList GetPkInfo(UInt32 pkID);

	int FindRoleMapID( UInt32 roleID );
	int GetRoleMapID( UInt32 roleID);

	UInt32 IfRoleInPK(UInt32 roleID);//�鿴��ɫ�Ƿ���PK

	void EraseRolePKID(UInt32 roleID);
		
private:
	int CheckMapID( UInt32 mapID );
	
	void EraseRoleMapID( UInt32 roleID );
	void EraseRoleMapAccount( UInt32 roleID);
	void UpdateRoleMapID( UInt32 roleID, UInt32 mapID );
	void AddRoleMapID( UInt32 roleID, UInt32 mapID );
	
	
	void EraseRoleIDTeamID(UInt32 roleID);

	static void * OnTimerIsAdult(void * object);
	static void HandleEvent1( void * obj, void * arg, int argLen);

private:
	//----------ϵͳ����--------
	
	//�߳���
	mutable MutexLock mutex_;

	ConnectionPool *_cp;

	//��ҵ����
	MainSvc * _mainSvc;

	//----------ҵ������--------
	//��Ч��ͼ����
	int _mapCount;
	
	//��ͼ����ָ��
	vector<Map*> _vecMap;

	//roleID,mapID��ӳ��
	MAPROLEMAPID _mapRoleMapID;

	//�˺���Ϣ,roleID,��Account��ӳ��
	MAPAccount _mapAccount;
	
	//������
	MAPROLETEAM _mapTeam;

	//ROLEID��teamID��ӳ��
	MAPROLETEAMID _mapRoleTeam;

	//pk��¼����
	map<UInt32,PKBriefList> _mapPkInfo;

	map<UInt32,UInt32> _mapRolePKID;
	
	//teamID���
	UInt32 lastTeamID;
	
	//ֹͣ��־	true ֹͣ    false ����,Map�м������Եģ��ᵽCoreData��
	bool _isStop;
	
	pthread_t _thrTimerRoleIsAdult;//�����Եļ��
	
	int _intervalIsAdult;//�����Լ��ļ��ʱ��

	std::list<NewTimer*> _timerList;
};


#endif


