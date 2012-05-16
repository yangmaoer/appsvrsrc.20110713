#include "TaskSvc.h"
#include "MainSvc.h"
#include "TaskLua.h"
#include "DBOperate.h"
#include "ArchvTask.h"
#include "CoreData.h"
#include "Role.h"
#include "./Bag/BagSvc.h"
#include "./RoleInfo/RoleInfoSvc.h"

using namespace TaskLua;


TaskSvc::TaskSvc(void* service, ConnectionPool * cp)
{
	_mainSvc = (MainSvc*)(service);
	_cp = cp;
}

TaskSvc::~TaskSvc()
{
}

void TaskSvc::OnProcessPacket(Session& session,Packet& packet)
{

DEBUG_PRINTF1( "C_S req pkg-------MsgType[%d] \n", packet.MsgType );
	DEBUG_SHOWHEX( packet.GetBuffer()->GetReadPtr()-PACKET_HEADER_LENGTH, packet.GetBuffer()->GetDataSize()+PACKET_HEADER_LENGTH, 0, __FILE__, __LINE__ );

	switch(packet.MsgType)
	{
		case 701: //��ɫ�����ѯ
			ProcessGetRoleTask(session,packet);
			break;

		case 703: //��������
			ProcessAcceptTask(session,packet);
			break;

		case 704: //��������
			ProcessDeliverTask(session,packet);
			break;

		case 705: //��������
			ProcessAbandonTask(session,packet);
			break;


		default:
			ClientErrorAck(session,packet,ERR_SYSTEM_PARAM);
			LOG(LOG_ERROR,__FILE__,__LINE__,"MsgType[%d] not found",packet.MsgType);
			break;
		}
}


//�ͻ��˴���Ӧ��
//@param  session ���Ӷ���
//@param	packet �����
//@param	RetCode ����errorCode ֵ
void TaskSvc::ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode)
{
	//��Ӧ������
	DataBuffer	serbuffer(1024);
	Packet p(&serbuffer);
	Serializer s(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_C_S_RESP;
	p.PackHeader();

	s<<RetCode;
	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF1( "C_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );

}



//@brief	�¼�����:��ֽ���
//				goalType 1
//@param	roleID				��ɫID
//@param	monsterType		������
//@param	killNum				ɱ������
//@return	��
void TaskSvc::OnAfterKillMonster( UInt32 roleID, UInt32 monsterType, UInt32 killNum )
{
	UpdateTaskDetailStatusByNum( roleID, 1, monsterType, killNum );
}


//@brief	�¼�����:������Ʒ��ɾ
//				goalType 2
//@param	roleID				��ɫID
//@param	itemID				��ƷID
//@return	��
void TaskSvc::OnBagItemAddOrDelete( UInt32 roleID, UInt32 itemID )
{
	UpdateTaskDetailStatusByHoldItem( roleID, itemID );
}


//@brief	�¼�����:��Ʒʹ��
//				goalType 3
//@param	roleID				��ɫID
//@param	itemID				��ƷID
//@return	��
void TaskSvc::OnUseItem( UInt32 roleID, UInt32 itemID )
{
	UpdateTaskDetailStatusByNum( roleID, 3, itemID , 1 );
}

//@brief	�¼�����:��װ��
//				goalType 4
//@param	roleID				��ɫID
//@param	itemID				��ƷID
//@return	��
void TaskSvc::OnRoleDress( UInt32 roleID, UInt32 itemID )
{
	UpdateTaskDetailStatusByNum( roleID, 4, itemID , 1 );
}


//@brief	�¼�����:�ٻ�����
//				goalType 5
//@param	roleID				��ɫID
//@param	petType				��������
//@return	��
void TaskSvc::OnCallPet( UInt32 roleID, UInt32 petType )
{
	UpdateTaskDetailStatusByNum( roleID, 5, petType , 1 );
}

//@brief	�¼�����:ѧϰ����
//				goalType 6
//@param	roleID				��ɫID
//@return	��
void TaskSvc::OnRoleLearnSkill( UInt32 roleID )
{
	UpdateTaskDetailStatusByNum( roleID, 6, 0 , 1 );
}



//@brief	�¼�����:װ���ϳ�
//				goalType 7
//@param	roleID				��ɫID
//@return	��
void TaskSvc::OnComposeEquip( UInt32 roleID )
{
	UpdateTaskDetailStatusByNum( roleID, 7, 0 , 1 );
}

//@brief	�¼�����:��Ӻ���
//				goalType 8
//@param	roleID				��ɫID
//@return	��
void TaskSvc::OnRoleAddFriend( UInt32 roleID )
{
	UpdateTaskDetailStatusByNum( roleID, 8, 0 , 1 );
}


//@brief	�¼�����:��Ӳ���
//				goalType 9
//@param	roleID				��ɫID
//@return	��
void TaskSvc::OnRoleAddTeam( UInt32 roleID )
{
	UpdateTaskDetailStatusByNum( roleID, 9, 0 , 1 );
}


//ִ��������������ű�
//@return 0 �ɹ�,����ɽ�   ��0 ִ�нű�ʧ�ܣ���������������
int TaskSvc::ExecCanAcceptScript( UInt32 roleID, const string& script )
{

	if( script.size() == 0 )
		return 0;

	//ִ�нű�
	lua_State* state = _luaState.GetState(RegisterTaskCAPI);

	SET_TASKSVC(state,this);
	SET_ROLEID(state, roleID );
	if(LuaState::Do(state, script))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"exec lua error, roleID[%d], script[%s], errorMsg[%s]",			roleID, script.c_str(), LuaState::GetErrorMsg(state));
		return -1;
	}

	return GetLuaRetCode(state);

}

//ִ������to-do�ű�
//@return 0 �ɹ�,����ɽ�   ��0 ִ�нű�ʧ�ܣ���������������
int TaskSvc::ExecToDoScript( UInt32 roleID, UInt32 taskID, const string& script )
{

	if( script.size() == 0 )
		return 0;

	//ִ�нű�
	lua_State* state = _luaState.GetState(RegisterTaskCAPI);

	SET_TASKSVC(state,this);
	SET_ROLEID(state, roleID );
	SET_TASKID(state, taskID );
	if(LuaState::Do(state, script))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"exec lua error, roleID[%d], script[%s], errorMsg[%s]",			roleID, script.c_str(), LuaState::GetErrorMsg(state));
		return -1;
	}

	return GetLuaRetCode(state);

}




//ִ�����񽻸���Ľű�
//@return 0 �ɹ�,����ɽ�   ��0 ִ�нű�ʧ�ܣ���������������
int TaskSvc::ExecAfterDeliverScript( UInt32 roleID, const string& script )
{

	if( script.size() == 0 )
		return 0;

	//ִ�нű�
	lua_State* state = _luaState.GetState(RegisterTaskCAPI);

	SET_TASKSVC(state,this);
	SET_ROLEID(state, roleID );
	if(LuaState::Do(state, script))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"exec lua error, roleID[%d], script[%s], errorMsg[%s]",			roleID, script.c_str(), LuaState::GetErrorMsg(state));
		return -1;
	}

	return GetLuaRetCode(state);

}

//@brief	���ݽ�ɫ������Ʒ,������������״̬
//@param	roleID				��ɫID
//@return	��
void TaskSvc::CheckAllItemTypeTaskStatus(UInt32 roleID)
{
	char szSql[1024];
	Connection con,conSub;
	DBOperate dbo,dboSub;
	int iRet = 0;
	list<UInt32> lItemID;

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//���ұ�����Ʒ�б�
	sprintf( szSql, "select distinct(ItemID) \
									from Bag \
									where RoleID = %d ", roleID );
	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		return;
	}
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return;
 	}

 	while(dbo.HasRowData())
	{
		lItemID.push_back( dbo.GetIntField(0) );

	 	//��¼����һ����¼
		dbo.NextRow();
	}

	list<UInt32>::iterator it;
	for( it = lItemID.begin(); it != lItemID.end(); it++ )
	{
//LOG(LOG_ERROR,__FILE__,__LINE__,"UpdateTaskDetailStatusByHoldItem  roleID[%d],itemID[%d] ", roleID, *it);
		UpdateTaskDetailStatusByHoldItem( roleID, *it );
	}

}


//@brief	���ݱ�����Ʒ����(��Ʒ����),��������ϸ��״̬
//@param	roleID
//@param	itemID
//@return	��
void TaskSvc::UpdateTaskDetailStatusByHoldItem( UInt32 roleID, UInt32 itemID )
{
	char szSql[1024];
	Connection con,conSub;
	DBOperate dbo,dboSub;
	int iRet = 0;
	UInt32 bagItemNum = 0;

	TaskDetail td;
	list<TaskDetail> ltd;
	list<TaskDetail>::iterator it;

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	conSub = _cp->GetConnection();
	dboSub.SetHandle(conSub.GetHandle());

DEBUG_PRINTF2( "UpdateTaskDetailStatusByHoldItem ----- roleID[%d], itemID[%d] \n", roleID, itemID );

	//��Ʒ��ص�������ϸ
	sprintf( szSql, "select DetailID, TaskID, GoalNum \
									from RoleTaskDetail \
									where RoleID = %d \
										and GoalType = 2 \
										and GoalID = %d \
									order by DetailID \
								", roleID, itemID );
	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		//��������Ʒ��ص�����,ֱ�ӷ���
		return;
	}
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return;
 	}

	while(dbo.HasRowData())
	{
		td.detailID = dbo.GetIntField(0);
		td.taskID = dbo.GetIntField(1);
		td.goalNum = dbo.GetIntField(2);

		ltd.push_back(td);

	 	//��¼����һ����¼
		dbo.NextRow();
	}


	//���ý�ɫ����Ʒ��ص�����ϸ��,��Ϊ��ʼ״̬
	//	��״̬Ϊδ���,�������Ϊ0
	sprintf( szSql, "update RoleTaskDetail \
									set IsFinish = 0, \
										FinishNum = 0 \
									where RoleID = %d \
										and GoalType = 2 \
										and GoalID = %d ", roleID, itemID );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return;
 	}

	//��ȡ�����и���Ʒ��������
	sprintf( szSql, "select Num \
									from Bag \
									where RoleID = %d \
										and ItemID = %d ", roleID, itemID );
	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		return;
	}
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return;
 	}

	while(dbo.HasRowData())
	{
		bagItemNum += dbo.GetIntField(0);

	 	//��¼����һ����¼
		dbo.NextRow();
	}



LOG(LOG_ERROR,__FILE__,__LINE__,"bagItemNum[%d] ", bagItemNum);

	//����Ʒ����,���·��������ϸ��
	for( it = ltd.begin(); it != ltd.end(); it++ )
	{
		if( bagItemNum < it->goalNum )
		{//��Ʒ��������,���¸�����ϸ���,ֹͣ����
 			sprintf( szSql, "update RoleTaskDetail \
											set IsFinish = 0, \
												FinishNum = %d \
											where DetailID = %d ", bagItemNum, it->detailID );
			iRet = dbo.ExceSQL(szSql);
			if( iRet < 0 )
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
				return;
		 	}

			break;
		}
		else
		{//���¸�����ϸ��Ϊ�����
			sprintf( szSql, "update RoleTaskDetail \
											set IsFinish = 1, \
												FinishNum = %d \
											where DetailID = %d ", it->goalNum, it->detailID );
			iRet = dbo.ExceSQL(szSql);
			if( iRet < 0 )
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
				return;
		 	}

		 	bagItemNum -= it->goalNum;
		}

		//���� UpdateTaskStatus
		UpdateTaskStatus( roleID, it->taskID );

	}

	//����״̬֪ͨ
	for( it = ltd.begin(); it != ltd.end(); it++ )
	{
		NotifyTaskStatus( roleID, it->taskID );
	}

}






//@brief	��������Ŀ�����ɴ���, ��������ϸ��״̬
//@param	roleID				��ɫID
//@param	goalType			Ŀ������
//@param	goalID				Ŀ��ID
//@param	inputNum			������ɵĴ���
//@return	��
void TaskSvc::UpdateTaskDetailStatusByNum( UInt32 roleID, Byte goalType, UInt32 goalID, UInt32 inputNum )
{
	char szSql[1024];
	Connection con,conSub;
	DBOperate dbo,dboSub;
	int iRet = 0;
	UInt32 detailID = 0;
	UInt32 taskID = 0;
	UInt32 goalNum = 0;
	UInt32 finishNum = 0;
	list<UInt32> lTaskID;

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	conSub = _cp->GetConnection();
	dboSub.SetHandle(conSub.GetHandle());

DEBUG_PRINTF4( "UpdateTaskDetailStatusByNum ----- roleID[%d], goalType[%d], goalID[%d],inputNum[%d]  \n", roleID, goalType, goalID, inputNum );

	//����У��
	if( 0 == inputNum )
		return;

	//������ص�������ϸ
	sprintf( szSql, "select DetailID, TaskID, GoalNum, FinishNum \
									from RoleTaskDetail rtd \
									where RoleID = %d \
										and GoalType = %d \
										and GoalID = %d \
										and IsFinish = 0 ", roleID, goalType, goalID);
	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		return;
	}
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return;
 	}

 	while(dbo.HasRowData())
	{
		detailID = dbo.GetIntField(0);
		taskID = dbo.GetIntField(1);
		goalNum = dbo.GetIntField(2);
		finishNum = dbo.GetIntField(3);

		lTaskID.push_back(taskID);

		//�Ƿ�ﵽĿ��
		if( (finishNum+inputNum) < goalNum )
		{
			//���������������
			sprintf( szSql, " update RoleTaskDetail \
												set finishNum = %d \
												where DetailID = %d "
												, finishNum+inputNum, detailID );
			iRet = dboSub.ExceSQL(szSql);
			if( iRet < 0 )
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(conSub.GetHandle()), szSql);
				return;
		 	}
		}
		else
		{
			//��������ϸ��Ϊ�����
			sprintf( szSql, " update RoleTaskDetail \
												set IsFinish = 1, \
													finishNum = %d \
												where DetailID = %d "
												, goalNum, detailID );
			iRet = dboSub.ExceSQL(szSql);
			if( iRet < 0 )
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(conSub.GetHandle()), szSql);
				return;
		 	}

			//���� UpdateTaskStatus
			UpdateTaskStatus( roleID, taskID);
		}

	 	//��¼����һ����¼
		dbo.NextRow();
	}

	//����״̬֪ͨ
	list<UInt32>::iterator it;
	for( it = lTaskID.begin(); it != lTaskID.end(); it++ )
		NotifyTaskStatus( roleID, *it );
}



//@brief ����״̬����
//	���ݽ�ɫ����ϸ�� RoleTaskDetail ��������
//	������������ RoleTask ������״̬Ϊ 'δ���' �� '�����' �� '�ѽ���'
//@param	roleID	��ɫID
//@param	taskID	����ID
//@return ��
void TaskSvc::UpdateTaskStatus( UInt32 roleID, UInt32 taskID )
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	Byte status = 0;
	int	hasNoTaskDetail = 0;
	int isChange2Finished = 0;				//����״̬,��������
	UInt32 exitNpcID = 0;
	Byte finishStatus = 0;
	Byte addDeliverNum = 0;

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//����״̬��ѯ
	iRet = GetRoleTaskStatus( roleID, taskID, status );
	if( iRet )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"GetRoleTaskStatus error" );
		return;
	}

	//����״̬�ж�
	//	״̬����Ϊδ�� �� �ѽ���
	if( status <= 1 || status >= 4 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"role status error, status[%d]", status );
		return;
	}

	//ȡ������ϸ
	sprintf( szSql, "select   RoleID, TaskID \
									from RoleTaskDetail \
									where RoleID = %d \
										and TaskID = %d ", roleID, taskID  );
	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		hasNoTaskDetail = 1;
	}
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return;
 	}

 	//ȡ���� Npc
 	sprintf( szSql, "select ExitNpcID \
									from Task \
									where TaskID = %d ", taskID  );
	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s]; task not exist!!, taskID[%d] " , szSql, taskID);
		return;
	}
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return;
 	}
 	exitNpcID = dbo.GetIntField(0);

 	//�Ƿ���������ϸ
 	if( hasNoTaskDetail )
 	{//û��������ϸ,����̸��������

		if( 0 == exitNpcID )
		{//���轻�� npc������ֱ����Ϊ�ѽ���
			finishStatus = 2;
			addDeliverNum = 1;
		}
		else
		{//�轻�� npc������ֱ����Ϊ�����
			finishStatus = 1;
			addDeliverNum = 0;
		}

 		sprintf( szSql, "update RoleTask \
		 									set FinishStatus = %d, \
		 											DeliverNum = DeliverNum + %d \
											where RoleID = %d \
												and TaskID = %d ", finishStatus, addDeliverNum, roleID, taskID );
		iRet = dbo.ExceSQL(szSql);
		if( iRet < 0 )
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			return;
	 	}
 	}
 	else
 	{//��������ϸ,�жϸ�����ϸ���������޸� ����״̬
 		int isAllDetailFinish = 0;
		sprintf( szSql, "select DetailID \
									from RoleTaskDetail \
									where RoleID = %d \
										and TaskID = %d \
										and IsFinish = 0 ", roleID, taskID  );
		iRet = dbo.QuerySQL(szSql);
		if( iRet < 0 )
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			return;
	 	}
	 	if( 1 == iRet )
		{
			isAllDetailFinish = 1;
		}
		else
		{
			isAllDetailFinish = 0;
		}

		//����������ϸ�����
		if( isAllDetailFinish )
		{
			if( 0 == exitNpcID )
			{//���轻�� npc������ֱ����Ϊ�ѽ���
				finishStatus = 2;
				addDeliverNum = 1;
			}
			else
			{//�轻�� npc������ֱ����Ϊ�����
				finishStatus = 1;
				addDeliverNum = 0;
			}

			sprintf( szSql, "update RoleTask \
		 									set FinishStatus = %d, \
		 											DeliverNum = DeliverNum + %d \
											where RoleID = %d \
												and TaskID = %d ", finishStatus, addDeliverNum, roleID, taskID );
			iRet = dbo.ExceSQL(szSql);
			if( iRet < 0 )
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
				return;
		 	}
		}

 	}

 	//ȡ����״̬���
	sprintf( szSql, "select FinishStatus \
									from RoleTask \
									where RoleID = %d \
										and TaskID = %d ", roleID, taskID  );
	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s]; task not exist!!, taskID[%d] " , szSql, taskID);
		return;
	}
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return;
 	}
 	status = dbo.GetIntField(0);

	//״̬Ϊ ��������� S_C ֪ͨ
	if( 1 == status )
	{
		_mainSvc->GetCoreData()->NotifyNpcStatus(roleID);	//[MsgType:0204](����) Npcͷ��״̬
	}

}


//@brief ���ҽ�ɫ��ְҵ
//@param	roleID	��ɫID
//@param	proID	���صĽ�ɫְҵ
//@return 0 �ɹ�  ��0 ʧ��
int TaskSvc::GetRoleProID( UInt32 roleID, UInt32&proID )
{
	proID = 0;
	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
	if(pRole->ID() == 0)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole error! roleID[%d] ", roleID );
		return -1;
	}

	proID = pRole->ProID();

	return 0;
}


//@brief ���ҽ�ɫ������״̬
//@param	roleID	��ɫID
//@param	taskID	����ID
//@param	status	���ص� ����״̬	1 δ��  2  δ���  3 �����  4 �ѽ���
//@return 0 �ɹ�  ��0 ʧ��
int TaskSvc::GetRoleTaskStatus( UInt32 roleID, UInt32 taskID, Byte &status )
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	Byte taskType = 0;
	UInt32 maxAcceptNum = 0;
	UInt32 maxLevel = 0;


	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//��ʼ��
	status = 0;

	//�鿴�����Ƿ����
	sprintf( szSql, "select   TaskType, MaxAcceptNum, MaxLevel \
									from Task \
									where TaskID = %d ", taskID  );
	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s]; task not exist!!, taskID[%d] " , szSql, taskID);
		return -1;
	}
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}

 	while(dbo.HasRowData())
	{
	 	taskType = dbo.GetIntField(0);
	 	maxAcceptNum = dbo.GetIntField(1);
	 	maxLevel = dbo.GetIntField(2);

	 	//��¼����һ����¼
		dbo.NextRow();
	}


	//���ҽ�ɫ����
	sprintf( szSql, "select FinishStatus, DeliverNum \
									from RoleTask \
									where RoleID = %d \
										and TaskID = %d ", roleID, taskID );
	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		status = 1;	//����δ��
		return 0;
 	}
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}

	while(dbo.HasRowData())
	{
		int finishStatus = dbo.GetIntField(0);
		int deliverNum = dbo.GetIntField(1);

		if( TASKFINISHSTATUS_NOTFINISHED == finishStatus )
		{
			status = 2;			//δ���
		}
		else if( TASKFINISHSTATUS_FINISHED == finishStatus )
		{
			status = 3;			//�����
		}
		else if( TASKFINISHSTATUS_DELIVERED == finishStatus )
		{
			if( TASKTYPE_DAILY == taskType )
			{//�ճ�����

				//���ɫ��Ϣ
				RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
				if(pRole->ID()==0)
				{
					LOG(LOG_ERROR,__FILE__,__LINE__,"role not found! roleID[%d] ", roleID );
					return -1;
				}

				//�Ƿ�ɷ���Ϊδ��״̬
				//	δ���ս�������,��û�г�����󼶱�					����δ��
				//	���򷵻��ѽ���
				if( deliverNum < maxAcceptNum &&
						pRole->Level() <= maxLevel )
					status = 1;		//δ��
				else
					status = 4;			//�ѽ���
			}
			else
				status = 4;			//�ѽ���
		}

		//��¼����һ����¼
		dbo.NextRow();
	}


	return 0;
}


//@brief ���ҽ�ɫ����ϸ��
//@return 0 �ɹ�  ��0 ʧ��
int TaskSvc::GetRoleTaskDetail( UInt32 roleID, UInt32 taskID, List<ArchvTaskDetail>& ltd )
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	Byte taskType = 0;
	ArchvTaskDetail taskDetail;


	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//��ȡ����
	sprintf( szSql, " select GoalType, GoalID, IsFinish, GoalNum, FinishNum \
											from RoleTaskDetail \
											where RoleID = %d \
											and TaskID = %d ",
											roleID, taskID );
	iRet = dbo.QuerySQL(szSql);
 	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}

	while(dbo.HasRowData())
	{
		//��������
		taskDetail.goalType = dbo.GetIntField(0);
		taskDetail.goalID = dbo.GetIntField(1);
		taskDetail.isFinish = dbo.GetIntField(2);
		taskDetail.goalNum = dbo.GetIntField(3);
		taskDetail.finishNum = dbo.GetIntField(4);

		ltd.push_back( taskDetail );

		//��¼����һ����¼
		dbo.NextRow();
	}

	return 0;
}


//@brief ����������ϸ
//	�� C API for lua  ����
//@param	roleID	��ɫID
//@param	taskID	����ID
//@goalType		Ŀ������ 1 ���  2 ��Ʒ��ȡ  3 ��Ϸ���� 4  �������  5 ��װ�� 6 ��Ʒʹ�� 7�̳ǹ���
//@goalID			Ŀ��ID
//			       ��Ŀ��������أ��ֵ�����ID����ƷID����Ϸ����
//@goalNum		Ŀ������
//@return 0 �ɹ�  ��0 ʧ��
int TaskSvc::AddRoleTaskDetail( UInt32 roleID, UInt32 taskID, Byte goalType, UInt32 goalID, UInt32 goalNum )
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//������ϸ
	sprintf( szSql, " insert into RoleTaskDetail(DetailID, RoleID, TaskID, \
											GoalType, GoalID, IsFinish, GoalNum, FinishNum ) \
										values(NULL, %d, %d, %d, %d, 0, %d, 0) "
										, roleID, taskID, goalType, goalID, goalNum );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}


	return 0;
}


void TaskSvc::ProcessGetRoleTask(Session& session,Packet& packet)
{
 	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	UInt32 roleID = packet.RoleID;
	ArchvTaskInfo taskInfo;
	List<ArchvTaskInfo> lti;
	List<ArchvTaskInfo>::iterator it;
	ArchvTaskDetail taskDetail;

	//���л���
	Serializer s(packet.GetBuffer());
//	s>>RoleID;

	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//��ȡ��ɫ��Ϣ
	if( 0 == pRole->ID() )
	{
		RetCode = ERR_APP_ROLENOTEXISTS;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole error,role not exists, roleID[%d]", roleID  );
		goto EndOf_Process;
	}

	//���ҽ�ɫ���е�δ������
	//	�����ѽ�����ɣ�δ�ﵽ���޵��ճ�����
	sprintf( szSql, "select   TaskID, TaskName, TaskType, 1 as TaskStatus, CanAcceptScript \
									from Task \
									where MinLevel <= %d \
										and TaskID not in (  select TaskID \
																						from RoleTask  \
																						where RoleID = %d \
																				) \
									union \
									select   t.TaskID, t.TaskName, t.TaskType, 1 as TaskStatus, t.CanAcceptScript  \
									from Task t, RoleTask rt \
									where t.TaskType = 3 \
										and t.MinLevel <= %d \
										and t.MaxLevel >= %d \
										and rt.RoleID = %d \
										and t.TaskID = rt.TaskID \
										and rt.FinishStatus = 2 \
										and t.MaxAcceptNum > rt.DeliverNum \
										", pRole->Level(), roleID,
										pRole->Level(), pRole->Level(), roleID	);
	iRet = dbo.QuerySQL(szSql);
 	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}

	while(dbo.HasRowData())
	{
		//�ű�ִ�У��ж��Ƿ�����ɽ�����
		iRet = ExecCanAcceptScript( roleID, dbo.GetStringField(4));
		if( 0 == iRet )
		{
			//��������
			taskInfo.taskID= dbo.GetIntField(0);
			taskInfo.taskName = dbo.GetStringField(1);
			taskInfo.taskType = dbo.GetIntField(2);
			taskInfo.taskStatus= dbo.GetIntField(3);

			lti.push_back( taskInfo );
		}

		//��¼����һ����¼
		dbo.NextRow();
	}


	//���ҽ�ɫ���ѽ�����
 	sprintf( szSql, "select   TaskID, TaskName, TaskType, 2 as TaskStatus \
								from Task \
								where  TaskID in (  select TaskID \
																					from RoleTask \
																					where RoleID = %d \
																						and FinishStatus = 0 \
																			) \
								union select   TaskID, TaskName,TaskType, 3 as TaskStatus \
								from Task \
								where TaskID in (  select TaskID \
																					from RoleTask \
																					where RoleID = %d \
																						and FinishStatus = 1 \
																			) \
									", roleID, roleID  );

	iRet = dbo.QuerySQL(szSql);
 	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}

	while(dbo.HasRowData())
	{
		//��������
		taskInfo.taskID= dbo.GetIntField(0);
		taskInfo.taskName = dbo.GetStringField(1);
		taskInfo.taskType = dbo.GetIntField(2);
		taskInfo.taskStatus= dbo.GetIntField(3);

		lti.push_back( taskInfo );

		//��¼����һ����¼
		dbo.NextRow();
	}

	//��������ϸ
	for( it = lti.begin(); it != lti.end(); it++ )
	{
		sprintf( szSql, " select GoalType, GoalID, IsFinish, GoalNum, FinishNum \
												from RoleTaskDetail \
												where RoleID = %d \
												and TaskID = %d ",
												roleID, it->taskID );
		iRet = dbo.QuerySQL(szSql);
	 	if( iRet < 0 )
		{
			RetCode = ERR_SYSTEM_DBERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			goto EndOf_Process;
	 	}

		while(dbo.HasRowData())
		{
			//��������
			taskDetail.goalType = dbo.GetIntField(0);
			taskDetail.goalID = dbo.GetIntField(1);
			taskDetail.isFinish = dbo.GetIntField(2);
			taskDetail.goalNum = dbo.GetIntField(3);
			taskDetail.finishNum = dbo.GetIntField(4);

			it->ltd.push_back( taskDetail );

			//��¼����һ����¼
			dbo.NextRow();
		}

	}


EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_C_S_RESP;
	p.PackHeader();


	s<<RetCode;
	if( 0 == RetCode )
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
		s<<lti;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF( "ack pkg=======, \n" );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );

	return;

}




void TaskSvc::ProcessAcceptTask(Session& session,Packet& packet)
{
 	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	UInt32 roleID = packet.RoleID;
	UInt32 taskID;
	UInt32	taskType = 0;
	UInt32	minLevel = 0;
	string canAcceptScript;
	string toDoScript;
	string afterDeliverScript;
	int dateDiff = 0;
	Byte status = 0;
	UInt32 maxAcceptNum = 0;
	UInt32 maxLevel = 0;
	int iHasRecord = 0;					//�ý�ɫ ��RoleTask �Ƿ��и������¼

	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);

	//���л���
	Serializer s(packet.GetBuffer());
	s>>taskID;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());


	//��ȡ����������Ϣ
	sprintf( szSql, "select TaskType, MinLevel, CanAcceptScript, TodoScript, AfterDeliverScript, MaxAcceptNum, MaxLevel \
									from Task \
									where TaskID = %d ", taskID );
	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		RetCode = ERR_SYSTEM_DBNORECORD;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
		goto EndOf_Process;
	}
 	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}

	//���ݻ�ȡ
 	taskType = dbo.GetIntField(0);
 	minLevel = dbo.GetIntField(1);
 	canAcceptScript = dbo.GetStringField(2);
 	toDoScript = dbo.GetStringField(3);
 	afterDeliverScript = dbo.GetStringField(4);

 	maxAcceptNum = dbo.GetIntField(5);
 	maxLevel = dbo.GetIntField(6);

	//��ȡ��ɫ��Ϣ
	if( 0 == pRole->ID() )
	{
		RetCode = ERR_APP_ROLENOTEXISTS;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole error,role not exists, roleID[%d]", roleID  );
		goto EndOf_Process;
	}

	//����ȼ�У��
	if( pRole->Level() < minLevel || pRole->Level() > maxLevel )
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"level error!roleLevel[%d],minLevel[%d], maxLevle[%d]", pRole->Level(), minLevel, maxLevel );
		goto EndOf_Process;
	}

	//����״̬��ȡ
	iRet = GetRoleTaskStatus( roleID, taskID, status);
	if(iRet)
	{
		RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"GetRoleTaskStatus error !!!!roleID[%d],taskID[%d]", roleID, taskID );
		goto EndOf_Process;
	}

	//����״̬У��,״̬����Ϊ δ��
	if( 1 != status )
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"taskStatus error!! taskStatus[%d] must be 1 !roleID[%d],taskID[%d]", status, roleID, taskID );
		goto EndOf_Process;
	}

	//����ɽ�����,���������
	iRet = ExecCanAcceptScript( roleID, canAcceptScript );
	if( 0 == iRet )
	{
		//���ý�ɫ�������¼�Ƿ��Ѿ�����
		sprintf( szSql, "select   TaskID \
										from RoleTask \
										where RoleID = %d \
											and TaskID = %d \
											", roleID, taskID );
		iRet = dbo.QuerySQL(szSql);
	 	if( iRet < 0 )
		{
			RetCode = ERR_SYSTEM_DBERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			goto EndOf_Process;
	 	}
	 	if( 1 == iRet )
	 		iHasRecord = 0;
	 	else
	 		iHasRecord = 1;


		//����ý�ɫ�� RoleTaskDetail ���е�����
		//	��ֹ�쳣���ݸ��ź������е�������ز���
		sprintf( szSql, " delete from RoleTaskDetail \
											where RoleID = %d \
												and not exists ( select TaskID \
																					from RoleTask \
																					where RoleID = %d \
																				)",
										roleID, taskID );
		iRet = dbo.ExceSQL(szSql);
		if( iRet < 0 )
		{
			RetCode = ERR_SYSTEM_DBERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			goto EndOf_Process;
	 	}

		//����to-do�ű���ִ�У������� RoleTaskDetail ���Ӽ�¼
		iRet = ExecToDoScript( roleID, taskID, toDoScript );
		if( iRet )
		{
			RetCode = ERR_APP_OP;
			LOG(LOG_ERROR,__FILE__,__LINE__,"ExecToDoScript error, roleID[%d], taskID[%d], toDoScript[%s]", roleID, taskID, toDoScript.c_str() );
			goto EndOf_Process;
		}

		//���ճ�����, RoleTask ����м�¼��ֱ��ɾ��
		if( TASKTYPE_DAILY != taskType && iHasRecord )
		{
			//���� RoleTask ��
			//	��ֹ�쳣���ݸ���
		 	sprintf( szSql, " delete from RoleTask \
												where RoleID = %d \
													and TaskID = %d ",
											roleID, taskID );
			iRet = dbo.ExceSQL(szSql);
			if( iRet < 0 )
			{
				RetCode = ERR_SYSTEM_DBERROR;
				LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
				goto EndOf_Process;
		 	}
		}

		//������������ RoleTask �� ���ӻ��߸��¼�¼
		if( iHasRecord )
		{
			//�м�¼�� �ճ�����,ֱ�Ӹ���
			sprintf( szSql, " update RoleTask \
												set FinishStatus = 0 \
												where RoleID = %d \
													and TaskID = %d ",
													roleID, taskID );
			iRet = dbo.ExceSQL(szSql);
			if( iRet < 0 )
			{
				RetCode = ERR_SYSTEM_DBERROR;
				LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
				goto EndOf_Process;
		 	}
		}
		else
		{
 			//���� RoleTask ��
			sprintf( szSql, " insert into RoleTask( RoleId, TaskID, FinishStatus, DeliverNum) \
												values( %d, %d, 0, 0 ) ",
													roleID, taskID );
			iRet = dbo.ExceSQL(szSql);
			if( iRet < 0 )
			{
				RetCode = ERR_SYSTEM_DBERROR;
				LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
				goto EndOf_Process;
		 	}
	 	}
	}

	//���ݱ�����Ʒ,��������ϸ��״̬
	CheckAllItemTypeTaskStatus(roleID);

	//�������������������״̬�ı�
	UpdateTaskStatus( roleID, taskID );

	//���µ�����״̬��ȡ,���ڷ���
	iRet = GetRoleTaskStatus( roleID, taskID, status);
	if(iRet)
	{
		RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"GetRoleTaskStatus error !!!!roleID[%d],taskID[%d]", roleID, taskID );
		goto EndOf_Process;
	}


EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_C_S_RESP;
	p.PackHeader();


	s<<RetCode;
	if( 0 == RetCode )
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
		s<<taskID<<status;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF( "ack pkg=======, \n" );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );

	//---------------------------------S_C ֪ͨ-----------------------------------------
	//�ɹ��������͹㲥
	if( 0 == RetCode )
	{
		//����״̬
		NotifyTaskStatus( roleID, taskID);

		//npcͷ��״̬
		_mainSvc->GetCoreData()->NotifyNpcStatus(roleID);
	}


	return;

}





void TaskSvc::ProcessDeliverTask(Session& session,Packet& packet)
{
 	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	UInt32 roleID = packet.RoleID;
	UInt32 taskID =0;
	UInt32 npcID = 0;
	Byte	taskType = 0;
	UInt32 exitNpcID = 0;
	string	canAcceptScript;
	string	toDoScript;
	string	afterDeliverScript;
	Byte finishStatus = 0;
	Byte status = 0;

	//���л���
	Serializer s(packet.GetBuffer());
	s>>taskID>>npcID;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//��ȡ����������Ϣ
	sprintf( szSql, "select TaskType, ExitNpcID, CanAcceptScript, TodoScript, AfterDeliverScript \
									from Task \
									where TaskID = %d ", taskID );
	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		RetCode = ERR_SYSTEM_DBNORECORD;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
		goto EndOf_Process;
	}
 	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}

	//���ݻ�ȡ
 	taskType = dbo.GetIntField(0);
 	exitNpcID = dbo.GetIntField(1);
 	canAcceptScript = dbo.GetStringField(2);
 	toDoScript = dbo.GetStringField(3);
 	afterDeliverScript = dbo.GetStringField(4);

 	//npcУ��
 	if( npcID != exitNpcID )
 	{
 		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"not the right NPC, req npcID[%d], taskID[%d],  exitNpcID[%d]", npcID, taskID, exitNpcID );
		goto EndOf_Process;
 	}

	//����״̬��ȡ
	iRet = GetRoleTaskStatus( roleID, taskID, status);
	if(iRet)
	{
		RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"GetRoleTaskStatus error !!!!roleID[%d],taskID[%d]", roleID, taskID );
		goto EndOf_Process;
	}

	//����״̬У��,״̬����Ϊ�����
	if( 3 != status )
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"error:task is not Finished!!!!roleID[%d],taskID[%d]", roleID, taskID );
		goto EndOf_Process;
	}

 	//ɾ�� RoleTaskDetail��
	sprintf( szSql, " delete from RoleTaskDetail \
										where RoleID = %d \
 											and TaskID = %d ",
										roleID, taskID );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}

 	//�޸�����״̬
 	sprintf( szSql, " update RoleTask \
 										set FinishStatus = 2, \
 											DeliverNum = DeliverNum + 1 \
										where RoleID = %d \
 											and TaskID = %d ",
										roleID, taskID );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}

	//���񽻸��ű�ִ��
	iRet = ExecAfterDeliverScript( roleID, afterDeliverScript );
	if( iRet )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExecAfterDeliverScript error, roleID[%d], taskID[%d], afterDeliverScript[%s]", roleID, taskID, afterDeliverScript.c_str() );
	}


EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_C_S_RESP;
	p.PackHeader();


	s<<RetCode;
	if( 0 == RetCode )
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
		s<<taskID;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF( "ack pkg=======, \n" );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );

	//---------------------------------S_C ֪ͨ-----------------------------------------
	//�ɹ��������͹㲥
	if( 0 == RetCode )
	{
		_mainSvc->GetCoreData()->NotifyNpcStatus(roleID);	//Npcͷ��״̬����

		NotifyTaskCanAccept(roleID);		//��ɫ�ɽ��������֪ͨ
	}

	return;

}


void TaskSvc::ProcessAbandonTask(Session& session,Packet& packet)
{
 	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	UInt32 roleID = packet.RoleID;
	UInt32 taskID =0;
	Byte	taskType = 0;
	Byte	finishStatus = 0;
	int dateDiff = 0;
	Byte status = 0;

	//���л���
	Serializer s(packet.GetBuffer());
	s>>taskID;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//����״̬��ȡ
	iRet = GetRoleTaskStatus( roleID, taskID, status);
	if(iRet)
	{
		RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"GetRoleTaskStatus error !!!!roleID[%d],taskID[%d]", roleID, taskID );
		goto EndOf_Process;
	}

	//����״̬У��, ״̬����Ϊ 2δ��� ����  3�����
	if( status!= 2 && status != 3 )
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"role taskStatus error!roleID[%d],taskID[%d],status[%d]", roleID, taskID, status );
		goto EndOf_Process;
	}

	//��ȡ����������Ϣ
	sprintf( szSql, "select TaskType \
									from Task \
									where TaskID = %d ", taskID );
	iRet = dbo.QuerySQL(szSql);
 	if( iRet )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}

	//���ݻ�ȡ
 	taskType = dbo.GetIntField(0);


 	//ɾ�� RoleTaskDetail��
	sprintf( szSql, " delete from RoleTaskDetail \
										where RoleID = %d \
 											and TaskID = %d ",
										roleID, taskID );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}

	//������������,ɾ�����߸��¼�¼
	if( TASKTYPE_DAILY != taskType )
	{
		//ɾ�� RoleTask���¼
		sprintf( szSql, " delete from RoleTask \
											where RoleID = %d \
													and TaskID = %d ",
											roleID, taskID );
		iRet = dbo.ExceSQL(szSql);
		if( iRet < 0 )
		{
			RetCode = ERR_SYSTEM_DBERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			goto EndOf_Process;
		}
	}
	else
	{
		//�ճ�����,����� RoleTask�� ������״̬Ϊ �ѽ���
		sprintf( szSql, " update RoleTask \
											set FinishStatus = 2 \
											where RoleID = %d \
													and TaskID = %d ",
											roleID, taskID );
		iRet = dbo.ExceSQL(szSql);
		if( iRet < 0 )
		{
			RetCode = ERR_SYSTEM_DBERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			goto EndOf_Process;
		}
	}


EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_C_S_RESP;
	p.PackHeader();


	s<<RetCode;
	if( 0 == RetCode )
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
//		s<<ltib;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF( "ack pkg=======, \n" );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );

	//---------------------------------S_C ֪ͨ-----------------------------------------
	//�ɹ��������͹㲥
	if( 0 == RetCode )
	{
		_mainSvc->GetCoreData()->NotifyNpcStatus(roleID);
	}

	return;

}

int TaskSvc::AddRoleExp( UInt32 roleID, UInt32 exp )
{
	return _mainSvc->GetCoreData()->RoleExpAdd( roleID,exp);
}

int TaskSvc::AddRoleMoney( UInt32 roleID,UInt32 money )
{
	return _mainSvc->GetBagSvc()->Getmoney(roleID,money);
}


int TaskSvc::AddRoleBindMoney( UInt32 roleID,UInt32 money )
{
	return _mainSvc->GetBagSvc()->GetBindMoney(roleID,money);
}


int TaskSvc::AddRoleItem( UInt32 roleID ,UInt32 ItemID,UInt32 num )
{
	List<ItemList> it;
	ItemList p;
	p.ItemID=ItemID;
	p.num=num;
	it.push_back(p);
	int iRet=0;
	iRet= _mainSvc->GetBagSvc()->RoleGetItem(roleID,it);
	return iRet;
}

int TaskSvc::DeleteItem(UInt32 roleID,UInt32 ItemID,UInt32 num)
{
		List<ItemList> it;
		ItemList p;
		p.ItemID=ItemID;
		p.num=num;
		it.push_back(p);
		int iRet=0;
		iRet= _mainSvc->GetBagSvc()->RoleDropItem(roleID, it);
		return iRet;
}


void TaskSvc::callNotifybag(UInt32 roleID,List < ItemCell > & lic,Byte flag)
{
	_mainSvc->GetBagSvc()->NotifyBag(roleID,lic);
}



//@brief	��ȡ��ɫ��ǰ���пɽ�����
//@return ��
void TaskSvc::GetRoleCanAcceptTask(UInt32 roleID, List<ArchvTaskInfo>& lti)
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	ArchvTaskInfo taskInfo;

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//��ȡ��ɫ��Ϣ
	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr( roleID);
	if( 0 == pRole->ID() )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole error,role not exists, roleID[%d]", roleID  );
		return;
	}

	//���ҽ�ɫ���е�δ������
	//	�����ѽ�����ɣ�δ�ﵽ���޵��ճ�����
	sprintf( szSql, "select   TaskID, TaskName, TaskType, 1 as TaskStatus, CanAcceptScript \
									from Task \
									where MinLevel <= %d \
										and TaskID not in (  select TaskID \
																						from RoleTask  \
																						where RoleID = %d \
																				) \
									union \
									select   t.TaskID, t.TaskName, t.TaskType, 1 as TaskStatus, t.CanAcceptScript  \
									from Task t, RoleTask rt \
									where t.TaskType = 3 \
										and t.MinLevel <= %d \
										and t.MaxLevel >= %d \
										and rt.RoleID = %d \
										and t.TaskID = rt.TaskID \
										and rt.FinishStatus = 2 \
										and t.MaxAcceptNum > rt.DeliverNum \
										", pRole->Level(), roleID,
										pRole->Level(), pRole->Level(), roleID	);
	iRet = dbo.QuerySQL(szSql);
 	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return;
 	}

	while(dbo.HasRowData())
	{
		//�ű�ִ�У��ж��Ƿ�����ɽ�����
		iRet = ExecCanAcceptScript( roleID, dbo.GetStringField(4));
		if( 0 == iRet )
		{
			//��������
			taskInfo.taskID= dbo.GetIntField(0);
			taskInfo.taskName = dbo.GetStringField(1);
			taskInfo.taskType = dbo.GetIntField(2);
			taskInfo.taskStatus= dbo.GetIntField(3);

			lti.push_back( taskInfo );
		}

		//��¼����һ����¼
		dbo.NextRow();
	}

	//��������ϸ
	List<ArchvTaskInfo>::iterator it;
	ArchvTaskDetail taskDetail;
	for( it = lti.begin(); it != lti.end(); it++ )
	{
		sprintf( szSql, " select GoalType, GoalID, IsFinish, GoalNum, FinishNum \
												from RoleTaskDetail \
												where RoleID = %d \
												and TaskID = %d ",
												roleID, it->taskID );
		iRet = dbo.QuerySQL(szSql);
	 	if( iRet < 0 )
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			return;
	 	}

		while(dbo.HasRowData())
		{
			//��������
			taskDetail.goalType = dbo.GetIntField(0);
			taskDetail.goalID = dbo.GetIntField(1);
			taskDetail.isFinish = dbo.GetIntField(2);
			taskDetail.goalNum = dbo.GetIntField(3);
			taskDetail.finishNum = dbo.GetIntField(4);

			it->ltd.push_back( taskDetail );

			//��¼����һ����¼
			dbo.NextRow();
		}
	}

	return;
}

//@brief : ��ȡ��ɫ�ѽ�δ��ɵ�������Ϣ
//@param: roleTask, ��ɫδ��ɵ�������Ϣ
//return :  ����0 �ɹ����� 0 ʧ��
int TaskSvc::GetRoleUnFinishedTask(list<UInt32>licRoleID,List<ArchvUnfinishedTask>&licRoleTask)
{
	char szSql[1024];
	int iRet = 0;
	UInt32 taskID = 0;
    ArchvUnfinishedTask roleTask;
	list<UInt32>::iterator itor;
	Connection con;
	DBOperate dbo;

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	for(itor = licRoleID.begin(); itor != licRoleID.end(); itor++)
	{
		sprintf(szSql,"select TaskID from RoleTaskDetail \
		           where RoleID =%d and GoalType = 2 and IsFinish = 0;",*itor);
		iRet = dbo.QuerySQL(szSql);
		if(iRet == 0)
		{
			roleTask.roleID = *itor;
			while(dbo.HasRowData())
			{
				taskID = dbo.GetIntField(0);
				roleTask.licTaskID.push_back(taskID);
				dbo.NextRow();
			}

			licRoleTask.push_back(roleTask);
		}
		else
		{
			continue;
		}
	}

	if(licRoleTask.size() != 0)
	{
		return 0;
	}

	return -1;

}



//@brief	[MsgType:0701]������ϸ��״̬֪ͨ������������״̬Ϊδ��ɡ������ʱ��
//	����
void TaskSvc::NotifyTaskStatus( UInt32 roleID, UInt32 taskID )
{
	List<UInt32>lrid;

	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);

	Byte	taskStatus = 0;
	List<ArchvTaskDetail> ltd;
	int iRet = 0;

	//���ͷ
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 701;
	p.UniqID = 123;

	p.PackHeader();


	//��ȡ��������״̬
	iRet = GetRoleTaskStatus( roleID, taskID, taskStatus );
	if(iRet)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"GetRoleTaskStatus! roleID[%d], taskID[%d] ", roleID, taskID );
		return;
	}

	//��ȡ����ϸ������
	iRet = GetRoleTaskDetail( roleID, taskID, ltd );
	if(iRet)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"GetRoleTaskDetail! roleID[%d], taskID[%d] ", roleID, taskID );
		return;
	}

	//д����
	s<<taskID<<taskStatus<<ltd;
	p.UpdatePacketLength();


	lrid.clear();
	lrid.push_back(roleID);
	if( _mainSvc->Service()->Broadcast( lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
	}

DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size()[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );


}


//[MsgType:0702]��ɫ�ɽ��������֪ͨ��������ÿ�ν�������ʱ����
void TaskSvc::NotifyTaskCanAccept( UInt32 roleID )
{
	List<UInt32>lrid;

	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);

	List<ArchvTaskInfo> lti;

	//���ͷ
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 702;
	p.UniqID = 123;

	p.PackHeader();


	//��ȡ��������״̬
	GetRoleCanAcceptTask( roleID, lti);

	//д����
	s<<lti;
	p.UpdatePacketLength();


	lrid.clear();
	lrid.push_back(roleID);
	if( _mainSvc->Service()->Broadcast( lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
	}

DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size()[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );


}

