#include "MapSvc.h"
#include "MainSvc.h"
#include "ArchvTask.h"
#include "DBOperate.h"
#include "ArchvTask.h"
#include "CoreData.h"
#include "ArchvRole.h"
#include "SSClientManager.h"
#include "ArchvMap.h"
#include "Role.h"
#include "./Guide/GuideSvc.h"
#include "./OffLineUpdate/OffLineUpdateSvc.h"

MapSvc::MapSvc(void* service, ConnectionPool * cp)
{
	_mainSvc = (MainSvc*)(service);
	_cp = cp;
}

MapSvc::~MapSvc()
{
}

void MapSvc::OnProcessPacket(Session& session,Packet& packet)
{

DEBUG_PRINTF1( "C_S req pkg-------MsgType[%d] \n", packet.MsgType );
	DEBUG_SHOWHEX( packet.GetBuffer()->GetReadPtr()-PACKET_HEADER_LENGTH, packet.GetBuffer()->GetDataSize()+PACKET_HEADER_LENGTH, 0, __FILE__, __LINE__ );

	switch(packet.MsgType)
	{
		case 201:
			ProcessEnterMap(session,packet);
			break;

		case 202:
			ProcessMapMove(session,packet);
			break;

 		case 203:
			ProcessTeleportByRune(session,packet);
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
void MapSvc::ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode)
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



//����X������
//	��������-->pk����
//@param	X	����X����
//@param  originX ����pkԭ�� X����
//@return  X����
int MapSvc::AdjustCoordinateX( UInt32 X, UInt32 originX )
{
	int iTmp = X - originX;

	if( iTmp <= 0 )
		return 0;
	else if( iTmp > PKSCREEN_XLENGTH )
		return PKSCREEN_XLENGTH;
	else
		return iTmp;
}



//����Y������
//	��������-->pk����
//@param	Y	����Y����
//@param  originX ����pkԭ�� Y����
//@return  Y����
int MapSvc::AdjustCoordinateY( UInt32 Y, UInt32 originY )
{
	int iTmp = Y - originY;

	if( iTmp <= 0 )
		return 0;
	else if( iTmp > PKSCREEN_YLENGTH )
		return PKSCREEN_YLENGTH;
	else
		return iTmp;
}


//@brief	[MsgType:0201]���볡��
void MapSvc::ProcessEnterMap(Session& session,Packet& packet)
{
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	UInt32 roleID = packet.RoleID;
	UInt32 mapID = 0;
	UInt16 x;
	UInt16 y;
	UInt32 Time = 0;

	//���л���
	Serializer s(packet.GetBuffer());
	s>>mapID>>x>>y;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

//DEBUG_PRINTF4( " ProcessEnterMap--roleID[%d],MapID[%d],.X[%d],.Y[%d]  \n", roleID, mapID, x, y );
	//ҵ����
	iRet = _mainSvc->GetCoreData()->ProcessRoleEnterMap( roleID, mapID, x, y);
	if( iRet == -1)
	{
		RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"GetCoreData()->ProcessRoleEnterMap error");
		goto EndOf_Process;
	}
	if(iRet == 1)
	{
		RetCode = ERR_APP_ALREADYINMAP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"GetCoreData()->ProcessRoleEnterMap error");
		goto EndOf_Process;

	}

EndOf_Process:
	//Ӧ���ʱ���ֶ�
	Time = time(NULL);

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
		s<<Time;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "session->Send() error,errno[%d],strerror[%s]", errno, strerror(errno) );
	}

//DEBUG_PRINTF1( "C_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
//	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );
	if(RetCode==0)
	{
		RolePtr pRole=_mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
		//[MsgType:0202]�����������볡������������������ɫ���б�
		_mainSvc->GetCoreData()->NotifyAllRoleInfo(roleID);

		//s-c205�����ͼ
		//NotifyEnterMap(roleID,mapID,x,y);
		if(pRole->TeamFlag()==1)
		{
			NotifyTeamLeader(roleID,pRole->LeaderRoleID());
		}

 LOG(LOG_DEBUG,__FILE__,__LINE__,"ProcessEnterMap OK!!roleID[%d],mapID[%d], EnterMapNum[%d]", pRole->ID(), mapID, pRole->EnterMapNum());

		//��ɫ��½����ͼ��δ��ɵĹһ�����
		_mainSvc->GetOffLineUpdateSvc()->OnRoleLoginNotify(roleID);

		if(pRole->EnterMapNum() == 1)
		{
			//��ɫ��¼�ķ���ʹ������
		    List<RoleRune> lic;
		    pRole->PopulateRoleRuneList(lic);

			if(lic.size()>0){
				NotifyActRuneListOnLogin(roleID, lic);
			}
		}
	}

	return;
}

//S-C��һ���㲥
void MapSvc::NotifyActRuneListOnLogin(UInt32 roleID,List<RoleRune>& lic)
{
	DataBuffer	serbuffer(1024);
	Int16 iRet = 0;
	List<UInt32> lrid;
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 2202;
	p.UniqID = 259;
	p.PackHeader();

	s<<lic;
	p.UpdatePacketLength();
	lrid.push_back(roleID);

	if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!");
	}
	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}




//@brief	[MsgType:0202]�ƶ�
void MapSvc::ProcessMapMove(Session& session,Packet& packet)
{
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	int iRet = 0;

	UInt32 RoleID = packet.RoleID;
 	ArchvRoute art;

	//���л���
	Serializer s(packet.GetBuffer());
	s>>art;
	if( s.GetErrorCode()!= 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		return;
	}

DEBUG_PRINTF1( " ProcessMapMove  roleID[%d]\n", RoleID );
	//ҵ����
	iRet = _mainSvc->GetCoreData()->ProcessRoleMove( RoleID, art);
	if(iRet)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessRoleMove error" );
		return;
	}

	return;
}

//@brief	[MsgType:0203] (ʹ�÷��з���)˲��
void MapSvc::ProcessTeleportByRune(Session& session,Packet& packet)
{
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	UInt32 roleID = packet.RoleID;
	UInt32 mapID = 0;
	UInt16 x;
	UInt16 y;
 	ArchvRoute art;
 	ArchvPosition pos;
 	int isToCurrMap = 0;		//�Ƿ񱾵�ͼ��˲��	0��   1 ��

	//���л���
	Serializer s(packet.GetBuffer());
	s>>mapID>>x>>y;
	if( s.GetErrorCode()!= 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		return;
	}

DEBUG_PRINTF1( " ProcessTeleportByRune  roleID[%d]\n", roleID );
	//��ȡ��ɫ��Ϣ
	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
	if(pRole->ID()==0)
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"data error!! role not found,roleID[%d]", roleID);
		goto EndOf_Process;
	}

	//���״̬,����˲��
	if(pRole->TeamFlag())
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"status error!! role in team, roleID[%d]", roleID);
		goto EndOf_Process;
	}

	//�Ƿ��ڱ���ͼ˲��
	if( pRole->MapID() == mapID )
	{//����ͼ��˲��

		isToCurrMap = 1;

		//�ƶ�
		pos.X = x;
		pos.Y = y;
		art.listPos.push_back(pos);
		iRet = _mainSvc->GetCoreData()->ProcessRoleMove( roleID, art);
		if(iRet)
		{
			RetCode = ERR_APP_OP;
			LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessRoleMove error" );
			goto EndOf_Process;
		}

	}
	else
	{//���ͼ˲��
		pRole->MapID(mapID);
		isToCurrMap = 0;

		//ҵ����
		iRet = _mainSvc->GetCoreData()->ProcessRoleEnterMap( roleID, mapID, x, y);
		if( iRet == -1)
		{
			RetCode = ERR_APP_OP;
			LOG(LOG_ERROR,__FILE__,__LINE__,"GetCoreData()->ProcessRoleEnterMap error");
			goto EndOf_Process;
		}
		if(iRet == 1)
		{
			RetCode = ERR_APP_ALREADYINMAP;
			LOG(LOG_ERROR,__FILE__,__LINE__,"GetCoreData()->ProcessRoleEnterMap error");
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
//		s<<Time;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR, __FILE__, __LINE__, "session->Send() error,errno[%d],strerror[%s]", errno, strerror(errno) );
	}

DEBUG_PRINTF1( "C_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );


	if(RetCode==0)
	{
		//���ͼ˲��
		if(0==isToCurrMap)
		{
			//[MsgType:0202]�����������볡������������������ɫ���б�
			_mainSvc->GetCoreData()->NotifyAllRoleInfo(roleID);
		}
	}
	return;


}


void MapSvc::NotifyTeamLeader(UInt32 roleID,UInt32 leaderID)
{
	DataBuffer	serbuffer(1024);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	list<UInt32> itor;
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1406;
	p.UniqID = 218;
	itor.push_back(roleID);
	p.PackHeader();
	s<<leaderID;
	p.UpdatePacketLength();
	if(_mainSvc->Service()->Broadcast(itor, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!");
	}
	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, itor.size() );
}

void  MapSvc::NotifyEnterMap(UInt32 RoleID,UInt32 MapID,UInt16 X,UInt16 Y)
{
	DataBuffer	serbuffer(1024);
	Int16 iRet = 0;
	List<UInt32> lrid;
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 205;
	p.UniqID = 212;
	p.PackHeader();

	s<<MapID<<X<<Y;
	p.UpdatePacketLength();
	lrid.push_back(RoleID);

	if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!");
	}
	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}


