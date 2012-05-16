//ҵ���� server   �������

#ifndef TASKSVC_H
#define TASKSVC_H

#include "GWProxy.h"
#include "IniFile.h"
#include "ConnectionPool.h"
#include "LuaState.h"
#include "ArchvBagItemCell.h"
#include "ArchvTask.h"

class MainSvc;
class ArchvTaskDetail;

class TaskDetail
{
public:
	TaskDetail():detailID(0),taskID(0),goalNum(0)
	{
	}

public:
	UInt32	detailID;
	UInt32	taskID;
	UInt32	goalNum;
};


class TaskSvc
{
public:
	TaskSvc(void* service, ConnectionPool * cp);
	~TaskSvc();

 	void OnProcessPacket(Session& session,Packet& packet);

	void ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode);

	void OnAfterKillMonster( UInt32 roleID, UInt32 monsterType, UInt32 killNum );
	void OnBagItemAddOrDelete( UInt32 roleID, UInt32 itemID );

	void OnUseItem( UInt32 roleID, UInt32 itemID );
	void OnRoleDress( UInt32 roleID, UInt32 itemID );
	void OnCallPet( UInt32 roleID, UInt32 petType );
	void OnRoleLearnSkill( UInt32 roleID );
	void OnComposeEquip( UInt32 roleID );
	void OnRoleAddFriend( UInt32 roleID );
	void OnRoleAddTeam( UInt32 roleID );

	int ExecCanAcceptScript( UInt32 roleID, const string& script );

	int ExecToDoScript( UInt32 roleID, UInt32 taskID, const string& script );

	int ExecAfterDeliverScript( UInt32 roleID, const string& script );

	void CheckAllItemTypeTaskStatus(UInt32 roleID);

	void UpdateTaskDetailStatusByHoldItem( UInt32 roleID, UInt32 itemID );

	void UpdateTaskDetailStatusByNum( UInt32 roleID, Byte goalType, UInt32 goalID, UInt32 inputNum );

	void UpdateTaskStatus( UInt32 roleID, UInt32 taskID );

	int GetRoleProID( UInt32 roleID, UInt32&proID );

	int GetRoleTaskStatus( UInt32 roleID, UInt32 taskID, Byte &status );

	int GetRoleTaskDetail( UInt32 roleID, UInt32 taskID, List<ArchvTaskDetail>& ltd );

	int AddRoleTaskDetail( UInt32 roleID, UInt32 taskID, Byte goalType, UInt32 goalID, UInt32 goalNum );

	int AddRoleExp( UInt32 roleID, UInt32 exp );

	int AddRoleMoney( UInt32 roleID,UInt32 money );

	int AddRoleBindMoney( UInt32 roleID,UInt32 money );

	int AddRoleItem( UInt32 roleID , UInt32 ItemID,UInt32 num);


	int DeleteItem(UInt32 roleID,UInt32 ItemID,UInt32 num);

	void GetRoleCanAcceptTask(UInt32 roleID, List<ArchvTaskInfo>& lti);

	int GetRoleUnFinishedTask(list<UInt32>licRoleID,List<ArchvUnfinishedTask>&licRoleTask);

	//------------S_C ����-----------------------------------------
	//[MsgType:0701]������ϸ��״̬֪ͨ������������״̬Ϊδ��ɡ������ʱ��
	void NotifyTaskStatus( UInt32 roleID, UInt32 taskID );

	//[MsgType:0702]��ɫ�ɽ��������֪ͨ��������ÿ�ν�������ʱ����
	void NotifyTaskCanAccept( UInt32 roleID );

	void callNotifybag(UInt32 roleID,List < ItemCell > & lic,Byte flag);

	//------------C_S ����-----------------------------------------
	//[MsgType:0701]��ɫ�����б�
	void ProcessGetRoleTask(Session& session,Packet& packet);

	//[MsgType:0703]��������
	void ProcessAcceptTask(Session& session,Packet& packet);

	//[MsgType:0704]��������
	void ProcessDeliverTask(Session& session,Packet& packet);

	//[MsgType:0705]��������
	void ProcessAbandonTask(Session& session,Packet& packet);

	//------------��ҵ���� -----------------------------------------



private:
	MainSvc * _mainSvc;
	IniFile _file;
	ConnectionPool *_cp;

	LuaState _luaState;

};


#endif

