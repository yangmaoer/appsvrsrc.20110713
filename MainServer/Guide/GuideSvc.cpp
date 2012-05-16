#include "GuideSvc.h"
#include "MainSvc.h"
#include "DBOperate.h"
#include "CoreData.h"
#include "Role.h"

GuideSvc::GuideSvc(void* service, ConnectionPool * cp)
{
	_mainSvc = (MainSvc*)(service);
	_cp = cp;

}

GuideSvc::~GuideSvc()
{

}
//���ݰ���Ϣ
void GuideSvc::OnProcessPacket(Session& session,Packet& packet)
{
DEBUG_PRINTF1( "C_S req pkg-------MsgType[%d] \n", packet.MsgType );
	DEBUG_SHOWHEX( packet.GetBuffer()->GetReadPtr()-PACKET_HEADER_LENGTH, packet.GetBuffer()->GetDataSize()+PACKET_HEADER_LENGTH, 0, __FILE__, __LINE__ );

	switch(packet.MsgType)
	{
		case 1801: //[MsgType:1801]�������������Ľ���
			ProcessSetGuideStep(session,packet);
			break;

		case 1802: //[MsgType:1802]������������
			ProcessEndGuide(session,packet);
			break;
			
		default:
			ClientErrorAck(session,packet,ERR_SYSTEM_PARAM);
			LOG(LOG_ERROR,__FILE__,__LINE__,"MsgType[%d] not found",packet.MsgType);
		}
}




//�ͻ��˴���Ӧ��
//@param  session ���Ӷ���
//@param	packet �����
//@param	RetCode ����errorCode ֵ
void GuideSvc::ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode)
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


void GuideSvc::ProcessSetGuideStep(Session& session,Packet& packet)
{

	UInt32	RetCode = 0;
	DataBuffer	serbuffer(2048);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	UInt32 roleID = packet.RoleID;
	Byte stepID = 0;
	

	//���л���
	Serializer s(packet.GetBuffer());
	s>>stepID;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//��ȡ����������¼
	sprintf( szSql, "select StepID \
									 from RoleGuide \
									 where RoleID = %d " , roleID);
	iRet = dbo.QuerySQL(szSql);
	if( iRet )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}

 	//���¼�¼
 	sprintf( szSql, "update RoleGuide \
 									 set StepID = %d \
									 where RoleID = %d ", stepID, roleID);
	iRet = dbo.ExceSQL(szSql);
	if( iRet )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExecSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
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
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ����
//		s<<lic;
	}	

	p.UpdatePacketLength();
	
	//����Ӧ������
	//�ý��ײ�����Ӧ��
	/*
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}
	DEBUG_PRINTF( "ack pkg=======, \n" );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );
	*/

	return;
	
}



void GuideSvc::ProcessEndGuide(Session& session,Packet& packet)
{

	UInt32	RetCode = 0;
	DataBuffer	serbuffer(2048);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	UInt32 roleID = packet.RoleID;
	Byte stepID = 0;
	

	//���л���
	Serializer s(packet.GetBuffer());
//	s>>stepID;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

 	//ɾ����¼
 	sprintf( szSql, "delete from RoleGuide \
									 where RoleID = %d ", roleID);
	iRet=dbo.ExceSQL(szSql);
	if( iRet )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExecSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
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
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ����
//		s<<lic;
	}	

	p.UpdatePacketLength();
	
	//����Ӧ������
	//�ý��ײ�����Ӧ��
	/*
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}
	DEBUG_PRINTF( "ack pkg=======, \n" );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );
	*/

	return;
	
}


void GuideSvc::NotifyGuideStep(UInt32 roleID)
{
	DataBuffer	serbuffer(1024);
	int iRet = 0;
	Connection con;
	DBOperate dbo;
	List<UInt32> lrid;

	Byte stepID = 0;
	char szSql[1024];
	
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1801;
	p.UniqID = 250;
	p.PackHeader();

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//��ȡ����������¼
	sprintf( szSql, "select StepID \
									 from RoleGuide \
									 where RoleID = %d " , roleID);
	iRet=dbo.QuerySQL(szSql);
	if(iRet<0)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return;
 	}
 	if(1==iRet)
 	{//û������������¼����������Ϣ
 		return;
 	}

 	stepID = dbo.GetIntField(0);
	
	//�����
	s<<stepID;
	p.UpdatePacketLength();
	lrid.push_back(roleID);

	if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!");
	}
	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}

