#include "NpcSvc.h"
#include "MainSvc.h"
#include "ArchvDialogItem.h"
#include "DBOperate.h"
#include "NpcLua.h"
#include "ArchvNpc.h"
#include "ArchvTask.h"
#include "CoreData.h"
#include "Role.h"
#include "../Task/TaskSvc.h"

using namespace NpcLua;

NpcSvc::NpcSvc(void* service, ConnectionPool * cp)
{
	_mainSvc = (MainSvc*)(service);
	_cp = cp;

}

NpcSvc::~NpcSvc()
{
}

void NpcSvc::OnProcessPacket(Session& session,Packet& packet)
{
	
DEBUG_PRINTF1( "C_S req pkg-------MsgType[%d] \n", packet.MsgType );
	DEBUG_SHOWHEX( packet.GetBuffer()->GetReadPtr()-PACKET_HEADER_LENGTH, packet.GetBuffer()->GetDataSize()+PACKET_HEADER_LENGTH, 0, __FILE__, __LINE__ );

	switch(packet.MsgType)
	{
		case 601: //Npc�Ի�
			ProcessNpcTalk(session,packet);
			break;

		case 602: //Npc�̵���Ʒ��ѯ
			ProcessGetNpcShopItem(session,packet);
			break;

		default:
			ClientErrorAck(session,packet,ERR_SYSTEM_PARAM);
			LOG(LOG_ERROR,__FILE__,__LINE__,"MsgType[%d] not found",packet.MsgType);
			break;
		}
}


//ִ��������������ű�
//@return 0 �ɹ�,����ɽ�   ��0 ִ�нű�ʧ�ܣ���������������
int NpcSvc::ExecCanAcceptScript( UInt32 roleID, const string& script )
{

	if( script.size() == 0 )
		return 0;

	//ִ�нű�
	lua_State* state = _luaState.GetState(RegisterNpcCAPI);

	SET_TASKSVC(state, _mainSvc->GetTaskSvc());
	SET_NPCSVC(state,this);
	SET_ROLEID(state, roleID );
	if(LuaState::Do(state, script))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"exec lua error, roleID[%d], script[%s], errorMsg[%s]",			roleID, script.c_str(), LuaState::GetErrorMsg(state));
		return -1;
	}

	return GetLuaRetCode(state);

}



//�ͻ��˴���Ӧ��
//@param  session ���Ӷ���
//@param	packet �����
//@param	RetCode ����errorCode ֵ
void NpcSvc::ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode)
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

//@brief	��ȡ��ɫ��npcͷ��״̬
//	ÿ����ɫ��ÿ��npc���ж������״̬(δ��,δ���,�����)
//	����ֻ����ʾһ��״̬,���ȼ�: �����-->δ���-->δ��
//@param	status	���ص�����״̬
//							 0 û������	1 �ɽ� 2 δ��� 3 �����
//@return 	0 �ɹ�   ��0 ʧ��
int NpcSvc::GetNpcStatus( UInt32 roleID, UInt32 level, UInt32 npcID, Byte& status )
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	Byte taskType = 0;
	status = 0;

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());


	//���ҽ�ɫ�ڸ�npc�����������
 	sprintf( szSql, "select 3 as TaskStatus \
								from Task \
								where ExitNpcID = %d \
									and TaskID in (  select TaskID \
																					from RoleTask \
																					where RoleID = %d \
																						and FinishStatus = 1 \
																			) ", npcID, roleID  );

	iRet = dbo.QuerySQL(szSql);
 	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}

	while(dbo.HasRowData())
	{
		//��ȡ����
		status = dbo.GetIntField(0);
		return 0;

		//��¼����һ����¼
		dbo.NextRow();
	}

	//���ҽ�ɫ�ڸ�npc��δ�������
 	sprintf( szSql, "select 2 as TaskStatus \
								from Task \
								where ExitNpcID = %d \
									and TaskID in (  select TaskID \
																					from RoleTask \
																					where RoleID = %d \
																						and FinishStatus = 0 \
																			) ", npcID, roleID  );

	iRet = dbo.QuerySQL(szSql);
 	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}

	while(dbo.HasRowData())
	{
		//��ȡ����
		status = dbo.GetIntField(0);
		return 0;

		//��¼����һ����¼
		dbo.NextRow();
	}
	

	//���ҽ�ɫ�ڸ�npc��δ������
	//	�����ѽ�����ɣ�δ�ﵽ���޵��ճ�����
	sprintf( szSql, "select 1 as TaskStatus, CanAcceptScript \
									from Task \
									where EntryNpcID = %d \
										and MinLevel <= %d \
										and TaskID not in (  select TaskID \
																						from RoleTask  \
																						where RoleID = %d \
																				) \
									union \
									select 1 as TaskStatus, t.CanAcceptScript \
									from Task t, RoleTask rt \
									where t.EntryNpcID = %d \
										and t.MinLevel <= %d \
										and t.MaxLevel >= %d \
										and t.TaskType = 3 \
										and rt.RoleID = %d \
										and t.TaskID = rt.TaskID \
										and rt.FinishStatus = 2 \
										and t.MaxAcceptNum > rt.DeliverNum \
										", npcID, level, roleID,
										 npcID, level, level, roleID );
	iRet = dbo.QuerySQL(szSql);
 	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}
	
	while(dbo.HasRowData())
	{
		//�ű�ִ�У��ж��Ƿ�����ɽ�����
		iRet = ExecCanAcceptScript( roleID, dbo.GetStringField(1));
		if( 0 == iRet )
		{
			//��ȡ����
			status = dbo.GetIntField(0);
			return 0;
		}

		//��¼����һ����¼
		dbo.NextRow();
	}

	return 0;
}


void NpcSvc::ProcessNpcTalk(Session& session,Packet& packet)
{

	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	UInt32 npcID;
	UInt32 roleID = packet.RoleID;
	//��ȡ��ɫ��Ϣ
	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);

	ArchvTaskInfoBrief taskInfoBrief;
	List<ArchvDialogItem> ldi;
	string canAcceptScript;

	//���л���
	Serializer s(packet.GetBuffer());
	s>>npcID;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	if( 0 == pRole->ID() )
	{
		RetCode = ERR_SYSTEM_DATANOTEXISTS;
		LOG(LOG_ERROR,__FILE__,__LINE__,"roleID[%d] not exists", roleID  );
		goto EndOf_Process;
	}


	//���ҽ�ɫ�ڸ�npc��δ������
	//	�����ѽ�����ɣ�δ�ﵽ���޵��ճ�����
	sprintf( szSql, "select   TaskID, TaskName, TaskType, 1 as TaskStatus, CanAcceptScript, ifnull(AdviceLevel ,0) \
									from Task \
									where EntryNpcID = %d \
										and MinLevel <= %d \
										and TaskID not in (  select TaskID \
																						from RoleTask  \
																						where RoleID = %d \
																				) \
									union \
									select   t.TaskID, t.TaskName, t.TaskType, 1 as TaskStatus, t.CanAcceptScript, ifnull(t.AdviceLevel, 0)  \
									from Task t, RoleTask rt \
									where t.EntryNpcID = %d \
										and t.MinLevel <= %d \
										and t.MaxLevel >= %d \
										and t.TaskType = 3 \
										and rt.RoleID = %d \
										and t.TaskID = rt.TaskID \
										and rt.FinishStatus = 2 \
										and t.MaxAcceptNum > rt.DeliverNum \
										", npcID, pRole->Level(), roleID,
										 npcID, pRole->Level(), pRole->Level(), roleID );
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

DEBUG_PRINTF1( " ------CanAcceptScript[%s] \n", dbo.GetStringField(4) );

		iRet = ExecCanAcceptScript( roleID, dbo.GetStringField(4));

DEBUG_PRINTF1( " ------iRet[%d] \n", iRet );

		if( 0 == iRet )
		{
			//��������
			taskInfoBrief.taskID= dbo.GetIntField(0);
			taskInfoBrief.taskName = dbo.GetStringField(1);
			taskInfoBrief.taskType = dbo.GetIntField(2);
			taskInfoBrief.taskStatus= dbo.GetIntField(3);
			taskInfoBrief.adviceLevel = dbo.GetIntField(5);

			ldi.push_back( ArchvDialogItem( 2, taskInfoBrief ) );
		}

		//��¼����һ����¼
		dbo.NextRow();
	}


	//���ҽ�ɫ�ڸ�npc���ѽ�����
 	sprintf( szSql, "select   TaskID, TaskName, TaskType, 2 as TaskStatus, ifnull(AdviceLevel, 0) \
								from Task \
								where ExitNpcID = %d \
									and TaskID in (  select TaskID \
																					from RoleTask \
																					where RoleID = %d \
																						and FinishStatus = 0 \
																			) \
								union select   TaskID, TaskName,TaskType, 3 as TaskStatus, ifnull(AdviceLevel,0) \
								from Task \
								where ExitNpcID = %d \
									and TaskID in (  select TaskID \
																					from RoleTask \
																					where RoleID = %d \
																						and FinishStatus = 1 \
																			) \
									", npcID, roleID, npcID, roleID  );

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
		taskInfoBrief.taskID= dbo.GetIntField(0);
		taskInfoBrief.taskName = dbo.GetStringField(1);
		taskInfoBrief.taskType = dbo.GetIntField(2);
		taskInfoBrief.taskStatus= dbo.GetIntField(3);
		taskInfoBrief.adviceLevel = dbo.GetIntField(4);
		
 
		ldi.push_back( ArchvDialogItem( 2, taskInfoBrief ) );
 
		//��¼����һ����¼
		dbo.NextRow();
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
		s<<npcID<<ldi;
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


 


void NpcSvc::ProcessGetNpcShopItem(Session& session,Packet& packet)
{

	List<UInt32> lItemID;
/*
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	UInt32 RoleID;
	string strDisplay;
	

	//���л���
	Serializer s(packet.GetBuffer());
	s>>RoleID>>strDisplay;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
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
//		s<<ari;
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
		//���͹㲥
		NotifyScreenDisplay( RoleID, strDisplay );
 	}

 	*/

	return;
	
}



