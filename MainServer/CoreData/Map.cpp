#include "Map.h"
#include "MainSvc.h"
#include "Log.h"
#include <stdlib.h>
#include "DBOperate.h"
#include "Role.h"
#include "Monster.h"
#include "Npc.h"
#include "DebugData.h"
#include <math.h>
#include "ArchvMap.h"
#include <fstream>
#include <iostream>
#include "../Npc/NpcSvc.h"
#include "Account.h"
#include "CoreData.h"
#include "ArchvMap.h"
#include "ArchvBagItemCell.h"
#include "../Mail/ArchvMail.h"
#include "../Mail/MailSvc.h"

#include "TimerManager.h"
#include "NewTimer.h"

Map::Map()
:mutex_()
,_mapID(0)
,_mainSvc(NULL)
,_intervalCalcRolePos(30)
,_intervalCache2DB(30)
,_intervalCheckMonsterStatus(30)
,_intervalMonsterRevive(30)
,_intervalHpMpadd(10)
,_isStop(true)
{}

Map::~Map()
{

	//����ֹͣ��־�����ȴ����߳��˳�
	StopAndWait();

	//��ɫ�������
	ProcessAllRoleCache2DB();

	//��Դ�ͷ�
	MutexLockGuard lock(mutex_);
	{
		//�ͷ� ��ɫ��Դ
		MAPROLE::iterator it1;
//		for( it1 = _mapRole.begin(); it1 != _mapRole.end(); it1++)
//			delete it1->second;
		_mapRole.clear();

		//�ͷ� ����Դ
		MAPMONSTER::iterator it2;
		for( it2 = _mapMonster.begin(); it2 != _mapMonster.end(); it2++)
			delete it2->second;
		_mapMonster.clear();

		//�ͷ� Npc��Դ
		MAPNPC::iterator it3;
		for( it3 = _mapNpc.begin(); it3 != _mapNpc.end(); it3++)
			delete it3->second;
		_mapNpc.clear();

	}

}

//����ֹͣ��־�����ȴ����߳��˳�
void Map::StopAndWait()
{
	_isStop = true;

	//�ȴ����߳��˳�
//	pthread_join(_thrTimerCalcRolePos,NULL);
//	pthread_join(_thrTimerCache2DB,NULL);


}

//@��ʼ��
//@param	service				��ҵ�����
//@param	mapID					��ͼID
//@return 0 �ɹ�  ��0 ʧ��
int Map::Init( void * service, UInt32 mapID, ConnectionPool * cp)
{
	int iRet = 0;

	//������������
	_mainSvc = static_cast<MainSvc*>(service);
	_mapID = mapID;
	_cp = cp;

	//��ȡ����
	IniFile iniFile;
	string strJConfigScene;					//json�����ļ�Ŀ¼--��ͼ
	string strJConfigMonster;				//json�����ļ�Ŀ¼--����
	string strIntervalCalcRolePos;	//�����ɫ��ǰλ�õ�ʱ����
	string strIntervalDBCheckIn;		//	������DB��ʱ����
	string strIntervalCheckMonsterStatus;//	����״̬ʱ����
	string strIntervalMonsterRevive;//С�ָ���ʱ����

	if(!iniFile.open( _mainSvc->Service()->GetConfFilePath()))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"iniFile.open() error!" );
		return -1;
	}

	if( iniFile.read("Map","jConfigScene", strJConfigScene) ) return -1;
	if( iniFile.read("Map","jConfigMonster", strJConfigMonster) ) return -1;
	if( iniFile.read("Map","intervalCalcRolePos", strIntervalCalcRolePos) ) return -1;
	if( iniFile.read("Map","intervalDBCheckIn", strIntervalDBCheckIn) ) return -1;
	if( iniFile.read("Map","intervalCheckMonsterStatus", strIntervalCheckMonsterStatus) ) return -1;
	if( iniFile.read("Map","intervalMonsterRevive", strIntervalMonsterRevive) ) return -1;

	_intervalCalcRolePos = atoi(strIntervalCalcRolePos.c_str());
  _intervalCache2DB = atoi(strIntervalDBCheckIn.c_str());
  _intervalCheckMonsterStatus = atoi(strIntervalCheckMonsterStatus.c_str());
  _intervalMonsterRevive = atoi(strIntervalMonsterRevive.c_str());

	//����У��
	if( _intervalCalcRolePos <= 0 ||
			_intervalCache2DB	<= 0 ||
			_intervalCheckMonsterStatus <= 0 ||
			_intervalMonsterRevive <= 0)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"error param!! _intervalCalcRolePos[%d], _intervalCache2DB[%d], _intervalCheckMonsterStatus[%d],_intervalMonsterRevive[%d]", _intervalCalcRolePos, _intervalCache2DB, _intervalCheckMonsterStatus,_intervalMonsterRevive );
		return -1;
	}

	//��ʱ�����
	_isStop = false;	
	TimerManager& timerMgr = _mainSvc->_tm;

	//todo:��Խ�ɫ�Ķ�ʱ��Ӧ�÷ŵ���ɫ���б������ɫ�˳���ʱ��ɾ����ʱ��

	//��ʱ��: �����ɫ·��
	NewTimer *timer = _mainSvc->_timerPool.newTimer();
	if (timer != 0)
	{
		timer->Type(NewTimer::LoopTimer);
		iRet = timer->SetCallbackFun( TimerCBCalcAllRolePos, this, NULL, 0);
		if(iRet)
		{
			LOG (LOG_ERROR, __FILE__, __LINE__, "SetCallbackFun  error.");
	 		return -1;
		}
		timer->Interval(_intervalCalcRolePos);
		_tm.AddTimer(timer);	
	}
	
	//��ʱ��: �������
	timer = _mainSvc->_timerPool.newTimer();
	if (timer != 0)
	{
		timer->Type(NewTimer::LoopTimer);
		iRet = timer->SetCallbackFun( TimerCBCache2DB, this, NULL, 0);
		if(iRet)
		{
			LOG (LOG_ERROR, __FILE__, __LINE__, "SetCallbackFun  error.");
	 		return -1;
		}
		timer->Interval(_intervalCache2DB);
		_tm.AddTimer(timer);
	}
	
	//��ʱ��: С��ˢ��
	timer = _mainSvc->_timerPool.newTimer();
	if (timer != 0)
	{
		
		timer->Type(NewTimer::LoopTimer);
		iRet = timer->SetCallbackFun( TimerCBCheckMonsterStatus, this, NULL, 0);
		if(iRet)
		{
			LOG (LOG_ERROR, __FILE__, __LINE__, "SetCallbackFun  error.");
			return -1;
		}
		timer->Interval(_intervalCheckMonsterStatus);
		_tm.AddTimer(timer);
	}
	
	//��ʱ��: �����Զ���Ѫ
	timer = _mainSvc->_timerPool.newTimer();
	if (timer != 0)
	{
		timer->Type(NewTimer::LoopTimer);
		iRet = timer->SetCallbackFun( TimerCBHpMpadd, this, NULL, 0);
		if(iRet)
		{
			LOG (LOG_ERROR, __FILE__, __LINE__, "SetCallbackFun  error.");
	 		return -1;
		}
		timer->Interval(_intervalHpMpadd);
		_tm.AddTimer(timer);
	}
	

	//npc ��ʼ��
	iRet = InitNpc(strJConfigScene);
	if(iRet)
	{
		LOG (LOG_ERROR, __FILE__, __LINE__, "InitNpc error. mapID[%d]", _mapID);
 		return -1;
	}

	//�ֳ�ʼ��
	iRet = InitMonster( strJConfigScene, strJConfigMonster );
	if(iRet)
	{
		LOG (LOG_ERROR, __FILE__, __LINE__, "InitMonster error. mapID[%d]", _mapID);
 		return -1;
	}

	return 0;
}

//@brief	��ȡjson ������װ
//	�����쳣��д��־
//@return	json::Value
Json::Value Map::GetJsonValue( const Json::Value& jValue, const string& key )
{
	Json::Value vRet;
	if(jValue.isMember(key))
		vRet = jValue[key];

	return vRet;
}

//@brief 	�ַ���ȥ��ǰ���� 0
void Map::LeftTrim( string& output)
{
	string strTmp(output);
	int i = 0;
	for( i = 0; i < strTmp.size(); i++ )
	{
		if( "0" != strTmp.substr( i, 1 ) )
			break;
	}

	output = strTmp.substr(i);
}



//@brief	��ʼ��������
//@return	0 �ɹ�  ��0 ʧ��
int Map::InitNpc(const string& jConfigScene )
{
	//��ȡ json���ò���
	Json::Reader reader;
	Json::Value rootScene;

	//��ȡ ��ͼ�����ļ�
	ifstream fileScene(jConfigScene.c_str());
	if (!reader.parse(fileScene, rootScene, false))
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "reader.parse error., jConfigScene[%s]", jConfigScene.c_str() );
		return -1;
  }

	//��ȡ��ͼID
	char szMapID[64];
	sprintf( szMapID, "%d", _mapID );
	Json::Value jMap = GetJsonValue( rootScene, szMapID );
	if(jMap.empty())
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "jMap is empty, mapID[%d]", _mapID);
		return 0;
	}

	//��ȡ NPC
  Json::Value jNpcBrief = GetJsonValue( jMap, "npc" );
	if(jNpcBrief.empty())
	{
		LOG(LOG_DEBUG, __FILE__, __LINE__, "jNpcBrief is empty, mapID[%d]", _mapID);
		return 0;
	}

  string strCreatureID;
  string strCreatureType;
  Npc * npc = NULL;
  ArchvPosition pos;
  for( int i = 0; i < jNpcBrief.size(); i++ )
  {
  	Json::Value &curr = jNpcBrief[i];

		//��ͼ������ȡ
		strCreatureID = GetJsonValue( curr, "id" ).asString();
		strCreatureType = strCreatureID.substr(1,4);
		pos.X = atoi(GetJsonValue( curr, "posX" ).asString().c_str());
		pos.Y = atoi(GetJsonValue( curr, "posY" ).asString().c_str());

  	//Npc
  	npc = new Npc();
		npc->ID( atoi(strCreatureID.c_str()) );
		npc->Flag(3);
		npc->Type( atoi(strCreatureType.c_str()) );
		npc->Pos(pos);

		//��ӡ����
		if( 0 == i && 1 == _mapID  )
			npc->Print();

		//������б�
		_mapNpc.insert( make_pair(npc->ID(),npc) );

  }

	if( _mapNpc.size() == 0 )
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "no npc in map!!,mapID[%d], _mapNpc.size()[%d]", _mapID, _mapNpc.size() );
		return -1;
	}

	return 0;
}



//@brief	��ʼ��������
//@return	0 �ɹ�  ��0 ʧ��
int Map::InitMonster(const string& jConfigScene, const string& jConfigMonster )
{
	//��ȡ json���ò���
	Json::Reader reader;
	Json::Value rootScene;
	Json::Value rootMonster;
	SenceMonster monsterp;
	SenceMonsterAll monsters;
	UInt32 monsterTypeNum;
	list<UInt32> listNum;
	UInt32 TaskID=0;
	ItemList itms;

	ItemList item;
	list<ItemList> its;
	list<UInt32>::iterator it;
	//��ȡ ��ͼ�����ļ�
	ifstream fileScene(jConfigScene.c_str());
	if (!reader.parse(fileScene, rootScene, false))
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "reader.parse error.");
		return -1;
  }

	//��ȡ �������ļ�
	ifstream fileMonster(jConfigMonster.c_str());
  if (!reader.parse(fileMonster, rootMonster, false))
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "reader.parse error.");
		return -1;
  }

	//��ȡ��ͼID
	char szMapID[64];
	sprintf( szMapID, "%d", _mapID );
	Json::Value jMap = GetJsonValue( rootScene, szMapID );
	if(jMap.empty())
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "jMap is empty, mapID[%d]", _mapID);
		return 0;
	}

	//��ȡ С�� ����
  Json::Value jMonsterBrief = GetJsonValue( jMap, "monsters" );
 	if(jMonsterBrief.empty())
 	{
		LOG(LOG_DEBUG, __FILE__, __LINE__, "jMonsterBrief is empty, mapID[%d]", _mapID);
		return 0;
	}

	Json::Value jItemDrop=GetJsonValue( rootMonster, "PublicDropType" );
	char Ttype=0;
//	LOG(LOG_DEBUG,__FILE__,__LINE__,"jItemDrop.size[%d]",jItemDrop.size());
	for(int i=0;i< jItemDrop.size();i++)
	{		its.clear();

		Json::Value &cxrr = jItemDrop[i];//��ȡ
		string a=GetJsonValue( cxrr, "DropType" ).asString();

		Ttype=a[0];
		Json::Value JItemlist;
		JItemlist=GetJsonValue( cxrr, "ItemList" );
		for(int j=0;j<JItemlist.size();j++)
		{
			item.ItemID=atoi(GetJsonValue( JItemlist[j], "ItemID" ).asString().c_str());
			item.num=atoi(GetJsonValue( JItemlist[j], "rate" ).asString().c_str());
			its.push_back(item);
		}
		_mapItemdrop.insert( make_pair(Ttype,its) );

	}



	string strCreatureID;
	string strCreatureType;
	string strPCreatureType;
	string strID;
	ArchvPosition pos;
	for(int i=0;i< jMonsterBrief.size(); i++)
	{
		Json::Value &curr = jMonsterBrief[i];//��ȡ
		strCreatureID = GetJsonValue( curr, "id" ).asString();//��ȡID
		monsters.Monsterp.clear();
		strCreatureType = strCreatureID.substr(1,4);
		LeftTrim(strCreatureType);			//ȥ��ǰ���� 0
		pos.X = atoi(GetJsonValue( curr, "posX" ).asString().c_str());
		pos.Y = atoi(GetJsonValue( curr, "posY" ).asString().c_str());
		monsters.X=pos.X;
		monsters.Y=pos.Y;
		monsters.MonsterID=atoi(strCreatureID.c_str());
		monsters.MonsterType=atoi(strCreatureType.c_str());
		monsters.Status=1;//1���,2ս���У�4����
		if(GetallTheNum(monsters.MonsterType,listNum)!=1)
		{
			listNum.push_back(monsters.MonsterType);
		}


		Json::Value partners=GetJsonValue( curr, "monsterpartner" );
		for(int j=0;j< partners.size();j++)
		{
			Json::Value &corr =partners[j];
			strPCreatureType=GetJsonValue( corr, "monsterType" ).asString();
			monsterp.MonsterType=atoi(strPCreatureType.c_str());
			monsterp.num=atoi(GetJsonValue( corr, "monsternum" ).asString().c_str());

				if(GetallTheNum(monsterp.MonsterType,listNum)!=1)
				{
					listNum.push_back(monsterp.MonsterType);
				}
			monsters.Monsterp.push_back(monsterp);

		}


		//��Ҫ�õĹ������������ͼ��




		_mapsenceMonster.insert( make_pair(monsters.MonsterID,monsters) );
		//��Ӫ���ڲ���
	}
  //��ȡ������Ʒ����




  Monster * mstr = NULL;

 for( it = listNum.begin(); it!=listNum.end(); it++ )
  {
		char szTmp[64];
  	sprintf( szTmp, "%d",*it);
  	strID=szTmp;

		LeftTrim(strID);
		//��ȡ�����͹ֵ�����
  	Json::Value jMonsterDesc = GetJsonValue( rootMonster, strID);
		Json::Value jMonsterItem;
  	//����������ȡ
  	mstr = new Monster();
		mstr->ID( atoi(strID.c_str()) );							//��ID
		mstr->Flag(2);
		mstr->Type( atoi(strID.c_str()) );					//��������
		mstr->Camp(1);
		//mstr->Camp( atoi(GetJsonValue( jMonsterDesc, "defaultcomap" ).asString().c_str()) );
		mstr->MapID(_mapID);
		mstr->Status(1);						//����״̬
		mstr->MaxHp(atoi(GetJsonValue( jMonsterDesc, "maxHP" ).asString().c_str()) );
		mstr->MaxMp(atoi(GetJsonValue( jMonsterDesc, "maxMP" ).asString().c_str()) );
		mstr->Level(atoi(GetJsonValue( jMonsterDesc, "level" ).asString().c_str()) );
		mstr->Hp(mstr->MaxHp());
		mstr->Mp(mstr->MaxMp());
		mstr->MoveSpeed(atoi(GetJsonValue( jMonsterDesc, "moveSpeed" ).asString().c_str()) );
		mstr->AttackPowerHigh(atoi(GetJsonValue( jMonsterDesc, "attackPowerHigh" ).asString().c_str()) );
		mstr->AttackPowerLow(atoi(GetJsonValue( jMonsterDesc, "attackPowerLow" ).asString().c_str()) );
		mstr->AttackScope(atoi(GetJsonValue( jMonsterDesc, "attackArea" ).asString().c_str()) );
		mstr->AttackSpeed(atoi(GetJsonValue( jMonsterDesc, "attackSpeed" ).asString().c_str()) );
		mstr->BulletSpeed(atoi(GetJsonValue( jMonsterDesc, "attackBulletSpeed" ).asString().c_str()) );
		mstr->HitRate(atoi(GetJsonValue( jMonsterDesc, "hitRate" ).asString().c_str()) );
		mstr->DodgeRate(atoi(GetJsonValue( jMonsterDesc, "dodgeRate" ).asString().c_str()) );
		mstr->Defence(atoi(GetJsonValue( jMonsterDesc, "defense" ).asString().c_str()) );
		mstr->MDefence(atoi(GetJsonValue( jMonsterDesc, "mDefense" ).asString().c_str()) );
		//�����һЩ��ģ���ʱû�ж������ļ�
		mstr->CritRate(atoi(GetJsonValue( jMonsterDesc, "critRate" ).asString().c_str()));//����
		mstr->Money=atoi(GetJsonValue( jMonsterDesc, "money" ).asString().c_str());
		mstr->Exp=atoi(GetJsonValue( jMonsterDesc, "Exp" ).asString().c_str());
		jMonsterItem=GetJsonValue( jMonsterDesc, "ItemDropTask" );//������䲻��
		for(int pp=0;pp<jMonsterItem.size();pp++)
		{
			TaskID=atoi(GetJsonValue( jMonsterItem[pp], "TaskID" ).asString().c_str());
			itms.ItemID=atoi(GetJsonValue( jMonsterItem[pp], "ItemID" ).asString().c_str());
			itms.num=atoi(GetJsonValue( jMonsterItem[pp], "rate" ).asString().c_str());
			mstr->ItemDropTask.insert( make_pair(TaskID,itms) );
		}
		jMonsterItem=GetJsonValue(jMonsterDesc, "ItemDropSpec" );//��Ʒ����
		for(int pp=0;pp<jMonsterItem.size();pp++)
		{
			itms.ItemID=atoi(GetJsonValue( jMonsterItem[pp], "ItemID" ).asString().c_str());
			itms.num=atoi(GetJsonValue( jMonsterItem[pp], "rate" ).asString().c_str());
			mstr->ItemDropSpec.push_back(itms);
		}
		string publicchar;
		publicchar=GetJsonValue( jMonsterDesc, "ItemDropPublic" ).asString();//��������
		mstr->ItemDropPublic=publicchar[0];


		mstr->Pos(pos);

		_mapMonster.insert( make_pair(mstr->ID(),mstr) );

  }
	if( _mapMonster.size() == 0 )
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "no monster in map!!,mapID[%d], _mapMonster.size()[%d]", _mapID, _mapMonster.size() );
		return -1;
	}


	return 0;
}


//��ʱ���ص�
//	��ʱ�������н�ɫ��ǰλ��,�������ƶ�·��
void Map::TimerCBCalcAllRolePos( void * obj, void * arg, int argLen )
{
	Map * service = static_cast<Map *>(obj);
	service->ProcessCalcAllRolePos();
}

//��ʱ���ص�
//	�������
void Map::TimerCBCache2DB( void * obj, void * arg, int argLen )
{
	Map * service = static_cast<Map *>(obj);
	service->ProcessAllRoleCache2DB();

//	if( 1 == service->MapID())
//		DEBUG_PRINTF2("TimerCBCache2DB, threadID[%d], time[%d]", pthread_self(), time(NULL));

}


//��ʱ���ص�
//	��ʱС��ˢ��
void Map::TimerCBCheckMonsterStatus( void * obj, void * arg, int argLen )
{
	Map * service = static_cast<Map *>(obj);
	service->ProcessCheckAllMonsterStatus();
}

//��ʱ���ص�
//	�Զ���Ѫ
void Map::TimerCBHpMpadd( void * obj, void * arg, int argLen )
{
	Map * service = static_cast<Map *>(obj);
	service->ProcessMpHpTimeadd();
}

//��ʱ���ص�
//	�Ӻ��� S_C 204
void Map::TimerCBS2C204( void * obj, void * arg, int argLen )
{
 	UInt32 roleID = *((UInt32*)arg);
 	Map * service = static_cast<Map *>(obj);
	service->NotifyCtStatusUnicast( roleID, 0x07 );

	DEBUG_PRINTF1( "TimerCBS2C204-------roleID[%d]--> ", roleID );
}


//��ȷ������������ֱ�߾���
//	ʹ�ù��ɶ���
//@param	pos1		����1
//@param	pos2		����2
//@param	direct	���صķ���
//@return	>=0		�������ֱ�߾���   <0		ʧ��
double Map::CalcPosDistanceExact( const ArchvPosition &pos1, const ArchvPosition &pos2 )
{
	return sqrt( pow( abs(pos1.X-pos2.X), 2 ) + pow( abs(pos1.Y-pos2.Y), 2 ) );
}

//@brief �齨��ɫ������״̬�㲥��
//@return ��
void Map::MakeSCPkgForRole( RolePtr& role, Byte status, List<ArchvCreatureStatus>& lcs )
{
	ArchvCreatureStatus cs;

	cs.creatureFlag = 1;
	cs.creatureType = role->ProID();
	cs.ID = role->ID();
	cs.status = status;

	lcs.push_back(cs);
}

//@brief �齨�ֵ�����״̬�㲥��
//@return ��
void Map::MakeSCPkgForMonster( SenceMonsterAll& mst, Byte status, List<ArchvCreatureStatus>& lcs )
{
	ArchvCreatureStatus cs;

	cs.creatureFlag = 2;
	cs.creatureType = mst.MonsterType;
	cs.ID = mst.MonsterID;
	cs.status = status;

	lcs.push_back(cs);
}

//@brief	���ö�������--״̬
//@return 0�ɹ� 1 ���󲻴���   ���� ʧ��
int Map::SetRoleStatus( UInt32 roleID, Byte status )
{
	MutexLockGuard lock(mutex_);

	MAPROLE::iterator it;
	it = _mapRole.find(roleID);
	if( _mapRole.end() == it )
	{
		LOG (LOG_ERROR, __FILE__, __LINE__, "SetRoleStatus() error! role not found, roleID[%d]", roleID );
		return 1;
	}

	//�޸�״̬
	it->second->Status(status);

	//����״̬�㲥
	List<ArchvCreatureStatus> lcs;
	MakeSCPkgForRole( it->second, status, lcs );
	NotifyCtStatusBroadcast(lcs);

	return 0;
}

//@brief	���ù�����--״̬
//@return 0�ɹ� 1 ���󲻴���   ���� ʧ��
int Map::SetMonsterStatus( UInt32 monsterID, Byte status )
{
	MutexLockGuard lock(mutex_);

	MapMonsters::iterator it;
	it = _mapsenceMonster.find(monsterID);
	if( _mapsenceMonster.end() == it )
	{
		LOG (LOG_ERROR, __FILE__, __LINE__, "SetMonsterStatus() error! monster not found, monsterID[%d]", monsterID );
		return 1;
	}
	//�޸�״̬
	if(status==0)
	{
		it->second.dieTime=time(NULL);
	}
	it->second.Status=status;
	//������������ʱ��
	//����״̬�㲥
	List<ArchvCreatureStatus> lcs;
	MakeSCPkgForMonster( it->second, status, lcs );
	NotifyCtStatusBroadcast(lcs);

	return 0;
}


//@brief	��ɫ�������
//@return ��
void Map::ProcessAllRoleCache2DB()
{
	MutexLockGuard lock(mutex_);

	MAPROLE::iterator it;
//	LOG (LOG_ERROR, __FILE__, __LINE__, "hello time %d",time(NULL));
	for( it = _mapRole.begin(); it != _mapRole.end(); it++ )
	{
		it->second->Cache2DB();
	}
	//LOG (LOG_ERROR, __FILE__, __LINE__, "hello time %d",time(NULL));
}
void Map::ProcessMpHpTimeadd() {
	MutexLockGuard lock(mutex_);
	MAPROLE::iterator it;
	//	LOG (LOG_ERROR, __FILE__, __LINE__, "hello time %d",time(NULL));

	for (it = _mapRole.begin(); it != _mapRole.end(); it++) {
		if (it->second->Hp() < it->second->MaxHp() || it->second->Mp()
				< it->second->MaxMp()) {
			it->second->HpMpadd(_intervalHpMpadd);
			NotifyHPandMP(it->second->ID(), it->second->Hp(), it->second->Mp());
			//		LOG (LOG_DEBUG, __FILE__, __LINE__, "1111111 RoleID[%d] HP[%d] MP[%d]",it->second->ID(), it->second->Hp(), it->second->Mp());
		}
	}
}

UInt32 Map::MapID()
{
	return _mapID;
}

//@brief	С�ֶ�ʱ����
//@return ��
void Map::ProcessCheckAllMonsterStatus()
{
	MutexLockGuard lock(mutex_);

	map<UInt32,SenceMonsterAll>::iterator it;
	ArchvCreatureStatus cs;
	List<ArchvCreatureStatus> lcs;
	for( it = _mapsenceMonster.begin(); it != _mapsenceMonster.end(); it++ )
	{
		//�����Ĺ�,��������ʱ����,�򸴻�
		if( 0 == it->second.Status &&
				(time(NULL) - it->second.dieTime > _intervalMonsterRevive) )
		{
			it->second.Status=1;
			cs.creatureFlag = 2;
			cs.creatureType = it->second.MonsterType;
			cs.ID = it->second.MonsterID;
			cs.status = 4;				//��ˢ�¸���

			lcs.push_back(cs);
		}
	}

	//�㲥
	if(lcs.size()>0)
		NotifyCtStatusBroadcast(lcs);

}


//�������н�ɫ��ǰλ��,�������ƶ�·��
void Map::ProcessCalcAllRolePos()
{
	MutexLockGuard lock(mutex_);

	MAPROLE::iterator it;
	for( it = _mapRole.begin(); it != _mapRole.end();it++ )
	{
		//�����ɫ��ǰλ��
		it->second->CalcCurrPos();
	}

}

//@brief	���� �õ�ͼ����������ɫ����Ϣ
void Map::ProcessNotifyAllRoleInfo(UInt32 roleID)
{
	MutexLockGuard lock(mutex_);

	//�����û���Ϣ�б����͸����볡���Ľ�ɫ
	NotifyAllRoleInfo( roleID );
}
list<ItemList> Map::GetPublicDrop(char type)//��ȡ�����������Ʒ
{

	MutexLockGuard lock(mutex_);
	MapItemDorp::iterator it;
	list<ItemList> itemlist;

	it = _mapItemdrop.find(type);

	//��ɫ�Ѿ��ڵ�ͼ��
	if( _mapItemdrop.end() == it )
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "find the type to the monster erro type=%c", type );
		return itemlist;
	}

	itemlist=it->second;
	return itemlist;

}



//@brief	��ɫ�����ͼ
//@param	role	��ɫ����
//@return 0 �ɹ�  ��0 ʧ��
int Map::ProcessRoleEnterMap( RolePtr &pRole )
{
	MutexLockGuard lock(mutex_);

	MAPROLE::iterator it;
	it = _mapRole.find(pRole->ID());

	//��ɫ�Ѿ��ڵ�ͼ��
	if( _mapRole.end() != it )
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "ProcessRoleEnterMap() error, Role exists!!!!! roleID[%d]", pRole->ID() );
		return -1;
	}

	//�����ڴ�,�����¼
	pRole->SetConnectPool(_cp);			//�����еĽ�ɫ,��ҪDB ����
	pRole->MapID(_mapID);						//ǿ�����޸Ľ�ɫ��ͼ,�����ⲿ��ֵ������
	pRole->AddEnterMapNum();				//�����ͼ�ܴ����ĸ���
	_mapRole.insert( make_pair( pRole->ID(), pRole));

	if (pRole->ISVIP() <= 6)
	{
		GiveVipGiftOnEnterMap(pRole);
	}
	//����д��DB
	pRole->Cache2DB();

	//���ͽ��볡���㲥
	ArchvRoute route(pRole->Route());
	NotifyRoleMove( pRole->ID(), 0, route );


	//��ʱ�� �Ӻ�N�뷢��:
	//	�������������ɫ���֡����״̬�����͸������ͼ��
	UInt32 roleID = pRole->ID();
	NewTimer* timer = _mainSvc->_timerPool.newTimer();
	if (timer)
	{
		timer->Type(NewTimer::OnceTimer);
		int iRet = timer->SetCallbackFun( TimerCBS2C204, this, &roleID, sizeof(roleID));
		if(iRet)
		{
			LOG (LOG_ERROR, __FILE__, __LINE__, "SetCallbackFun  error.");
 			return -1;
		}
		timer->Interval(2);
		_tm.AddTimer(timer);
	}

DEBUG_PRINTF2("ProcessRoleEnterMap success!!!+++++roleID[%d] login, mapID[%d]", pRole->ID(), _mapID );
DEBUG_PRINTF3("roleID[%d],name[%s], proID[%d]", pRole->ID(), pRole->Name().c_str(), pRole->ProID());
DEBUG_PRINTF1("_mapRole->size()[%d]", _mapRole.size() );

	return 0;
}



//@brief	��ɫ�뿪��ͼ
//@return Role ����ֵ
RolePtr Map::ProcessRoleExitMap( UInt32 roleID )
{
	MutexLockGuard lock(mutex_);

	MAPROLE::iterator it;
	it = _mapRole.find(roleID);
	if( _mapRole.end() != it )
	{
		//���ض���ĸ�ֵ
		RolePtr pRole = it->second;

		//�����ۼ�����ʱ��
		it->second->UpdateTotalOnlineSec();
		//��¼��ɫ��������
		it->second->LastX( it->second->Pos().X);
		it->second->LastY( it->second->Pos().Y);

		//����д��DB
		it->second->Cache2DB();

		//�ͷŽ�ɫ��Դ
		_mapRole.erase(it);

		//�����뿪�����㲥
		ArchvRoute route;
		NotifyRoleMove( roleID, 2, route );

		return pRole;
	}
	else
	{
		LOG (LOG_ERROR, __FILE__, __LINE__, "RoleExitMap() error! role not found, roleID[%d]", roleID );
		return RolePtr(new Role());
	}

//DEBUG_PRINTF2("ProcessRoleExitMap success!!!-----roleID[%d] login, mapID[%d]", pRole->ID(), _mapID );
//DEBUG_PRINTF1("_mapRole.size()[%d]", _mapRole.size() );
}

//@brief	��ȡ��������
//@return RolePtr ����ֵ
RolePtr Map::ProcessGetRolePtr(UInt32 roleID) {
	MutexLockGuard lock(mutex_);

	MAPROLE::iterator it = _mapRole.find(roleID);
	if (_mapRole.end() != it) {
		return it->second;
	} else {
		LOG(LOG_ERROR, __FILE__, __LINE__,
				"ProcessGetRole() error! role not found, roleID[%d]", roleID);
		return RolePtr(new Role());
	}
}





//@brief	��ȡ������
//@return Monster ����ֵ
Monster Map::ProcessGetMonster( UInt32 mID )
{
	MutexLockGuard lock(mutex_);
	Monster monster;

	MAPMONSTER::iterator it;
	it = _mapMonster.find(mID);
	if( _mapMonster.end() != it )
	{
		monster = *(it->second);
	}
	else
	{
		LOG (LOG_ERROR, __FILE__, __LINE__, "ProcessGetMonster() error! monster not found, mID[%d]", mID );
	}

	return monster;
}


//@brief	��ɫ�ƶ�z
//@return Role ����ֵ
void Map::ProcessRoleMove( UInt32 roleID, ArchvRoute &art )
{
	MutexLockGuard lock(mutex_);

	MAPROLE::iterator it;
	it = _mapRole.find(roleID);
	if( _mapRole.end() == it )
	{
		LOG (LOG_ERROR, __FILE__, __LINE__, "ProcessRoleMove() error! role not found, roleID[%d]", roleID );
		return;
	}

	//�û��ƶ�·������ʼʱ�����Ϊ������ʱ��
	art.time = time(NULL);

	//�����ƶ�·��
	it->second->Route(art);

	//���õ�ǰ��λ
	it->second->Pos(art.listPos.front());

	//���㵱ǰ��λ
	it->second->CalcCurrPos();

	//����ͻ��ˡ�����˵�ǰλ�õ�������
	//	������Ҫ�����ͻ��˷���У���
	int iRet = 0;
	ArchvRoute currRoute = it->second->Route();
	ArchvPosition firstPos = art.listPos.front();
	ArchvPosition currPos = currRoute.listPos.front();
	double dDistance = CalcPosDistanceExact( firstPos , currPos );
	if( dDistance >= POSACCURACY )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"client currPos error!!!! roleID[%d], mapID[%d], dDistance[%f], moveSpeed[%d], client currPos.X[%d]Y[%d], server currPos.X[%d]Y[%d] ", roleID, _mapID, dDistance, ROLE_MOVE_SPEED, firstPos.X, firstPos.Y, currPos.X, currPos.Y );
		NotifyCorrectRoute( roleID, currRoute );
	}

	//���ͽ�ɫ�ƶ��㲥
	NotifyRoleMove( roleID, 1, art );

	return;
}



void Map::TradeInfo(TradeItem tradeItem,UInt32 roleID)
{
  MutexLockGuard lock(mutex_);
	MAPROLE::iterator it;
	it = _mapRole.find(roleID);
	if( _mapRole.end() == it )
	{
	  LOG (LOG_ERROR, __FILE__, __LINE__, "TradeInfo error! role not found, roleID[%d]", roleID );
	  return ;
	}

	it->second->TradeInfo(tradeItem);
}

UInt32 Map::FromNameTOFindID(string Name)
{
	MutexLockGuard lock(mutex_);

	MAPROLE::iterator itor;
	UInt32 RoleID=0;
	for( itor = _mapRole.begin(); itor != _mapRole.end(); ++itor )
	{
		if( Name == itor->second->Name() )
		{
			RoleID=itor->first;
			break;
		}
	}
	return RoleID;
}


//@brief	��ȡcoredata  ���ߵ����� roleID
void Map::GetRoleIDs( list<UInt32>& lrid )
{
	MutexLockGuard lock(mutex_);
	MAPROLE::iterator it;
	lrid.clear();
	for( it = _mapRole.begin(); it != _mapRole.end(); it++ )
	{
		lrid.push_back(it->first);
	}
}

//@brief	��ȡcoredata  ���ߵ����� roleID
//				���������� roleID ����
void Map::GetRoleIDs( UInt32 roleID, list<UInt32>& lrid )
{
	MutexLockGuard lock(mutex_);
	MAPROLE::iterator it;
	lrid.clear();
	for( it = _mapRole.begin(); it != _mapRole.end(); it++ )
	{
		if( it->first != roleID )
			lrid.push_back(it->first);
	}
}
ArchvPosition Map::GetSenceMonster(UInt32 MonsterID,list<SenceMonster>& Monsters)
{
	MutexLockGuard lock(mutex_);
	map<UInt32,SenceMonsterAll>::iterator it;
	SenceMonster monsster;
	ArchvPosition pos;

	it=_mapsenceMonster.find(MonsterID);
	if(_mapsenceMonster.end()==it)
	{
		pos.X=0;
		pos.Y=0;
		LOG (LOG_ERROR, __FILE__, __LINE__, "Donot find the Monster ,MonsterID is %d",MonsterID);
		return pos;
	}
	Monsters.clear();
	Monsters=it->second.Monsterp;
	monsster.MonsterType=(MonsterID%100000000)/10000;
	monsster.num=1;
	Monsters.push_front(monsster);
	pos.X=it->second.X;
	pos.Y=it->second.Y;
	return pos;

}


//@brief	[MsgType:0201]���㲥���ƶ��㲥�������롢�뿪������
//	��ɫ�ƶ���Ϣ�㲥
//@param	roleID	��ɫID
//@param	MoveType		�ƶ����ͣ�0���볡�� 1�ƶ� 2�뿪����
//@param	art	��ɫ�ƶ�·��
//@return	��
void Map::NotifyRoleMove( UInt32 roleID, Byte moveType, ArchvRoute &art )
{
	list<UInt32> lrid;
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	UInt32 speed = ROLE_MOVE_SPEED;

	//���ͷ
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType= 201;
	p.UniqID= 0;

	p.PackHeader();


	//�����
	s<<roleID<<moveType<<speed<<art;
	p.UpdatePacketLength();

	//��Ҫ�㲥���û��б�
	lrid.clear();
	MAPROLE::iterator itor;

	for( itor = _mapRole.begin(); itor != _mapRole.end(); ++itor )
	{
		//�����͸�����������û�
		if( roleID == itor->first )
			continue;

		lrid.push_back( itor->first );
	}


	if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error !!!");
	}

DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );

}

//@brief [MsgType:0202]�����������볡������������������ɫ���б�
//@param	roleID	��ɫID
//@return	��
void Map::NotifyAllRoleInfo( UInt32 roleID )
{
	list<UInt32>	lrid;
	List<ArchvRoleMoveDesc> larmd;
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	int iCount = 0;
	int iTmp = 0;
	ArchvRoleMoveDesc	armd;
DEBUG_PRINTF1( "roleID----[%d]\n", roleID );
	//����֪ͨ��Ŀ���û�
	lrid.push_back( roleID);

	//��ͼ�е��û�����
	MAPROLE::iterator itor;
	UInt32 currRoleID = 0;
	UInt32	Time = 0;

	for( itor = _mapRole.begin(); itor != _mapRole.end(); itor++ )
	{
		currRoleID = itor->first;
		iCount++;

		//Time�ֶ�����
		Time = time(NULL);

DEBUG_PRINTF1( "roleID[%d]===========>>>>>>>\n", roleID );

		//�û���Ϣ���뵽Ӧ���
		if( roleID != currRoleID )
		{
DEBUG_PRINTF2( "roleID[%d], art.listPos.size[%d]\n", roleID, itor->second->Route().listPos.size() );
			armd.roleID = currRoleID;
			armd.speed = ROLE_MOVE_SPEED;
 			armd.route = itor->second->Route();

			larmd.push_back( armd );
		}

		if( iTmp < RNUM_PER_SEND &&
			iCount < (int)_mapRole.size() )
		{
			iTmp++;
			continue;
		}
		else
		{
			iTmp = 0;
		}

		//��������������
		serbuffer.Reset();

		//���ͷ
		p.Direction = DIRECT_S_C_REQ;
		p.SvrType = 1;
		p.SvrSeq =1 ;
		p.MsgType= 202;
		p.UniqID= 0;

		p.PackHeader();

		//�����
		s<<Time<<larmd;
		p.UpdatePacketLength();

		//��������
		if(_mainSvc->Service()->Broadcast( lrid, &serbuffer))
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!!" );
		}

DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
		DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );


		//����û�״̬�б�
		lrid.clear();
	}


}

//@brief [MsgType:206] ��ɫ����֪ͨ
//@param roleID �����Ľ�ɫ
void Map::NotifyRoleAddLevl(UInt32 roleID)
{
    List<UInt32> lrid;
	List<ArchvRoleMoveDesc> larmd;
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);

	//��ͼ�е��û��б�
	MAPROLE::iterator itor;

	for(itor = _mapRole.begin();itor != _mapRole.end(); itor++)
	{
	    lrid.push_back(itor->first);
	}

	//��������������
	serbuffer.Reset();

	//���ͷ
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq =1 ;
	p.MsgType= 206;
	p.UniqID= 0;

	p.PackHeader();

	//�����
	s<<roleID;
	p.UpdatePacketLength();

	//��������
	if(_mainSvc->Service()->Broadcast( lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!!" );
	}

	//��յ�ͼ�е��û��б�
	lrid.clear();

}





//@brief [MsgType:0203]���������ƶ�·������֪ͨ
//	�������͸�������ҽ�ɫ
//@param	roleID	��ɫID
//@return	��
void Map::NotifyCorrectRoute( UInt32 roleID, ArchvRoute &rt )
{
	list<UInt32> lrid;
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();


	//���ͷ
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType= 203;
	p.UniqID= 0;

	p.PackHeader();

	//�·���ɫID
	lrid.push_back( roleID );

	//�����
	s<<rt;
	p.UpdatePacketLength();

	if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error !!!");
	}

DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}




//@brief  [MsgType:0204]�������������ɫ���֡����״̬��Npcͷ��״̬
//	�������͸���ҽ�ɫ
//@param	roleID	��ɫID
//@param	level		��ɫ����
//@param	flag		���ͱ�־
//								flag& 0x01		���ͽ�ɫ״̬
//								flag& 0x02		���͹�״̬
//								flag& 0x04		����npcͷ��״̬
//@return	��
void Map::NotifyCtStatusUnicast( UInt32 roleID, Byte flag)
{
	MutexLockGuard lock(mutex_);
	list<UInt32> lrid;
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();

	List<ArchvCreatureStatus> lbs;

	//���ͷ
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType= 204;
	p.UniqID= 0;

	p.PackHeader();

	//�Ƿ���Ҫ�㲥
	lrid.clear();
	lrid.push_back( roleID );

	//ȡ��ɫ���֡�״̬
	MAPROLE::iterator it1;
	MapMonsters::iterator it2;
	MAPNPC::iterator it3;
	ArchvCreatureStatus bs;

	//��ȡ��ɫ״̬
	if( flag&0x01 )
	{
		for( it1 = _mapRole.begin(); it1 != _mapRole.end(); it1++ )
		{
			bs.creatureFlag = 1;
			bs.creatureType = it1->second->ProID();
			bs.ID = it1->first;
			bs.status = it1->second->Status();
			lbs.push_back(bs);
		}
	}

	//��ȡ��״̬
	if( flag&0x02 )
	{
		for( it2 = _mapsenceMonster.begin(); it2 != _mapsenceMonster.end(); it2++ )
		{
			bs.creatureFlag = 2;
			bs.creatureType = it2->second.MonsterType;
			bs.ID = it2->second.MonsterID;
			bs.status = it2->second.Status;
			lbs.push_back(bs);
		}
	}

	//��ȡ Npc ״̬
	if( flag&0x04 )
	{
		Byte status = 0;
		MAPROLE::iterator itTmp;
		itTmp = _mapRole.find(roleID);
		if( _mapRole.end() == itTmp )
		{
			LOG (LOG_ERROR, __FILE__, __LINE__, "_mapRole.find error! role not found, roleID[%d]", roleID );
			return;
		}

		for( it3 = _mapNpc.begin(); it3 != _mapNpc.end(); it3++ )
		{
			bs.creatureFlag = 3;
			bs.creatureType = it3->second->Type();
			bs.ID = it3->first;

			_mainSvc->GetNpcSvc()->GetNpcStatus( roleID, itTmp->second->Level(), it3->second->ID(), status);
			if( status <= 3 )
			{
				bs.status = status + 10;
				lbs.push_back(bs);
			}
		}
	}


	//�����
	s<<lbs;
	p.UpdatePacketLength();

	if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error !!!");
	}

DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}




//@brief [MsgType:0204]���㲥�������ɫ���֡����״̬��Npcͷ��״̬
//	�����͸�״̬�ı��߱���
//@param	lbs				����״̬�����б�
//@return	��
void Map::NotifyCtStatusBroadcast( List<ArchvCreatureStatus> &lcs )
{
	list<UInt32> lrid;
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();

	//���ͷ
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType= 204;
	p.UniqID= 0;

	p.PackHeader();

	//�����
	s<<lcs;
	p.UpdatePacketLength();

	//�Ƿ���Ҫ�㲥
	lrid.clear();
	MAPROLE::iterator it;
	List<ArchvCreatureStatus>::iterator itCs;

	for( it = _mapRole.begin(); it != _mapRole.end(); ++it )
	{
		//�����͸�״̬�ı��߱���
		for( itCs = lcs.begin(); itCs != lcs.end(); itCs++ )
			if( 1 == itCs->creatureFlag && it->first == itCs->ID )
				continue;

		lrid.push_back( it->first );
	}

	if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error !!!");
	}

DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}

//S-C��Ѫ����������
void Map::NotifyHPandMP(UInt32 roleID, UInt32 hp, UInt32 mp)
{
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 305;
	p.UniqID = 251;
	p.PackHeader();
	List<UInt32> lrid;
	s<<hp<<mp;

//	LOG(LOG_ERROR,__FILE__,__LINE__,"%d,%d !!",hp,mp);
	p.UpdatePacketLength();
	lrid.push_back(roleID);
	if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!");
	}
	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}


int Map::GetallTheNum(UInt32 ID,list<UInt32> lis)
{
	int flag=0;
	list<UInt32>::iterator it;
	for(it=lis.begin();it!=lis.end();it++)
	{
		if(ID==*it)
		{
			flag=1;
			break;
		}

	}
	return flag;
}

void Map::GiveVipGiftOnEnterMap(RolePtr& role)
{
	int vipLevel = role->ISVIP();
	if (vipLevel >= 0 && vipLevel <= 6)
	{
		time_t t = time(0);
		struct tm * timeinfo = localtime (&t);
		timeinfo->tm_hour = 0;
		timeinfo->tm_min = 0;
		timeinfo->tm_sec = 0;
		time_t today = mktime(timeinfo);

		Connection con;
		DBOperate dbo;
		con = _cp->GetConnection();
		dbo.SetHandle(con.GetHandle());

		char szSql[1024];
		sprintf(szSql, "select DisseminationTime from VipGiftDissemination where RoleID = %d;",
				role->ID());
		int iRet = dbo.QuerySQL(szSql);
		bool bDisseminationOnce = false;

		if(iRet<0)
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			return;
		}
		int DisseminationTime = 0;
		while (dbo.HasRowData())
		{
			DisseminationTime = dbo.GetIntField(0);
			bDisseminationOnce = true;
			dbo.NextRow();
		}
		//�µ�һ�죬��������
		if (today != (time_t)-1 && DisseminationTime < today)
		{
			sprintf(szSql, "select VipLevel, ItemID, ItemNum from VIPItemGiftDesc where VipLevel = %d;",
				vipLevel);

			iRet = dbo.QuerySQL(szSql);
			if(iRet==1)
			{//
				LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL Not find[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
				return;
			}
			if(iRet<0)
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
				return;
			}

			int itemID = 0;
			int itemNum = 0;
			bool bDisseminate = false;
			while (dbo.HasRowData())
			{
				itemID = dbo.GetIntField(1);
				itemNum = dbo.GetIntField(2);
				ArchvSystemMailItem mail;
				mail.RecvRoleName = role->Name();
				mail.Content = "System Mail";
				mail.Money = 0;
				mail.MoneyType = 0;
				mail.ItemID = itemID;
				mail.Num = itemNum;
				mail.EntityID = 0;
				_mainSvc->GetMailSvc()->OnSendSystemMail(mail);
				bDisseminate = true;
				dbo.NextRow();
			}

			if (bDisseminate)
			{
				if (bDisseminationOnce)
				{
					sprintf(szSql, "update VipGiftDissemination set DisseminationTime = %d where RoleID = %d;",
						time(NULL),role->ID());
				}
				else
				{
					sprintf(szSql, "insert into VipGiftDissemination (RoleID,DisseminationTime) values(%d, %d);",
						role->ID(), time(NULL));
				}
				iRet = dbo.ExceSQL(szSql);
				if(iRet != 0)
				{
					LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error szSql[%s] ",szSql);
				}
			}
		}
	}
}


