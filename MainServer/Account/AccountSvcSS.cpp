#include "AccountSvcSS.h"
#include "DBOperate.h"
#include "DebugData.h"
#include "Packet.h"
#include "ArchvRole.h"
#include "MainSvc.h"
#include "CoreData.h"
#include "ArchvAccount.h"


AccountSvcSS::AccountSvcSS(MainSvc * mainSvc, ConnectionPool *cp )
:_mainSvc(mainSvc)
,_cp(cp)
{
}

AccountSvcSS::~AccountSvcSS()
{
}

void AccountSvcSS::OnProcessPacket(Session& session,Packet& packet)
{

DEBUG_PRINTF1( "S_S req pkg->->->->->->MsgType[%d]\n", packet.MsgType );
	DEBUG_SHOWHEX( packet.GetBuffer()->GetReadPtr()-PACKET_HEADER_LENGTH, packet.GetBuffer()->GetDataSize()+PACKET_HEADER_LENGTH, 0, __FILE__, __LINE__ );
//	LOG(LOG_ERROR,__FILE__,__LINE__,"MSG(%d)",packet.MsgType);

	switch(packet.MsgType)
	{
		case 301: // Accountע��
			ProcessAccountReg(session,packet);
			break;

		case 302: // Account��½
			ProcessAccountLogin(session,packet);
			break;

		case 303: // ��ѯAccount��Rolesӳ��
			ProcessGetAccountRoles(session,packet);
			break;

		case 304: // ����Role
			ProcessAddRole(session,packet);
			break;

		case 305: // ɾ��Role
			ProcessDelRole(session,packet);
			break;

		case 306: // �����֤֪ͨ����ÿ��C_S��½ǰ���ͣ�
			ProcessIdNotify(session,packet);
			break;

		case 307: // �����Լ��
			ProcessPlayTimeCheck(session,packet);
			break;

		case 308: // ���֤ע��
			ProcessIDCardReg(session,packet);
			break;

		case 399: // Session��֤
			ProcessCheckSession(session,packet);
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
void AccountSvcSS::ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode)
{
	//��Ӧ������
	DataBuffer	serbuffer(1024);
	Packet p(&serbuffer);
	Serializer s(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_S_S_RESP;
	p.PackHeader();

	s<<RetCode;
	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF1( "S_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );

}

//@brief	����ְҵ,������
//@return 0 �ɹ� ��0  ʧ��
int AccountSvcSS::SetProAttr( Byte proID, RolePtr& role )
{
	//����ְҵ,������
	switch(proID)
	{
		case 1:		//����սʿ
			role->Strength(15);
			role->Intelligence(5);
			role->Agility(10);
			role->Hp(510);
			role->Mp(180);
			role->MaxHp(510);
			role->MaxMp(180);
			role->HpRegen(10);
			role->MpRegen(10);
			role->AttackPowerHigh(55);
			role->AttackPowerLow(40);
			role->AttackScope(3);
			role->AttackSpeed(21);
			role->BulletSpeed(0);
			role->Defence(0);
			role->MDefence(0);
			role->CritRate(0);
			role->HitRate(70);
			role->MoveSpeed(240);
			break;

		case 2:		//��������
			role->Strength(10);
			role->Intelligence(10);
			role->Agility(10);
			role->Hp(460);
			role->Mp(260);
			role->MaxHp(460);
			role->MaxMp(260);
			role->HpRegen(10);
			role->MpRegen(10);
			role->AttackPowerHigh(48);
			role->AttackPowerLow(42);
			role->AttackScope(3);
			role->AttackSpeed(20);
			role->BulletSpeed(0);
			role->Defence(0);
			role->MDefence(0);
			role->CritRate(0);
			role->HitRate(70);
			role->MoveSpeed(260);
			break;

		case 3:		//���鹭����
			role->Strength(9);
			role->Intelligence(6);
			role->Agility(15);
			role->Hp(410);
			role->Mp(240);
			role->MaxHp(410);
			role->MaxMp(240);
			role->HpRegen(10);
			role->MpRegen(10);
			role->AttackPowerHigh(44);
			role->AttackPowerLow(36);
			role->AttackScope(18);
			role->AttackSpeed(19);
			role->BulletSpeed(2);
			role->Defence(0);
			role->MDefence(0);
			role->CritRate(0);
			role->HitRate(70);
			role->MoveSpeed(300);
			break;

		case 4:		//�����з�ʦ
			role->Strength(8);
			role->Intelligence(13);
			role->Agility(9);
			role->Hp(430);
			role->Mp(380);
			role->MaxHp(430);
			role->MaxMp(380);
			role->HpRegen(10);
			role->MpRegen(10);
			role->AttackPowerHigh(42);
			role->AttackPowerLow(32);
			role->AttackScope(15);
			role->AttackSpeed(20);
			role->BulletSpeed(2);
			role->Defence(0);
			role->MDefence(0);
			role->CritRate(0);
			role->HitRate(70);
			role->MoveSpeed(270);
			break;

		case 5:		//����Ů��ʦ
			role->Strength(5);
			role->Intelligence(14);
			role->Agility(11);
			role->Hp(380);
			role->Mp(360);
			role->MaxHp(380);
			role->MaxMp(360);
			role->HpRegen(10);
			role->MpRegen(10);
			role->AttackPowerHigh(40);
			role->AttackPowerLow(32);
			role->AttackScope(15);
			role->AttackSpeed(21);
			role->BulletSpeed(2);
			role->Defence(0);
			role->MDefence(0);
			role->CritRate(0);
			role->HitRate(70);
			role->MoveSpeed(290);
			break;

		case 6:		//��ì��
			role->Strength(11);
			role->Intelligence(6);
			role->Agility(13);
			role->Hp(450);
			role->Mp(280);
			role->MaxHp(450);
			role->MaxMp(280);
			role->HpRegen(10);
			role->MpRegen(10);
			role->AttackPowerHigh(60);
			role->AttackPowerLow(42);
			role->AttackScope(12);
			role->AttackSpeed(19);
			role->BulletSpeed(2);
			role->Defence(0);
			role->MDefence(0);
			role->CritRate(0);
			role->HitRate(70);
			role->MoveSpeed(280);
			break;



		default:
			LOG(LOG_ERROR,__FILE__,__LINE__,"error proID! proID[%d] " , proID);
			return -1;
	}

	return 0;
}

//@brief	���ӽ�ɫ
//@roleID	���صĽ�ɫID	,ֻ�ڳɹ�����ʱ��Ч
//@return 0 �ɹ� ��0  ʧ��
int AccountSvcSS::AddRole( UInt32 accountID, const string& passwd, const string& roleName, Byte proID, UInt32& roleID )
{
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con,con1;
	DBOperate dbo,dbo1;
	int iRet = 0;
	RolePtr role(new Role());

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	con1=_cp->GetConnection();
	dbo1.SetHandle(con1);
	//��ɫ���ԣ���ʼֵ
	role->Name(roleName);
	role->Password(passwd);
	role->ProID(proID);
	role->Level(1);
	role->Exp(0);
	role->MaxExp(20);
	role->Camp(1);

	//����ְҵ��������
	iRet = SetProAttr( proID, role);
	if(iRet)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"SetProAttr error!! " );
		return -1;
	}

	//�����ɫ��
	sprintf( szSql, "insert into Role( RoleID, AccountID  ,RoleName, Password ,Level    ,Exp,MaxExp,Camp \
											,ProID ,GuildID ,Glory ,AddPoint,Strength ,Intelligence   ,Agility \
											,HP    ,MP   ,MaxHP, MaxMP, HPRegen, MPRegen \
											,AttackPowerHigh, AttackPowerLow, AttackScope, AttackSpeed, BulletSpeed \
											,Defence  ,MDefence ,CritRate , HitRate, Crime \
											,TotalOnlineSec, MoveSpeed ) \
										values( NULL, %d, '%s', '%s', %d, %d, %d, %d \
											,%d ,%d ,%d, %d, %d, %d, %d \
											,%d, %d, %d, %d, %d, %d \
											,%d, %d, %d, %d, %d \
											,%d, %d, %d, %d, %d \
											,%d, %d	 ) " ,
											accountID, role->Name().c_str(), role->Password().c_str(), role->Level(), role->Exp(), role->MaxExp(), role->Camp(),
											role->ProID(), role->GuildID(), role->Glory(), role->AddPoint(), role->Strength(), role->Intelligence(), role->Agility(),
											role->Hp(), role->Mp(), role->MaxHp(), role->MaxMp(), role->HpRegen(), role->MpRegen(),
											role->AttackPowerHigh(), role->AttackPowerLow(), role->AttackScope(), role->AttackSpeed(), role->BulletSpeed(),
											role->Defence(), role->MDefence(), role->CritRate(), role->HitRate(), role->Crime(),
											role->TotalOnlineSec(), role->MoveSpeed() );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}

	//��ȡ ��ɫID
	sprintf( szSql, "select last_insert_id()" );
	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
		return -1;
	}
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}
 	roleID = dbo.GetIntField(0);

 	//�����ɫ��������
	sprintf( szSql, "insert into RoleLastLoc( RoleID, MapId, LastX, LastY ) \
										values( %d, %d, %d, %d )",
											roleID, 1, 146, 69 );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}

	//�����ɫ��Ǯ
	sprintf( szSql, "insert into RoleMoney(RoleID,Money)	\
										values( %d, %d )",
											roleID, 10000 );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}

	for(int i=0;i<12;i++)
	{
		//�����ɫ��ʼ����
		sprintf( szSql, "insert into RoleSkill(RoleID,SkillID,SkillLev) \
										values( %d,%d,0)",roleID,i+301 );
		iRet = dbo.ExceSQL(szSql);
		if( iRet < 0 )
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			return -1;
	 	}

	}

	sprintf( szSql, "select SkillID from ProSkillDesc where ProID=%d",proID);
		iRet = dbo.QuerySQL(szSql);
		if( iRet != 0 )
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			return -1;
	 	}
		while(dbo.HasRowData())
		{
				sprintf( szSql, "insert into RoleSkill(RoleID,SkillID,SkillLev) \
										values( %d,%d,0)",roleID,dbo.GetIntField(0));
					iRet=dbo1.ExceSQL(szSql);
					if( iRet < 0 )
						{
							LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con1.GetHandle()), szSql);
							return -1;
					 	}
				dbo.NextRow();
		}

 	//װ��
	for( int i = 0; i < 13; i++ )
	{
		sprintf( szSql, "insert into Equip( RoleID, EquipIndex, ItemType, ItemID, EntityID) \
											values( %d, %d, 0, 0, 0 )",
												roleID, i+1 );
		iRet = dbo.ExceSQL(szSql);
		if( iRet < 0 )
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			return -1;
		}
	}

	//��ɫ���Լӳ�
	sprintf( szSql, "insert into RoleBonus(RoleID ,Strength,Intelligence   ,Agility \
											,MovSpeed,MaxHP  ,MaxMP  ,HPRegen \
											,MPRegen,AttackPowerHigh,AttackPowerLow ,AttackSpeed \
											,Defence,MDefence,CritRate,HitRate \
											,DodgeRae) \
										values( %d, 0, 0, 0  \
											, 0, 0, 0, 0 \
											, 0, 0, 0, 0 \
											, 0, 0, 0, 0 \
											, 0 )",
												roleID );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}

	//Ĭ�����ӵ�һ������
	sprintf( szSql, "insert into RoleTask(RoleID, TaskID, FinishStatus, DeliverNum)	\
										values( %d, 1, 1, 0 )", roleID );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
 	}


	return 0;
}


void AccountSvcSS::ProcessAccountReg(Session& session,Packet& packet)
{

	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	string accountName;
	string passwd;
	Byte	isAdult = 0;

 	//���л���
	Serializer s(packet.GetBuffer());
	s>>accountName>>passwd>>isAdult;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//�ֶν�ȡ,ȡǰ 64λ
	accountName = accountName.substr(0,64);
	passwd = passwd.substr(0,64);


	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//�˺����Ƿ��Ѵ���
	sprintf( szSql, "select AccountID \
									from Account \
									where AccountName = '%s' ", accountName.c_str() );
	iRet = dbo.QuerySQL(szSql);
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}
 	if( 1 != iRet )
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__," account exists!! accountName[%s]", accountName.c_str() );
		goto EndOf_Process;
	}

	//���� Account ��
	sprintf( szSql, "insert into Account( AccountName, Password, IsAdult, SessionID ) \
											values( '%s', '%s', %d, %d ) ",
										accountName.c_str(), passwd.c_str(), isAdult, time(NULL) );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}


EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_S_S_RESP;
	p.PackHeader();


	s<<RetCode;
	if( 0 == RetCode )
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
//		s<<ri;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF1( "S_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );


	//---------------------------------S_C ֪ͨ-----------------------------------------
	//�ɹ��������͹㲥
	if( 0 == RetCode )
	{
	}

}




void AccountSvcSS::ProcessAccountLogin(Session& session,Packet& packet)
{

	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	string accountName;
	string inputPasswd;
	string passwd;
	Byte	isAdult = 0;
	UInt32 sessionID = 0;

 	//���л���
	Serializer s(packet.GetBuffer());
	s>>accountName>>inputPasswd;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//�ֶν�ȡ,ȡǰ 64λ
	accountName = accountName.substr(0,64);
	inputPasswd = inputPasswd.substr(0,64);


	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//��ȡ����
	sprintf( szSql, "select Password \
									from Account \
									where AccountName = '%s' ", accountName.c_str() );
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
 	passwd = dbo.GetStringField(0);

 	//����У��
 	if( strncmp( inputPasswd.c_str(), passwd.c_str(), 64 ) )
 	{
 		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"passwd error!! inputPasswd[%s],passwd[%s] ", inputPasswd.c_str(), passwd.c_str() );
		goto EndOf_Process;
 	}

	//��ȡ sessionID
	sessionID = time(NULL);


	//���� Account �� sessionID
	sprintf( szSql, "update Account	\
										set SessionID = %d \
										where AccountName = '%s' ",
										sessionID, accountName.c_str() );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}


EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_S_S_RESP;
	p.PackHeader();


	s<<RetCode;
	if( 0 == RetCode )
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
		s<<sessionID;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF1( "S_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );


	//---------------------------------S_C ֪ͨ-----------------------------------------
	//�ɹ��������͹㲥
	if( 0 == RetCode )
	{
	}

}







void AccountSvcSS::ProcessGetAccountRoles(Session& session,Packet& packet)
{

	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	string accountName;
	UInt32 inputSessionID = 0;
	UInt32 accountID = 0;
	ArchvAccountRoleInfo ari;
	List<ArchvAccountRoleInfo> lari;

 	//���л���
	Serializer s(packet.GetBuffer());
	s>>accountName>>inputSessionID;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//�ֶν�ȡ,ȡǰ 64λ
	accountName = accountName.substr(0,64);

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//�˺��Ƿ����
	sprintf( szSql, "select AccountID \
									from Account \
									where AccountName = '%s' ", accountName.c_str() );
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

 	accountID = dbo.GetIntField(0);

DEBUG_PRINTF1( "==accountName[%s]", accountName.c_str());

	//��ȡ�˺Ŷ�Ӧ�Ľ�ɫ��Ϣ
	sprintf( szSql, "select RoleID, RoleName, ProID, Level \
									from Role \
									where AccountID = %d ", accountID );
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
		ari.roleID = dbo.GetIntField(0);
		ari.roleName = dbo.GetStringField(1);
		ari.proID = dbo.GetIntField(2);
		ari.level = dbo.GetIntField(3);

		lari.push_back( ari );

		//��¼����һ����¼
		dbo.NextRow();
	}



EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_S_S_RESP;
	p.PackHeader();


	s<<RetCode;
	if( 0 == RetCode )
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
		s<<lari;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF1( "S_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );


	//---------------------------------S_C ֪ͨ-----------------------------------------
	//�ɹ��������͹㲥
	if( 0 == RetCode )
	{
	}

}


void AccountSvcSS::ProcessAddRole(Session& session,Packet& packet)
{

	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	string str="echo ' ";
	FILE   *stream;
	char   buf[1024];
	string str2=" | egrep '";
	string str3="' |wc";
	FILE *fp;
	char gg=0;
	const char *strs;


	string accountName;
	string passwd;
	string inputRoleName;
	UInt32 inputSessionID = 0;
	Byte proID = 0;
	UInt32 roleID = 0;
	UInt32 accountID = 0;
	UInt32 roleNum = 0;
	UInt32 level = 1;

 	//���л���
	Serializer s(packet.GetBuffer());
	s>>accountName>>inputSessionID>>proID>>inputRoleName;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//�ֶν�ȡ,ȡǰ 64λ
	accountName = accountName.substr(0,64);

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//�˺��Ƿ����
	sprintf( szSql, "select AccountID, Password \
									from Account \
									where AccountName = '%s' ", accountName.c_str() );
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

 	accountID = dbo.GetIntField(0);
 	passwd = dbo.GetStringField(1);

	//��ȡ�˺Ŷ�Ӧ�Ľ�ɫ����
	sprintf( szSql, "select count(RoleID) \
									from Role \
									where AccountID = %d ", accountID );
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

 	roleNum = dbo.GetIntField(0);

	//��ɫ��������У��
	if( roleNum >=5 )
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"roleNum limit!! curr roleNum[%d] ", roleNum );
		goto EndOf_Process;
	}

	if(inputRoleName.size()>20)//����20
	{
		RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"%c ", buf[6] );
		goto EndOf_Process;
	}


	sprintf( szSql, "select RoleName \
									from Role \
									where RoleName = '%s' ", inputRoleName.c_str() );
	iRet = dbo.QuerySQL(szSql);
	if( 0 == iRet )
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
		goto EndOf_Process;
	}

	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}


	/*str=str+inputRoleName;

	str.push_back(39);
	str=str+str2;

	if ((fp=fopen("./filters.txt","r"))==NULL)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"can not open the file ");
	}
	do{
			 gg=getc(fp);
			 str.push_back(gg);

			}while(gg!='&');

	fclose(fp);
	str=str+str3;
	memset( buf, 0, sizeof(buf) );
	strs=str.c_str();
	stream = popen( strs, "r" );

	fread( buf, sizeof(char), sizeof(buf), stream);
	if(buf[6]!='0')
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"illegal world!! system() retCode[%c] ", buf[6] );
		goto EndOf_Process;
	}
	*/
	//���ӽ�ɫ
	iRet = AddRole( accountID, passwd, inputRoleName, proID, roleID );

	if(iRet)
	{
		RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"AddRole error!! accountID[%d], inputRoleName[%s] ", accountID, inputRoleName.c_str() );
		goto EndOf_Process;
	}


EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_S_S_RESP;
	p.PackHeader();


	s<<RetCode;
	if( 0 == RetCode )
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
		s<<roleID<<inputRoleName<<proID<<level;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF1( "S_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );


	//---------------------------------S_C ֪ͨ-----------------------------------------
	//�ɹ��������͹㲥
	if( 0 == RetCode )
	{
	}

}



void AccountSvcSS::ProcessDelRole(Session& session,Packet& packet)
{

	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	string accountName;
	UInt32 inputSessionID = 0;
	UInt32 roleID = 0;
	UInt32 accountID = 0;

 	//���л���
	Serializer s(packet.GetBuffer());
	s>>accountName>>inputSessionID>>roleID;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//�ֶν�ȡ,ȡǰ 64λ
	accountName = accountName.substr(0,64);

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//�˺��Ƿ����
	sprintf( szSql, "select AccountID \
									from Account \
									where AccountName = '%s' ", accountName.c_str() );
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

 	accountID = dbo.GetIntField(0);

	//��ȡ�˺Ŷ�Ӧ�Ľ�ɫ��Ϣ
	sprintf( szSql, "select RoleID \
									from Role \
									where RoleID = %d \
										and AccountID = %d ", roleID, accountID );
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

 	//ɾ����ɫ
	sprintf( szSql, "delete	from Role \
									where RoleID = %d ", roleID );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}


 	//ɾ����ɫ��������
	sprintf( szSql, "delete	from RoleLastLoc \
									where RoleID = %d ", roleID );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}

	//ɾ����ɫ��Ǯ
	sprintf( szSql, "delete	from RoleMoney \
									where RoleID = %d ", roleID );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}


	//ɾ����ɫ����
	sprintf( szSql, "delete	from RoleSkill \
									where RoleID = %d ", roleID );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}

 	//ɾ����ɫװ��
	sprintf( szSql, "delete	from Equip \
									where RoleID = %d ", roleID );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}

	//ɾ����ɫ���Լӳ�
	sprintf( szSql, "delete	from RoleBonus \
									where RoleID = %d ", roleID );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}


EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_S_S_RESP;
	p.PackHeader();


	s<<RetCode;
	if( 0 == RetCode )
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
//		s<<lari;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF1( "S_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );


	//---------------------------------S_C ֪ͨ-----------------------------------------
	//�ɹ��������͹㲥
	if( 0 == RetCode )
	{
	}

}



void AccountSvcSS::ProcessIdNotify(Session& session,Packet& packet)
{

	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	string accountName;
	UInt32 inputSessionID = 0;
	Byte	isAdult = 0;
	UInt32 accountID = 0;

 	//���л���
	Serializer s(packet.GetBuffer());
	s>>accountName>>inputSessionID>>isAdult;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//�ֶν�ȡ,ȡǰ 64λ
	accountName = accountName.substr(0,64);


	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//�˺��Ƿ����
	sprintf( szSql, "select AccountID \
									from Account \
									where AccountName = '%s' ", accountName.c_str() );
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
 	accountID = dbo.GetIntField(0);

 	//���ݵ���
 	if( 0 != isAdult )
 		isAdult = 1;


	//���� Account ��
	sprintf( szSql, "update Account	\
										set IsAdult = %d \
										where AccountID = %d ",
										isAdult, accountID );
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}


EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_S_S_RESP;
	p.PackHeader();


	s<<RetCode;
	if( 0 == RetCode )
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
//		s<<sessionID;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF1( "S_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );


	//---------------------------------S_C ֪ͨ-----------------------------------------
	//�ɹ��������͹㲥
	if( 0 == RetCode )
	{
	}

}





void AccountSvcSS::ProcessPlayTimeCheck(Session& session,Packet& packet)
{

	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	string accountName;
	UInt32 inputSessionID = 0;
	Byte	isAdult = 0;
	UInt32 accountID = 0;

 	//���л���
	Serializer s(packet.GetBuffer());
	s>>accountName>>inputSessionID;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//�ֶν�ȡ,ȡǰ 64λ
	accountName = accountName.substr(0,64);


	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//�˺��Ƿ����
	sprintf( szSql, "select AccountID \
									from Account \
									where AccountName = '%s' ", accountName.c_str() );
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
 	accountID = dbo.GetIntField(0);

 	//�˺ŷ����Լ��
 	iRet = _mainSvc->GetCoreData()->AccountPlaytimeAccess(accountID);
 	if(iRet)
 	{
		RetCode = ERR_APP_PLAYTIMEACCESS;
		LOG(LOG_ERROR,__FILE__,__LINE__,"AccountPlaytimeAccess error! accountID[%d] ", accountID );
		goto EndOf_Process;
 	}


EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_S_S_RESP;
	p.PackHeader();


	s<<RetCode;
	if( 0 == RetCode )
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
//		s<<sessionID;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF1( "S_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );


	//---------------------------------S_C ֪ͨ-----------------------------------------
	//�ɹ��������͹㲥
	if( 0 == RetCode )
	{
	}

}

void AccountSvcSS::ProcessIDCardReg(Session& session,Packet& packet)
{

	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	string accountName;
	UInt32 inputSessionID = 0;
	string name;
	string IDCardNum;
	UInt32 accountID = 0;

 	//���л���
	Serializer s(packet.GetBuffer());
	s>>accountName>>inputSessionID>>name>>IDCardNum;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//�ֶν�ȡ,ȡǰ 64λ
	accountName = accountName.substr(0,64);


	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//�˺��Ƿ����
	sprintf( szSql, "select AccountID \
									from Account \
									where AccountName = '%s' ", accountName.c_str() );
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
 	accountID = dbo.GetIntField(0);

	//���֤ע��
	iRet = _mainSvc->GetCoreData()->AccountIsAdult( accountID, 1, name, IDCardNum );
LOG(LOG_ERROR,__FILE__,__LINE__,"+++++nname[%s] ", name.c_str() );


	if( 1 == iRet )
	{
		RetCode = ERR_SYSTEM_DATAEXISTS;
		LOG(LOG_ERROR,__FILE__,__LINE__,"Name or IDCardNum exists!!name[%s],IDCardNum[%s] ", name.c_str(), IDCardNum.c_str() );
		goto EndOf_Process;
 	}
 	else if(iRet)
 	{
		RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"Name or IDCardNum exists!!name[%s],IDCardNum[%s] ", name.c_str(), IDCardNum.c_str() );
		goto EndOf_Process;
 	}



EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_S_S_RESP;
	p.PackHeader();


	s<<RetCode;
	if( 0 == RetCode )
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
//		s<<sessionID;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF1( "S_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );


	//---------------------------------S_C ֪ͨ-----------------------------------------
	//�ɹ��������͹㲥
	if( 0 == RetCode )
	{
	}

}



void AccountSvcSS::ProcessCheckSession(Session& session,Packet& packet)
{

	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	string accountName;
	UInt32 inputSessionID = 0;
	UInt32 sessionID = 0;

 	//���л���
	Serializer s(packet.GetBuffer());
	s>>accountName>>inputSessionID;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//�ֶν�ȡ,ȡǰ 64λ
	accountName = accountName.substr(0,64);


	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//��ȡ sessionID
	sprintf( szSql, "select SessionID \
									from Account \
									where AccountName = '%s' ", accountName.c_str() );
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
 	sessionID = dbo.GetIntField(0);

 	// sessionID У��
 	if( inputSessionID != sessionID )
 	{
 		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"sessionID error!! inputSessionID[%d],sessionID[%d] ", inputSessionID, sessionID );
		goto EndOf_Process;
 	}

EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_S_S_RESP;
	p.PackHeader();


	s<<RetCode;
	if( 0 == RetCode )
	{//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
		s<<sessionID;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF1( "S_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );


	//---------------------------------S_C ֪ͨ-----------------------------------------
	//�ɹ��������͹㲥
	if( 0 == RetCode )
	{
	}

}




