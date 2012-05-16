
//��ͼ

#ifndef MAP_H
#define MAP_H
#include <string>
#include <map>
#include <vector>
#include "Mutex.h"
#include "OurDef.h"
#include "GWProxy.h"
#include <json/json.h>
#include "ArchvMap.h"
#include "ArchvBagItemCell.h"
#include "ConnectionPool.h"
#include "./Trade/ArchvTrade.h"
#include "TimerManager.h"
#include "Role.h"

using namespace std;

class MainSvc;
class Role;
class Monster;
class Npc;
class ArchvRoute;
class ArchvPosition;
class ArchvCreatureStatus;

//����ĵ�ͼ
class Map
{
public:
	friend class CoreData;
	typedef	map<UInt32, RolePtr> MAPROLE;
	typedef	map<UInt32, Monster*> MAPMONSTER;
	typedef	map<UInt32, Npc*> MAPNPC;
	typedef map<UInt32,SenceMonsterAll> MapMonsters;  
	typedef map<char,list<ItemList> > MapItemDorp;
  

public:
	Map();
	
	~Map();

	void StopAndWait();
	static double CalcPosDistanceExact( const ArchvPosition &pos1, const ArchvPosition &pos2 );
	void ProcessAllRoleCache2DB();
	void ProcessCalcAllRolePos();
	void ProcessCheckAllMonsterStatus();//�������
	void ProcessMpHpTimeadd();
	UInt32 MapID();

protected:
	int Init( void * service, UInt32 mapID, ConnectionPool *cp);
	Json::Value GetJsonValue( const Json::Value& jValue, const string& key );
	void LeftTrim( string& output);
	int InitNpc(const string& jConfigScene );
	int InitMonster(const string& jConfigScene, const string& jConfigMonster );//�������

	//--------------ҵ����---------------------------------------
	int ProcessRoleEnterMap( RolePtr &role );
	RolePtr ProcessRoleExitMap( UInt32 roleID );
	RolePtr ProcessGetRolePtr( UInt32 roleID );
	Monster ProcessGetMonster( UInt32 mID );//�������
	void ProcessRoleMove( UInt32 roleID, ArchvRoute &art );
	void ProcessNotifyAllRoleInfo(UInt32 roleID);

	void TradeInfo(TradeItem tradeItem,UInt32 roleID);



	UInt32 FromNameTOFindID(string Name);//����RoleID

	

	void GetRoleIDs( list<UInt32>& lrid );
	void GetRoleIDs( UInt32 roleID, list<UInt32>& lrid );

	list<ItemList> GetPublicDrop(char type);//��ȡ�����������Ʒ
	ArchvPosition GetSenceMonster(UInt32 MonsterID,list<SenceMonster>& Monsters);//��ȡ�־���״��,
	int GetallTheNum(UInt32 ID,list<UInt32> lis);//�ж�lis���Ƿ���ID���еĻ�������1��û�еĻ�����0

	void GiveVipGiftOnEnterMap(RolePtr& role);
	
private:
	static void TimerCBCalcAllRolePos( void * obj, void * arg, int argLen );
	static void TimerCBCache2DB( void * obj, void * arg, int argLen );
 	static void TimerCBCheckMonsterStatus( void * obj, void * arg, int argLen );
	static void TimerCBHpMpadd( void * obj, void * arg, int argLen );
	static void TimerCBS2C204( void * obj, void * arg, int argLen );
		
	//static void * OnTimerIsAdult(void * object);
	void MakeSCPkgForRole( RolePtr& role, Byte status, List<ArchvCreatureStatus>& lcs );
	void MakeSCPkgForMonster( SenceMonsterAll& mst, Byte status, List<ArchvCreatureStatus>& lcs );//�������
	int SetRoleStatus( UInt32 roleID, Byte status );
	int SetMonsterStatus( UInt32 monsterID, Byte status );//�������

	//------------S_C ��Ϣ-----------------------------------------
	void NotifyRoleMove( UInt32 roleID, Byte moveType, ArchvRoute &art );
	void NotifyAllRoleInfo( UInt32 roleID );
	void NotifyCorrectRoute( UInt32 roleID, ArchvRoute &rt );
	void NotifyCtStatusUnicast( UInt32 roleID, Byte flag);
	void NotifyCtStatusBroadcast( List<ArchvCreatureStatus> &lcs );
	//	/void NotifyCtAdult(List<UInt32> &it,Byte Type);
	void NotifyRoleAddLevl(UInt32 roleID);
	void NotifyHPandMP(UInt32 roleID, UInt32 hp, UInt32 mp);


private:
	//----------ϵͳ����---------
	//��ʱ��ϵͳ
	TimerManager _tm;
	
	//�߳���
	mutable MutexLock mutex_;

	//DB���ӳ�
	ConnectionPool *_cp;
	
	//ֹͣ��־	true ֹͣ    false ����
	bool _isStop;

	//��ҵ����
	MainSvc * _mainSvc;

	//��ʱ����ʱ��:		��ʱ�����ɫ��ǰλ��
	int _intervalCalcRolePos;

	//��ʱ����ʱ��:		��ɫ�������
	int _intervalCache2DB;

	//�Զ���Ѫ ʱ����
	int _intervalHpMpadd;

	//��ʱ����ʱ��:		���С��״̬ʱ����
	int _intervalCheckMonsterStatus;

	//С�ָ���ʱ����
	int _intervalMonsterRevive;
	
	//int _intervalIsAdult;//�����Լ��ļ��ʱ��
	
	//----------ҵ������---------
	//��ͼ���
	UInt32 _mapID;

	//��ɫ���ݼ�
	MAPROLE _mapRole; //Guarded by mutex_
	
	//�����ݼ�
	MAPMONSTER _mapMonster;

	//Npc���ݼ�
	MAPNPC _mapNpc;

	//�����������

	MapMonsters _mapsenceMonster;

	//������Ʒ�������
	MapItemDorp _mapItemdrop;

	
	
	
};


#endif


