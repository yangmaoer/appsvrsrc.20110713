
#include "MainSvc.h"
#include "DBOperate.h"
#include "CoreData.h"
#include "Role.h"
#include "TeamSvc.h"	
#include "ArchvTeam.h"
#include "ArchvRole.h"

TeamSvc::TeamSvc(void* service, ConnectionPool * cp)
{
	_mainSvc = (MainSvc*)(service);
	_cp = cp;

}

TeamSvc::~TeamSvc()
{

}
//���ݰ���Ϣ
void TeamSvc::OnProcessPacket(Session& session,Packet& packet)
{
DEBUG_PRINTF1( "C_S req pkg-------MsgType[%d] \n", packet.MsgType );
	DEBUG_SHOWHEX( packet.GetBuffer()->GetReadPtr()-PACKET_HEADER_LENGTH, packet.GetBuffer()->GetDataSize()+PACKET_HEADER_LENGTH, 0, __FILE__, __LINE__ );

	switch(packet.MsgType)
	{
		case 1401: //����
			ProcessRequestjoin(session,packet);
		break;			
		case 1402:
			ProcessAnswerRequest(session,packet);
		break;
		case 1403:
			ProcessChangeCaptain(session,packet);
		break;
		case 1404:
			ProcessCallMember(session,packet);
		break;
		case 1405:
			ProcessCapt_Member(session,packet);
		break;
		case 1406:
			ProcessMember_OUt(session,packet);
		break;
		case 1407:
			ProcessBackOrleave(session,packet);
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
void TeamSvc::ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode)
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

void TeamSvc::ProcessRequestjoin(Session& session,Packet& packet)
{
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);

	UInt32 roleID = packet.RoleID;
	UInt32 ID=0;//�������Ľ�ɫID

	string Name;
	List<UInt32> it;
	UInt32 teamID=0;
	int iRet=0;
	Team team;
	UInt32 RoleID1=0;

		//���л���
	Serializer s(packet.GetBuffer());
	s>>ID;
	
	RolePtr pRole=_mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);

	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}
	if(ID==0||ID==roleID)
	{
			RetCode = 199;//���Լ��������
			LOG(LOG_ERROR,__FILE__,__LINE__,"there are some erro in the ID %d vs ResustID%d",ID,roleID );
			goto EndOf_Process;
	}
	//�������ж�	

	
	if(pRole->ID()!=0)
	{
		Name=pRole->Name();
	}
	else
	{
		 RetCode = ERR_SYSTEM_DBNORECORD;
		 LOG(LOG_ERROR,__FILE__,__LINE__,"there are some erro in the ID %d vs ResustID%d",ID,roleID );
		 goto EndOf_Process;
	}
	
	if(pRole->TeamFlag()==0)
	{
		//û�����

		//�Է���������û�����
			RoleID1=_mainSvc->GetCoreData()->ProcessGetRolePtr(ID)->LeaderRoleID();
			if(RoleID1==0)
			{
				//û���ҵ����������,��û�����
				//roleIDΪ�ӳ�
			}
			else
			{
				ID=RoleID1;
				//�ҵ�
				//��ӳ��������
			}
	}
	else if(pRole->TeamFlag()==2)
	{
		//�ӳ�
		//������ӵ�����
			if(_mainSvc->GetCoreData()->ProcessGetRolePtr(ID)->TeamID()!=0)
			{
				 RetCode = 200;//�Է������
				 LOG(LOG_ERROR,__FILE__,__LINE__,"there are some erro in the ID %d vs ResustID%d",ID,roleID );
				 goto EndOf_Process;
			}
	}
	else
	{	
		//����ӣ����ǲ��Ƕӳ�����Ȩ����
		//��ӳ������������
		RetCode = 201;//����ӣ����Ƕӳ�
		LOG(LOG_ERROR,__FILE__,__LINE__,"you are not the leader");
		 goto EndOf_Process;
	}
	//�ж�����һ���˶������
	DEBUG_PRINTF( "C_S ProcessProcessChat sucess!!!!!!!!\n" );	


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
	}	

	p.UpdatePacketLength();
	
	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}
	
	DEBUG_PRINTF( "ack pkg=======, \n" );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );
	if(0==RetCode)
	{
		it.push_back(ID);
	  NotifyReceive(roleID,Name,it);
	}
	return;

}


void TeamSvc::ProcessAnswerRequest(Session& session,Packet& packet)
{
	//���׵�Ӧ��,ͬ��˭������

		UInt32	RetCode = 0;
		DataBuffer	serbuffer(1024);

		UInt32 roleID = packet.RoleID;
		UInt32 ID=0;//Ӧ�����Ľ�ɫID
		Byte type=0;
		List<UInt32> it;
		Team team;
		Team team1;
		list<TeamRole>::iterator itor;
		list<TeamRole> li;
		TeamRole r;
		int iRet=0;
		UInt32 ItemID=0;
		Byte flag=0;
			//���л���
		Serializer s(packet.GetBuffer());
		s>>ID>>type;
		if( s.GetErrorCode()!= 0 )
		{
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
			goto EndOf_Process;
		}
		if(type==1)
		{
				//ͬ�����
				team1=_mainSvc->GetCoreData()->GetTeamsFromRoleID(roleID);
				team=_mainSvc->GetCoreData()->GetTeamsFromRoleID(ID);
				if(team1.GetLeaderRoleID()==roleID)
				{
						//�ӳ�ͬ���˶Է�������
						if(team.GetTeamID()==0)
						{
							iRet=_mainSvc->GetCoreData()->AddTeamRole(team1.GetTeamID(),ID);
							
								flag=1;
								if(iRet==-1)
								{
									//
									RetCode = 202;//���Ƕӳ��������Ҳ�����Ķ���
									LOG(LOG_ERROR,__FILE__,__LINE__,"find the team erro!" );
									goto EndOf_Process;
								}
						}
						else
						{//�Է��Ѿ��ڶ�����
									RetCode = 203;
									LOG(LOG_ERROR,__FILE__,__LINE__,"find the team erro!" );
									goto EndOf_Process;
						}
					
				}
				else if(team1.GetTeamID()==0)//û�����
				{
				
						if(team.GetTeamID()==0)//û�ж���
						{
							flag=2;
							_mainSvc->GetCoreData()->CreateTeam(ID,roleID);
						}
						else
						{
								if(team.GetLeaderRoleID()!=ID)//�Է����Ƕӳ�
								{
										RetCode = 204;
										LOG(LOG_ERROR,__FILE__,__LINE__,"The Role iS not the Leader" );
										goto EndOf_Process;
								}
								else
								{
									flag=3;
									iRet=_mainSvc->GetCoreData()->AddTeamRole(team.GetTeamID(),roleID);//�Է�һ�����Ƕӳ�
										if(iRet==-1)
										{
											//
											RetCode = ERR_SYSTEM_SERERROR;
											LOG(LOG_ERROR,__FILE__,__LINE__,"find the time erro!" );
											goto EndOf_Process;
										}
										
								}//end if  else


								
						}//end if else


						
				}//end 
				

			
		}//end type==1

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
		}	

		p.UpdatePacketLength();
		
		//����Ӧ������
		if( session.Send(&serbuffer) )
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
		}
		
		DEBUG_PRINTF( "ack pkg=======, \n" );
		DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );
		if(0==RetCode)
		{
			if(type==1)
			{
				//ͬ�����
				
					if(flag==1)
					{//�ӳ�ͬ��û����ӵļ������(�Լ��Ƕӳ�)
						li=team1.GetMemberRoleID();
						for(itor=li.begin();itor!=li.end();itor++)
						{
							it.push_back(itor->roleId);
						}
						r.roleId=ID;
						r.status=1;//ID�Լ���״̬������
						li.push_back(r);
						NotifyTeamChang(ID,1, it);//�����ж�Ա����S-C,���¼���ĳ�Ա����
						NotifyTeamInfo(ID,li);
					}
					if(flag==2)
					{
						r.roleId=ID;
						r.status=2;
						li.push_back(r);
						r.roleId=roleID;
						r.status=1;
						li.push_back(r);
						NotifyTeamInfo(ID,li);
						NotifyTeamInfo(roleID,li);
					//������û�����
						
					}
					if(flag==3)
					{//����û�ж��飬�ӳ�Ҫ��
						li=team.GetMemberRoleID();
						for(itor=li.begin();itor!=li.end();itor++)
						{
							it.push_back(itor->roleId);
						}
						r.roleId=roleID;
						r.status=1;//ID�Լ���״̬������
						li.push_back(r);
						NotifyTeamChang(roleID,1,it);
						NotifyTeamInfo(roleID,li);
					}
			}
			else
			{
				NotifyRefuse(roleID,ID);
				//��ͬ��
			}
		}
		return;
}
void TeamSvc::ProcessChangeCaptain(Session& session,Packet& packet)//����Ϊ�ӳ�
{
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);

	UInt32 roleID = packet.RoleID;
	UInt32 ID=0;//�������Ľ�ɫID
	string Name;
	List<UInt32> it;
	UInt32 teamID=0;
	int iRet=0;
	Team team;

	list<TeamRole>::iterator itor;
	list<TeamRole> li;
	
		//���л���
	Serializer s(packet.GetBuffer());
	s>>ID;
	
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}
	if(ID==0||ID==roleID)
	{
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"there are some erro in the ID %d vs ResustID%d",ID,roleID );
			goto EndOf_Process;
	}
	//�������ж�	
	team=_mainSvc->GetCoreData()->GetTeamsFromRoleID(roleID);
	teamID=team.GetTeamID();
	if(team.GetLeaderRoleID()!=roleID)
	{
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"you are not the leader !" );
			goto EndOf_Process;
	}
	
	
	iRet=_mainSvc->GetCoreData()->ChangtoBeLeader(teamID,ID);
	if(iRet!=0)
	{
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"RoleID is the leader  (%d) or Not find the team ",ID );
			goto EndOf_Process;
	}
	


	

	DEBUG_PRINTF( "C_S ProcessProcessChat sucess!!!!!!!!\n" );	


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
	}	

	p.UpdatePacketLength();
	
	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}
	
	DEBUG_PRINTF( "ack pkg=======, \n" );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );
	if(0==RetCode)
	{
		li=team.GetMemberRoleID();
		for(itor=li.begin();itor!=li.end();itor++)
		{
					it.push_back(itor->roleId);
		}
	  NotifyTeamChang(ID,2,it);//��Ϊ�ӳ�,���ö�������Ա
	  NotifyTeamChang(roleID,1,it);
	}
	return;
}

//�ߵ���Ա
void TeamSvc::ProcessCapt_Member(Session& session,Packet& packet)//�߳�����
{
		UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);

	UInt32 roleID = packet.RoleID;
	UInt32 ID=0;//�������Ľ�ɫID
	string Name;
	List<UInt32> it;
	UInt32 teamID=0;
	int iRet=0;
	Team team;

	list<TeamRole>::iterator itor;
	list<TeamRole> li;
	
		//���л���
	Serializer s(packet.GetBuffer());
	s>>ID;
	
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}
	if(ID==0||ID==roleID)
	{
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"there are some erro in the ID %d vs ResustID%d",ID,roleID );
			goto EndOf_Process;
	}
	//�������ж�	
	team=_mainSvc->GetCoreData()->GetTeamsFromRoleID(roleID);
	teamID=team.GetTeamID();
	if(team.GetLeaderRoleID()!=roleID)
	{
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"you are not the leader !" );
			goto EndOf_Process;
	}
	iRet=_mainSvc->GetCoreData()->DeleteTeamRole(teamID,ID);
	if(iRet==-1)
	{
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"there are some erro!in itw !" );
			goto EndOf_Process;
	}
	
	
	//�ߵ���Ա
	


	

	DEBUG_PRINTF( "C_S ProcessProcessChat sucess!!!!!!!!\n" );	


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
	}	

	p.UpdatePacketLength();
	
	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}
	
	DEBUG_PRINTF( "ack pkg=======, \n" );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );
	if(0==RetCode)
	{
		li=team.GetMemberRoleID();
		for(itor=li.begin();itor!=li.end();itor++)
		{
					it.push_back(itor->roleId);
		}
	  NotifyTeamChang(ID,5,it);//s-c���ö������˽���,���г�Ա,include self
	}
	return;
}

//�ٻ���Ա
void TeamSvc::ProcessCallMember(Session& session,Packet& packet)
{
			UInt32	RetCode = 0;
			DataBuffer	serbuffer(1024);

			UInt32 roleID = packet.RoleID;
			UInt32 ID=0;//�������Ľ�ɫID
			string Name;
			List<UInt32> it;
			UInt32 teamID=0;
			int iRet=0;
			int flag=0;
			Team team;
			list<TeamRole>::iterator itor;
			list<TeamRole> t;
				//���л���
			Serializer s(packet.GetBuffer());
			s>>ID;
			
			if( s.GetErrorCode()!= 0 )
			{
				RetCode = ERR_SYSTEM_SERERROR;
				LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
				goto EndOf_Process;
			}
			if(ID==0||ID==roleID)
			{
					RetCode = ERR_SYSTEM_SERERROR;
					LOG(LOG_ERROR,__FILE__,__LINE__,"there are some erro in the ID %d vs ResustID%d",ID,roleID );
					goto EndOf_Process;
			}
			//�ж��Ƿ��Ƕӳ�
			team=_mainSvc->GetCoreData()->GetTeamsFromRoleID(roleID);
			teamID=team.GetTeamID();
			if(team.GetLeaderRoleID()!=roleID)
			{
					RetCode = ERR_SYSTEM_SERERROR;
					LOG(LOG_ERROR,__FILE__,__LINE__,"you are not the leader !" );
					goto EndOf_Process;
			}
			//�������ж�	,�ж��Ƿ��ڶ����ڲ�
			t=team.GetMemberRoleID();
			for(itor= t.begin();itor!=t.end();itor++)
			{
				if(itor->roleId==roleID)
				{
					flag=1;
				}
			}
			if(flag==0)
			{
				//���Ƕ����ڳ�Ա
					RetCode = ERR_SYSTEM_SERERROR;
					LOG(LOG_ERROR,__FILE__,__LINE__,"you are not the leader !" );
					goto EndOf_Process;
			}

			

			DEBUG_PRINTF( "C_S ProcessProcessChat sucess!!!!!!!!\n" );	


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
			}	

			p.UpdatePacketLength();
			
			//����Ӧ������
			if( session.Send(&serbuffer) )
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
			}
			
			DEBUG_PRINTF( "ack pkg=======, \n" );
			DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );
			if(0==RetCode)
			{
				it.push_back(ID);
			  NotifyCaptionCall(it);//ok
			}
			return;
}
//��Ա�Լ������˳���
void TeamSvc::ProcessMember_OUt(Session& session,Packet& packet)//�˳�����
{
			UInt32	RetCode = 0;
			DataBuffer	serbuffer(1024);

			UInt32 roleID = packet.RoleID;
			string Name;
			List<UInt32> it;
			UInt32 teamID=0;
			int iRet=0;
			int flag=0;
			Team team;
			UInt32 ID=0;

			list<TeamRole>::iterator itor;
			list<TeamRole> li;
				//���л���
			Serializer s(packet.GetBuffer());
			
			
			if( s.GetErrorCode()!= 0 )
			{
				RetCode = ERR_SYSTEM_SERERROR;
				LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
				goto EndOf_Process;
			}

			team=_mainSvc->GetCoreData()->GetTeamsFromRoleID(roleID);
			teamID=team.GetTeamID();

			li=team.GetMemberRoleID();
			if(teamID==0)
			{
				//û�ж���
					RetCode = ERR_SYSTEM_SERERROR;
					LOG(LOG_ERROR,__FILE__,__LINE__,"there are some erro in the ID %d vs ResustID%d",ID,roleID );
					goto EndOf_Process;
				
			}
			//�ж��Ƿ��Ƕӳ�
			if(team.GetLeaderRoleID()==roleID&&li.size()>1)
			{
				
				//ID,Ϊ����Ķӳ�
					for(itor=li.begin();itor!=li.end();itor++)
					{
						if(itor->roleId!=roleID)
						{
							ID=itor->roleId;
							break;
						}
					}
					if(ID!=0)
					{
						_mainSvc->GetCoreData()->ChangtoBeLeader(teamID,ID);
					}
					//�����ӳ����Ȼ����ӳ���Ȼ��ȥ����Ա
			}
			//ȥ����Ա

			iRet=_mainSvc->GetCoreData()->DeleteTeamRole(teamID,roleID);
			
			

			DEBUG_PRINTF( "C_S ProcessProcessChat sucess!!!!!!!!\n" );	


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
			}	

			p.UpdatePacketLength();
			
			//����Ӧ������
			if( session.Send(&serbuffer) )
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
			}
			
			DEBUG_PRINTF( "ack pkg=======, \n" );
			DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );
			if(0==RetCode)
			{
				li.clear();
				li=team.GetMemberRoleID();
				for(itor=li.begin();itor!=li.end();itor++)
				{
						if(roleID!=itor->roleId)
							it.push_back(itor->roleId);
				}
				//it.push_back(ID);
				 NotifyTeamChang(roleID,5,it);
			  if(ID!=0)
			  {
			  	NotifyTeamChang(ID,2,it);
			  }
			}
			return;
}
void TeamSvc::ProcessBackOrleave(Session& session,Packet& packet)//��ӻ������
{
			UInt32	RetCode = 0;
			DataBuffer	serbuffer(1024);

			UInt32 roleID = packet.RoleID;
			UInt32 ID=0;//�������Ľ�ɫID
			string Name;
			List<UInt32> it;
			UInt32 teamID=0;
			int iRet=0;
			int flag=0;
			Team team;
			Byte oPflag=0;
			list<TeamRole>::iterator itor;
			list<TeamRole> li;
				//���л���
			Serializer s(packet.GetBuffer());
			s>>oPflag;
			
			if( s.GetErrorCode()!= 0 )
			{
				RetCode = ERR_SYSTEM_SERERROR;
				LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
				goto EndOf_Process;
			}

			team=_mainSvc->GetCoreData()->GetTeamsFromRoleID(roleID);
			teamID=team.GetTeamID();
			if(oPflag==1)//���
			{
				_mainSvc->GetCoreData()->ChangtoFlag(teamID,roleID,1);
			}
			else if(oPflag==2)//���	
			{
				_mainSvc->GetCoreData()->ChangtoFlag(teamID,roleID,2);
			}
		
			

			DEBUG_PRINTF( "C_S ProcessProcessChat sucess!!!!!!!!\n" );	


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
			}	

			p.UpdatePacketLength();
			
			//����Ӧ������
			if( session.Send(&serbuffer) )
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
			}
			
			DEBUG_PRINTF( "ack pkg=======, \n" );
			DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );
			if(0==RetCode)
			{
				li=team.GetMemberRoleID();
				for(itor=li.begin();itor!=li.end();itor++)
				{
							
							if(oPflag!=0)
							{
								it.push_back(itor->roleId);
								LOG(LOG_ERROR,__FILE__,__LINE__,"ID %d     RoleID %d",itor->roleId,itor->status);
							}
									
				}
			  if(oPflag==0)//����
			  {
			  	NotifyTeamChang(roleID,4,it);
			  }
			  if(oPflag==1)//���
			  {
			  	NotifyTeamChang(roleID,1,it);
			  }
			  if(oPflag==2)//���
			  {
			  	NotifyTeamChang(roleID,3,it);
			  }
			}
			return;
}
void TeamSvc::NotifyReceive(UInt32 RoleID,string Name,List<UInt32>& it)
{
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );	
	Packet p(&serbuffer);
	//���ͷ
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1401;
	p.UniqID = 123;

	p.PackHeader();


	//д����
	s<<RoleID<<Name;
	p.UpdatePacketLength();

	if( _mainSvc->Service()->Broadcast( it, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
	}

DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size()[%d]  \n", p.MsgType, it.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
	
}

void TeamSvc::NotifyTeamChang(UInt32 RoleID,Byte changeType,List<UInt32>& it)
{
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );	
	Packet p(&serbuffer);

	//���ͷ
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1402;
	p.UniqID = 123;
	p.PackHeader();

	//д����
	s<<RoleID<<changeType;
	p.UpdatePacketLength();


	if( _mainSvc->Service()->Broadcast( it, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
	}

DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size()[%d]  \n", p.MsgType, it.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
	
}

void TeamSvc::NotifyCaptionCall(List<UInt32>& it)
{
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );	
	Packet p(&serbuffer);

	//���ͷ
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1403;
	p.UniqID = 123;

	p.PackHeader();
	//д����,
	//����յ�
	p.UpdatePacketLength();

	if( _mainSvc->Service()->Broadcast( it, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
	}

DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size()[%d]  \n", p.MsgType, it.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
	
}
 	void TeamSvc::NotifyRefuse(UInt32 RoleID,UInt32 ID)//˭�ܾ�˭��RoleID��ָ˭ID���յ�����
	{
			DataBuffer	serbuffer(8196);
			Serializer s( &serbuffer );	
			Packet p(&serbuffer);
			List<UInt32> it;
			//���ͷ
			serbuffer.Reset();
			p.Direction = DIRECT_S_C_REQ;
			p.SvrType = 1;
			p.SvrSeq = 1;
			p.MsgType = 1404;
			p.UniqID = 123;

			p.PackHeader();
			//д����,
			s<<RoleID;
			//����յ�
			p.UpdatePacketLength();
			it.push_back(ID);
			if( _mainSvc->Service()->Broadcast( it, &serbuffer))
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
			}

		DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size()[%d]  \n", p.MsgType, it.size() );
			DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
			
		
	}

	
	void TeamSvc::NotifyTeamInfo(UInt32 RoleID, list<TeamRole>& l)
	{
			DataBuffer	serbuffer(8196);
			Serializer s( &serbuffer );	
			Packet p(&serbuffer);
			list<TeamRole>::iterator itor;
			List<TeamRole> li;
			TeamRole r;
			
			List<UInt32> it;
			//���ͷ
			serbuffer.Reset();
			p.Direction = DIRECT_S_C_REQ;
			p.SvrType = 1;
			p.SvrSeq = 1;
			p.MsgType = 1405;
			p.UniqID = 123;

			p.PackHeader();
			//д����,
			//����յ�
			for(itor=l.begin();itor!=l.end();itor++)
			{
				r=(*itor);
				li.push_back(r);
			}
			s<<li;
			p.UpdatePacketLength();
			it.push_back(RoleID);
			
			if( _mainSvc->Service()->Broadcast( it, &serbuffer))
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
			}

		DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size()[%d]  \n", p.MsgType, it.size() );
			DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
			
		
	}

