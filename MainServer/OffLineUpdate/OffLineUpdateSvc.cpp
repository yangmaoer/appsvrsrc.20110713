#include "OffLineUpdateSvc.h"
#include "DBOperate.h"
#include "OffLineUpdateSvc.h"
#include "Role.h"
#include "CoreData.h"
#include "../Bag/BagSvc.h"
#include "DateTime.h"
#include <math.h>
#include <time.h>



OffLineUpdateSvc::OffLineUpdateSvc(void *service, ConnectionPool * cp)
{
	_mainSvc = (MainSvc*)(service);
	_cp = cp;

}

OffLineUpdateSvc::~OffLineUpdateSvc()
{
	//
}

//���ݰ���Ϣ
void OffLineUpdateSvc::OnProcessPacket(Session& session,Packet& packet)
{
	DEBUG_PRINTF1( "C_S req pkg-------MsgType[%d] \n", packet.MsgType );
	DEBUG_SHOWHEX( packet.GetBuffer()->GetReadPtr()-PACKET_HEADER_LENGTH, packet.GetBuffer()->GetDataSize()+PACKET_HEADER_LENGTH, 0, __FILE__, __LINE__ );

	switch(packet.MsgType)
	{
		case 1601: //[MsgType:1601]�������ݲ�ѯ
			ProcessUpdateDataQuery(session, packet);
		break;

    case 1602: //[MsgType:1602]��ʼ��������
			ProcessBeginUpdate(session,  packet);
			break;

		case 1603: //[MsgType:1603]��Ҽ��ٹһ�
	    ProcessSpeedupUpdate(session, packet);
		  break;

		case 1604: //[MsgType:1604]ֹͣ��������
	    ProcessStopUpdate(session, packet);
		  break;

		case 1605:  //[MsgType:1605] ʣ��һ���ʱ��
	    ProcessLeftUpdateHour(session, packet);
	    break;

		case 1699: //[MsgType:1699]ǰ̨�����۷�
      ProcessLostBilling(session, packet);
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
void OffLineUpdateSvc::ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode)
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

//��ɫ�Ƿ�����ô���Ǯ, 1 ������2 ��ҡ�
//���� 0�ɹ���-1 ʧ�� ,1 û��
int OffLineUpdateSvc::IfHasSoMoney(UInt32 roleID, UInt32 money,Byte moneyType)
{
  char szSql[1024];
  char szSql1[1024];
  char szTemp[50];
  char szMoney[50];
  int iRet;
	UInt32 roleMoney;
  Connection con;
	DBOperate dbo;

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	if(1== moneyType)	//����
	{
		sprintf(szSql,"select %s from RoleMoney where RoleID = %d;","Money",roleID);
		sprintf(szSql1,"select %s from RoleMoney where RoleID = %d;","BindMoney",roleID);

		iRet = dbo.QuerySQL(szSql);
		if(iRet < 0)
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"DB operate failed !");
			return -1;
		}

		roleMoney = dbo.GetIntField(0);

		iRet = dbo.QuerySQL(szSql1);
		if(iRet < 0)
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"DB operate failed !");
			return -1;
		}
		roleMoney += dbo.GetIntField(0);
	}
	if(2 == moneyType) //���
	{
		sprintf(szSql,"select %s from RoleMoney where RoleID = %d;","Gold",roleID);

		iRet = dbo.QuerySQL(szSql);

		if(iRet < 0)
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"DB operate failed !");
			return -1;
		}
		roleMoney = dbo.GetIntField(0);
	}

	if (iRet == 0) {
		if (roleMoney < money) {
			return -1;
		}
	}

	return 0;
}

//�۳���������Ǯ
//����ֵ 0 �ɹ���1 ʧ��
int OffLineUpdateSvc::TakeoffAndLockMoney(UInt32 roleID, UInt32 money,Byte moneyType,const OffLineUpdateItem& offHang)
{
	char szSql[1024];
	UInt32	RetCode = 0;
  int iRet;
  char szTemp[100];
	Connection con;
	DBOperate dbo;

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	LOG(LOG_ERROR,__FILE__,__LINE__,"money[%d]",money);

	if(1 == moneyType)
	{
		sprintf(szSql,"select %s from RoleMoney where RoleID = %d;","BindMoney",roleID);

		iRet = dbo.QuerySQL(szSql);
		if(iRet < 0)
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"DB operate failed !");
			return -1;
		}

		int bindMoney = dbo.GetIntField(0);
		int decreaseMoney = bindMoney - money;
		if(bindMoney >= money){
			  sprintf(szSql,"update RoleMoney set \
			                 BindMoney = %d where RoleID = %d;",decreaseMoney,roleID);

				iRet = dbo.ExceSQL(szSql);
				if(0 != iRet)
				{
					LOG(LOG_ERROR,__FILE__,__LINE__,"DB operate failed !");
					return 1;
				}
		}else{
			sprintf(szSql,"update RoleMoney set \
						                 BindMoney = %d where RoleID = %d;",0,roleID);
			iRet = dbo.ExceSQL(szSql);
			if(0 != iRet)
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"DB operate failed !");
				return 1;
			}

			sprintf(szSql,"update RoleMoney set \
						                 Money = Money + %d where RoleID = %d;",decreaseMoney,roleID);
			iRet = dbo.ExceSQL(szSql);
			if(0 != iRet)
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"DB operate failed !");
				return 1;
			}

		}

	}
	if(2 == moneyType)
	{
	  sprintf(szSql,"update RoleMoney set\
	                 Gold = Gold - %d where RoleID = %d;",money,roleID);
	  	//LOG(LOG_ERROR,__FILE__,__LINE__,"szSql[%s]",szSql);
		iRet = dbo.ExceSQL(szSql);
		if(0 != iRet)
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"DB operate failed !");
			return 1;
		}
	}

	//LOG(LOG_ERROR,__FILE__,__LINE__,"offHang.BilledMoney[%f]",offHang.BilledMoney);

	sprintf(szSql,"insert OffLineUpdate(RoleID,\
										BegDate,\
										IsFinish,\
	                                    MoneyType,\
	                                    LockMoney,\
	                                    BegTime,\
	                                    DesSec,\
	                                    DesExp,\
	                                    DesLevel,\
	                                    DesNum,\
	                                    BilledNum,\
	                                    BilledMoney,\
	                                    FinishedSec,\
	                                    SpeedUpSec)\
	                                    value(%d,'%s',%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%d,%d);",\
	                                        offHang.RoleID,\
	                                        offHang.BegDate.c_str(),\
	                                        offHang.IsFinish,\
	                                        offHang.MoneyType,\
	                                        offHang.LockMoney,\
	                                        offHang.BegTime,\
	                                        offHang.DesSec,\
	                                        offHang.DesExp,\
	                                        offHang.DesLevel,\
	                                        offHang.DesNum,\
	                                        offHang.BilledNum,\
	                                        offHang.BilledMoney,\
	                                        offHang.FinishedSec,\
	                                        offHang.SpeedUpSec\
	                                      	);

  iRet = dbo.ExceSQL(szSql);
  if(0 != iRet)
  {
  	LOG(LOG_ERROR,__FILE__,__LINE__,"DB operate failed !");
		return 1;
  }

  return RetCode;

}

//[MsgType:1601]�������ݲ�ѯ
void OffLineUpdateSvc::ProcessUpdateDataQuery(Session& session, Packet& packet)
{
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	UInt32 roleID= packet.RoleID;

	Byte hangType,moneyType;

	int iRet;
	UInt32 silver = 0,gold = 0;
	UInt32 speedCast = 0;
	double costSum = 0.0;
	UInt32 expSum = 0, desLevel = 0;

 	double tmpCost = 0.0;
	UInt32 tmpExp = 0,tmpLevel = 0;


	//���л���
	Serializer s(packet.GetBuffer());
  s>>hangType>>moneyType;
	if( s.GetErrorCode()!= 0)
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}


	 //LOG(LOG_ERROR,__FILE__,__LINE__,"===============UpdateDataQuery===============");
	 //LOG(LOG_ERROR,__FILE__,__LINE__,"hangType[%d]",hangType);
	 //LOG(LOG_ERROR,__FILE__,__LINE__,"moneyType[%d]",moneyType);

	//���ݵ�ǰ�ȼ�,����,�һ�����
	//	����Ԥ�ƻ�þ���, Ŀ��ȼ�,�۳���������,�ܽ��
	//	����ǽ��,��þ����мӳ�

	iRet = CalcCostAndGainsBySilver( roleID, hangType, costSum, expSum,desLevel);
	if(iRet)
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"CalcCostAndGainsBySilver error !" );
		goto EndOf_Process;
	}

	silver = (int)(ceil(costSum));
	gold = (int)(ceil(costSum / 300 * 2));

	LOG(LOG_ERROR,__FILE__,__LINE__,"gold[%d]",gold);

  if(moneyType == 2)
  {
     expSum += expSum / 10;
  }

	//����10���ӵĿ۷�
	iRet = CalcCostAndGainsBySilver( roleID, 1, tmpCost, tmpExp,tmpLevel);
	if(iRet)
	{
	  RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"CalcCostAndGainsBySilver error !" );
		goto EndOf_Process;
	}

	//speedCast = (int)(ceil(tmpCost / 300 * 5 /3));
		speedCast = (int)ceil(tmpCost / 300 * 2 * 5 /6);

	//LOG(LOG_ERROR,__FILE__,__LINE__,"expSum[%d]",expSum);
	//LOG(LOG_ERROR,__FILE__,__LINE__,"desLevel[%d]",desLevel);
	LOG(LOG_ERROR,__FILE__,__LINE__,"speedCast[%d]",speedCast);

EndOf_Process:
	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();
	p.CopyHeader(packet);
	p.Direction = DIRECT_C_S_RESP;
	p.PackHeader();

	LOG(LOG_ERROR,__FILE__,__LINE__,"UpdateQuery: RetCode[%d]",RetCode);
	s<<RetCode;
	if( 0 == RetCode )
	{
		//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
		s<<expSum;
		s<<desLevel;
		s<<silver;
		s<<gold;
		s<<speedCast;
	}
	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

	DEBUG_PRINTF( "ack pkg=======, \n" );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );
}

//[MsgType:1602]��ʼ��������
void OffLineUpdateSvc::ProcessBeginUpdate(Session& session, Packet& packet)
{
	char szSql[1024];
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	UInt32 roleID= packet.RoleID, Money = 0,totalTime;
	int iRet;
	DateTime nowTime,tmpDate;
	string strDate;
	Byte hangType, moneyType;

	OffLineUpdateItem offupdate;
	UInt32 curLevel;
	double costSum = 0.0;
	UInt32 expSum,desLevel,cost = 0;
	double billedMoney = 0.0;
	UInt32 tmpExp, tmpLevel,exp;
	double tmpCost;
	string strTime;
	Byte isFinish = 0,offlineState = 0;

	Connection con;
	DBOperate dbo;


	//���л���
	Serializer s(packet.GetBuffer());
    s>>hangType>>moneyType;

	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr( roleID);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"hangtype[%d]---moneytype[%d]",hangType,moneyType);
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}


	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//�жϼ���
	//	10�����²�����һ�
	if(0 == pRole->ID())
	{
	 	RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole error! roleID[%d]", roleID );
		goto EndOf_Process;
	}

	if(pRole->Level() <= 10)
	{
	  RetCode = ERR_ROLE_LEVELNOTENOUGH;
		LOG(LOG_ERROR,__FILE__,__LINE__," curLevel <= 10 !" );
		goto EndOf_Process;
	}


	strTime = nowTime.Now().StringDate();
    sprintf(szSql,"select IsFinish from OffLineUpdate \
  	               where RoleID = %d and BegDate = '%s';",roleID,strTime.c_str());
    iRet = dbo.QuerySQL(szSql);
    if(iRet == 0)
    {
	  while(dbo.HasRowData())
	  {
		isFinish = dbo.GetIntField(0);
		if(isFinish == 1)
		{
			offlineState = 0;
		}
		else
		{
			offlineState = 1;
		}
		dbo.NextRow();
	  }
    }
    else if(iRet == 1)
    {
  	   offlineState = 0;
    }
    else
    {
	   RetCode = ERR_APP_OP;
  	   LOG(LOG_ERROR,__FILE__,__LINE__,"DB op error !" );
  	   goto EndOf_Process;
    }

    LOG(LOG_ERROR,__FILE__,__LINE__,"offlineState[%d]",offlineState);
    if(offlineState == 0)
    {
		pRole->IsOfflineUpdate(0);
    }


	//��ʼ�һ��Ŀ����Ƿ���
	if(pRole->IsOfflineUpdate())
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"role.IsOfflineUpdate() is true ");
		goto EndOf_Process;
	}

  //������������ʱ���Ƿ񳬹�24Сʱ
	strDate = tmpDate.Now().StringDate();

	sprintf(szSql,"select ifnull( sum(FinishedSec) ,0),ifnull(sum(SpeedUpSec),0)\
								 from OffLineUpdate \
	               where RoleID = %d and BegDate = '%s';",roleID ,strDate.c_str());
	iRet = dbo.QuerySQL(szSql);
	if(iRet < 0)
	{
	  RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found or  errro! ,szSql[%s] " , szSql);
		goto EndOf_Process;
	}
	if(1 == iRet)
	{
	  totalTime = 0;
	}

	totalTime = dbo.GetIntField(0) + dbo.GetIntField(1);
	//LOG(LOG_ERROR,__FILE__,__LINE__,"totalTime[%d]",totalTime);

	//�ж�ʣ��ʱ���Ƿ���ڹһ�ʱ��
	totalTime += hangType * 3600;

	if(totalTime > 3600 * 24)
	{
	 	RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"totalTime > 24h");
		goto EndOf_Process;
	}

  //�鿴�Ƿ����������ڵ�������¼,�������ɾ��
  strTime = nowTime.Now().StringDate();
  //LOG(LOG_ERROR,__FILE__,__LINE__,"strTime[%s]" , strTime.c_str());
	sprintf(szSql,"delete from OffLineUpdate \
	       					where RoleID = %d \
	       					and BegDate <> '%s' ;",roleID,strTime.c_str());
	iRet = dbo.ExceSQL(szSql);
	if(iRet)
	{
		RetCode = ERR_SYSTEM_DBERROR;
    LOG(LOG_ERROR,__FILE__,__LINE__,"DB OP Failed !");
		goto EndOf_Process;
	}

  //��ȡĿ�꼶�������顢�۷�
  iRet = CalcCostAndGainsBySilver( roleID, hangType, costSum, expSum,desLevel);
  if(iRet)
  {
		RetCode = ERR_APP_OP;
    LOG(LOG_ERROR,__FILE__,__LINE__,"CalcCostAndGainsBySilver Failed !");
		goto EndOf_Process;
  }

  //�ӳɼ���������
  if(1 == moneyType)
  {
		cost = (int)ceil(costSum);			//��������
  }
  else if(2 == moneyType)
  {
		expSum = expSum + expSum / 10;  //��ҹһ������мӳ�
		cost = (int)ceil(costSum / 300 * 2 );		//��������
  }
  else
  {
  	RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"moneyType error!moneyType[%d]! ", moneyType );
		goto EndOf_Process;
  }

   LOG(LOG_ERROR,__FILE__,__LINE__,"cost[%d]",cost);
  //��Ǯ�Ƿ��㹻
	iRet = IfHasSoMoney(roleID, cost, moneyType);
	if(iRet)
	{
		RetCode = ERR_ROLE_NOMONEY;
		LOG(LOG_ERROR,__FILE__,__LINE__,"You have no so much money[%d]",cost);
		goto EndOf_Process;
	}

	//�۳���������Ǯ
	offupdate.RoleID = roleID;
  offupdate.BegDate = strTime;
  offupdate.IsFinish = 0;
  offupdate.MoneyType = moneyType;
  offupdate.LockMoney = cost;
  offupdate.BegTime = time(NULL);
  offupdate.DesSec = hangType * 3600;
  offupdate.DesExp = expSum;
  offupdate.DesLevel = desLevel;
  offupdate.DesNum = hangType * 60;
  offupdate.BilledNum = 0;
  offupdate.BilledMoney = billedMoney;
  offupdate.FinishedSec = 0;
  offupdate.SpeedUpSec = 0;

	iRet = TakeoffAndLockMoney(roleID, cost,moneyType,offupdate);
	if(iRet)
	{
	  RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"TakeoffAndLockMoney failed ! ");
		goto EndOf_Process;
	}

	pRole->IsOfflineUpdate(1);

EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();
	p.CopyHeader(packet);
	p.Direction = DIRECT_C_S_RESP;
	p.PackHeader();
	LOG(LOG_ERROR,__FILE__,__LINE__,"BeginUpdate: RetCode[%d]",RetCode);
	s<<RetCode;
	if( 0 == RetCode )
	{
		//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
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
		//���� S_C ��Ǯ�䶯֪ͨ
		_mainSvc->GetBagSvc()->NotifyMoney(roleID);

	}


}

//[MsgType:1699]ǰ̨�����۷�
void OffLineUpdateSvc::ProcessLostBilling(Session& session, Packet& packet)
{
	char szSql[1024];
	UInt32	RetCode = 0,iRet;
	DataBuffer	serbuffer(1024);
	UInt32 roleID = packet.RoleID;

	double addCost = 0.0;
	Byte isOfflineUpdate = 0,moneyType  = 0;
	UInt32 addExp = 0,desLevel = 0,totalExp = 0;
	UInt32 updateID = 0,desNum = 0,billedNum = 0;
	Connection con;
	DBOperate dbo;
	Byte isFinish = 0,vipLevl = 0;
	UInt32 finalExp = 0;

	UInt32 startLevel = 0,lastLevel = 0,speedupSec = 0,speedMinute = 0;

	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);

	//���л���
	Serializer s(packet.GetBuffer());
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//LOG(LOG_ERROR,__FILE__,__LINE__,"===============LostBilling===============");

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());


	//�ж��Ƿ������߹һ�
	if(0 == pRole->ID())
	{
	    RetCode = ERR_APP_OP;
      LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole[%d] error !" , roleID);
			goto EndOf_Process;
	}

	if(!pRole->IsOfflineUpdate())
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"role.IsOfflineUpdate() is false! roleID[%d] ", roleID );
		goto EndOf_Process;
	}



	totalExp = pRole->Exp();
	startLevel = pRole->Level();
	vipLevl = pRole->VIP();

	//�ҵ��һ���¼
	//	�ж��Ƿ񳬹�Ŀ��۷Ѵ���
	sprintf(szSql,"select UpdateID,DesNum,BilledNum,MoneyType,SpeedUpSec from OffLineUpdate\
								where RoleID = %d \
									and IsFinish = 0 ",roleID);
	iRet = dbo.QuerySQL(szSql);
	if(iRet != 0)
	{
	  RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found or  errro! ,szSql[%s] " , szSql);
		goto EndOf_Process;
	}
	updateID = dbo.GetIntField(0);
	desNum = dbo.GetIntField(1);
	billedNum = dbo.GetIntField(2);
	moneyType = dbo.GetIntField(3);
	speedupSec = dbo.GetIntField(4);

	speedMinute = speedupSec / 60;
	if(billedNum >= (desNum - speedMinute))
	{
		RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"data error!! billedNum[%d], leftNum,[%d], roleID[%d] ", billedNum, desNum - speedMinute, roleID );
		goto EndOf_Process;
	}

	//����ôο۷�
	iRet = CalcCostPerMinite(roleID, pRole->Level(), pRole->Exp(), addCost,addExp, desLevel, finalExp,vipLevl);
	if(iRet)
	{
	  RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"CalcCostPerMinite failed ! ");
		goto EndOf_Process;
	}

	//�����Ǯ
	if(moneyType == 2)
	{
	   addCost = addCost / 300;
	}
	totalExp += addExp;

	//�޸����ݿ��¼����ֶ�
	if( (billedNum+1) >= desNum - speedMinute)
	{
	   isFinish = 1;  				//�һ�����
	   isOfflineUpdate = 0;		//�һ�������Ϊ��
	}

	else
	{
	   isFinish = 0;
	   isOfflineUpdate = 1;		//�һ�������Ϊ��
	}


	sprintf(szSql,"update OffLineUpdate \
								set \
									BilledNum = BilledNum + 1, \
									BilledMoney = BilledMoney + %f,\
									FinishedSec = FinishedSec + 60 ,\
									IsFinish = %d \
								where UpdateID = %d \
									and IsFinish = 0;",
								addCost, isFinish, updateID );
	iRet = dbo.ExceSQL(szSql);
	if(iRet != 0)
	{
	  RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"DB OP error ! ");
		goto EndOf_Process;
	}

	//���½�ɫ�������߹һ�״̬
  if(isOfflineUpdate == 0)
  {
    pRole->IsOfflineUpdate(0);
  }

  //LOG(LOG_ERROR,__FILE__,__LINE__,"startLevel[%d]",startLevel);

	//������ɫ���Ա仯
	iRet = _mainSvc->GetCoreData()->RoleExpAdd(roleID, addExp);
	if(iRet < 0)
	{
	  RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"RoleExpAdd [%d] error !",roleID);
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


	LOG(LOG_ERROR,__FILE__,__LINE__,"LostBilling: RetCode [%d]",RetCode);
	/*LOG(LOG_ERROR,__FILE__,__LINE__,"addExp [%d]",addExp);
	LOG(LOG_ERROR,__FILE__,__LINE__,"totalExp [%d]",totalExp);*/

	s<<RetCode;
	if( 0 == RetCode )
	{
		//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
		s<<addExp;
	  s<<totalExp;
	}
	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

	DEBUG_PRINTF( "ack pkg=======, \n" );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );

	//����֮����ٹһ��Ŀ۷ѷ����仯
	if(0 == RetCode)
	{
	 	 RolePtr tmpRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
	 	 if(tmpRole->ID() == 0)
	 	 {
	 	    LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole error! roleID[%d]", roleID );
	 	    return ;
	 	 }
	   lastLevel = tmpRole->Level();
	   if(startLevel < lastLevel)
	   {
	   			UInt32 castTenMinute = 0;
	   			iRet = SpeedTenMiniteCast(roleID,castTenMinute);
	   			if(iRet)
	   			{
	   			  LOG(LOG_ERROR,__FILE__,__LINE__,"SpeedTenMiniteCast failed !" );
	   			  return ;
	   			}
	        NotifySpeedUpCast(roleID,castTenMinute);

	   }

	}

}

//[MsgType:1603]��Ҽ��ٹһ�
void OffLineUpdateSvc::ProcessSpeedupUpdate(Session& session, Packet& packet)
{

	char szSql[1024];
	UInt32	RetCode = 0, iRet;
	DataBuffer	serbuffer(1024);
	UInt32 roleID= packet.RoleID;
	Byte speedupType,isOfflineUpdate = 0;

	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
	UInt32 exp = 0,cost = 0,level = 0,speedupSec = 0,billNum = 0;
	UInt32 finishedSec = 0,isFinish = 0,addFinishSec = 0,speedSec = 0;
	UInt32 updateID = 0,desNum = 0 ;
	UInt32 totalTime = 0,desSec = 0,speedCast = 0;
	double tmpCost = 0.0;

	UInt32 startLevel = 0,lastLevel = 0;
	Connection con;
	DBOperate dbo;

	//���л���
	Serializer s(packet.GetBuffer());
	s>>speedupType;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	// LOG(LOG_ERROR,__FILE__,__LINE__,"speedupType[%d]" , speedupType);

   //��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//�ж��Ƿ��ڹһ���

	if(0 == pRole->ID())
	{
	    RetCode = ERR_APP_OP;
      LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole[%d] error !" , roleID);
			goto EndOf_Process;
	}

	if(!pRole->IsOfflineUpdate())
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"role.IsOfflineUpdate() is false ");
		goto EndOf_Process;
	}

	 startLevel = pRole->Level();

	//���������Ҫ�Ľ�Һͻ�õľ��顢���ٺ��ܵľ���
	iRet = CalcCostAndGainsBySilver(roleID, 1,tmpCost,exp, level);
	if(iRet)
	{
	  RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"CalcCostAndGainsBySilver failed !" );
		goto EndOf_Process;
	}


	speedCast = (int)ceil(tmpCost / 300 * 2 * 5 /6);
  if(1 == speedupType)
  {//����10���ӵľ��顢��ҡ��۷Ѵ���
    exp = (exp + exp / 10) /6;
    //tmpCost = tmpCost / 300 * 2 * 5 /6;
    speedupSec = 10 * 60;
  }
  if(2 == speedupType)
  {//����60���ӵľ��顢��ҡ��۷Ѵ���
    exp = exp + exp / 10;
    //tmpCost = tmpCost / 300 * 2 * 5;
    speedupSec = 60 * 60;
    speedCast = speedCast * 6;
  }

  //LOG(LOG_ERROR,__FILE__,__LINE__,"speedupType[%d]--speedCast[%d]" ,speedupType,speedCast);

  //����Ƿ����㹻�Ľ�ң��������۳����
  sprintf(szSql,"select Gold from RoleMoney where RoleID = %d;",roleID);
  iRet = dbo.QuerySQL(szSql);
  if(iRet != 0)
  {
     RetCode = ERR_SYSTEM_DBERROR;
     LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found or  errro! ,szSql[%s] " , szSql);
		 goto EndOf_Process;
  }

  cost = speedCast;
  if(cost > dbo.GetIntField(0))
  {
     RetCode = ERR_APP_DATA;
     LOG(LOG_ERROR,__FILE__,__LINE__,"Role[%d] hasn't so much gold !" ,roleID);
		 goto EndOf_Process;
  }

	//�ҵ�״̬Ϊδ��ɵĹһ���¼
	sprintf(szSql,"select UpdateID, FinishedSec, DesSec,SpeedUpSec \
								from OffLineUpdate\
								where RoleID = %d \
									and IsFinish = 0 ",roleID);
	iRet = dbo.QuerySQL(szSql);
	if(iRet)
	{
	  RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"DB OP error");
		goto EndOf_Process;
	}
	updateID = dbo.GetIntField(0);
	totalTime = dbo.GetIntField(1);
	desSec = dbo.GetIntField(2);
	speedSec = dbo.GetIntField(3);
	addFinishSec = totalTime;
	totalTime += speedupSec + speedSec;

	//�۳����ٹһ��Ľ��
	sprintf(szSql,"update RoleMoney set \
								Gold = Gold - %d where RoleID = %d;",cost,roleID);
	iRet = dbo.ExceSQL(szSql);
	if(iRet != 0)
	{
	   RetCode = ERR_SYSTEM_DBERROR;
     LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found or  errro! ,szSql[%s] " , szSql);
		 goto EndOf_Process;
	}

	//����ôμ���,�Ƿ�ᵼ�¸ôιһ�����
	if(totalTime < desSec)
	{
	  isFinish = 0;
	  isOfflineUpdate = 1;
	  addFinishSec = speedupSec;

	}
	else
	{
	  isFinish = 1;    //ֹͣ���߹һ�
	  isOfflineUpdate = 0;   //���߹һ�������Ϊ��
	  addFinishSec = desSec - addFinishSec;
	}

		//LOG(LOG_ERROR,__FILE__,__LINE__,"isOfflineUpdate[%d] " , isOfflineUpdate);
	//�޸����߹һ�����������������۷Ѵ������ѿ۳���Ǯ���Լ��ٵ�����
  sprintf(szSql,"update OffLineUpdate set \
  																		SpeedUpSec = SpeedUpSec + %d, \
  																		IsFinish = %d \
  																		where UpdateID = %d ",
  																		speedupSec, isFinish, updateID );
  iRet = dbo.ExceSQL(szSql);
  if(iRet != 0)
  {
      RetCode = ERR_SYSTEM_DBERROR;
      LOG(LOG_ERROR,__FILE__,__LINE__,"UpdateRoleAttr RolID[%d] Failed ! " , roleID);
			goto EndOf_Process;
  }

  //���½�ɫ�������߹һ�״̬
  if(isOfflineUpdate == 0)
  {
    pRole->IsOfflineUpdate(0);
  }

	//���½�ɫ����
	iRet = _mainSvc->GetCoreData()->RoleExpAdd(roleID, exp);
	if(iRet < 0)
	{
	  	RetCode = ERR_APP_OP;
      LOG(LOG_ERROR,__FILE__,__LINE__,"UpdateRoleAttr RolID[%d] Failed ! " , roleID);
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
	LOG(LOG_ERROR,__FILE__,__LINE__,"SpeedUpUpdate: RetCode[%d]",RetCode);

	/*LOG(LOG_ERROR,__FILE__,__LINE__,"exp[%d]" , exp);
	LOG(LOG_ERROR,__FILE__,__LINE__,"role.Exp()+exp[%d]" , role.Exp()+exp);*/
	s<<RetCode;
	if( 0 == RetCode )
	{
		//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
		s<<exp;
		s<<(pRole->Exp()+exp);
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
		//���� S_C ��Ǯ�䶯֪ͨ
		_mainSvc->GetBagSvc()->NotifyMoney(roleID);
	}

	//����֮����ٹһ��Ŀ۷ѷ����仯
	if(0 == RetCode)
	{
	    RolePtr tmpRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
	 	 if(tmpRole->ID() == 0)
	 	 {
	 	    LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole error! roleID[%d]", roleID );
	 	    return ;
	 	 }
	   lastLevel = tmpRole->Level();
	   if(startLevel < lastLevel)
	   {
	   			UInt32 castTenMinute = 0;
	   			iRet = SpeedTenMiniteCast(roleID,castTenMinute);
	   			if(iRet)
	   			{
	   			  LOG(LOG_ERROR,__FILE__,__LINE__,"SpeedTenMiniteCast failed !" );
	   			  return ;
	   			}
	   			//LOG(LOG_ERROR,__FILE__,__LINE__,"SpeedTenMiniteCast [%d]",castTenMinute );
	        NotifySpeedUpCast(roleID,castTenMinute);

	   }
	}

}

//[MsgType:1604]ֹͣ��������
void OffLineUpdateSvc::ProcessStopUpdate(Session& session, Packet& packet)
{
	char szSql[1024];
	DataBuffer	serbuffer(1024);
	UInt32 Money,roleID = packet.RoleID;
	int iRet,RetCode = 0;


	char strTmp[1024];
	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
	UInt32 speedupSec = 0;
	UInt32 updateID,desNum ,billedNum,begTime,finishSec,lockMoney,desSec,moneyType;
	UInt32 addSec = 0,addBillNum = 0,addExp = 0,retMoney = 0;
	Int32 elapse = 0;
	double addCost = 0.0,billedMoney = 0.0, tmpCost = 0.0;
	UInt32 tmpExp = 0;
	UInt32 tmpLevel = 0;
	UInt32 currLevel = 0;
	UInt32 currExp = 0;
	UInt32 finalExp = 0;
	Int32	sec2Bill = 0;				//��Ҫ����۷ѵ�ʱ��		��λ ��
	UInt32 startLevel = 0,lastLevel = 0;
	Byte vipLevl = 0;

	Connection con;
	DBOperate dbo;

	//���л���
	Serializer s(packet.GetBuffer());
	if( s.GetErrorCode()!= 0 )
	{
	 RetCode = ERR_SYSTEM_SERERROR;
	 LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
	 goto EndOf_Process;
	}


	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//�ж��Ƿ������߹һ�
	if(0 == pRole->ID())
	{
	    RetCode = ERR_APP_OP;
      LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole[%d] error !" , roleID);
			goto EndOf_Process;
	}

	if(!pRole->IsOfflineUpdate())
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"pRole->IsOfflineUpdate() is false ");
		goto EndOf_Process;
	}

	//�ҵ��һ���¼
	sprintf(szSql,"select UpdateID,\
												DesNum,\
												BilledNum,\
												BegTime,\
												FinishedSec,\
												BilledMoney,\
												LockMoney,\
												DesSec,\
												MoneyType,\
												SpeedUpSec\
								from OffLineUpdate\
								where RoleID = %d \
									    and IsFinish = 0;",roleID);
	iRet = dbo.QuerySQL(szSql);
	if(iRet != 0)
	{
	  RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found or  errro! ,szSql[%s] " , szSql);
		goto EndOf_Process;
	}

	updateID = dbo.GetIntField(0);
	desNum = dbo.GetIntField(1);
	billedNum = dbo.GetIntField(2);
	begTime = dbo.GetIntField(3);
	finishSec = dbo.GetIntField(4);
	billedMoney = dbo.GetFloatField(5);
	lockMoney = dbo.GetIntField(6);
	desSec = dbo.GetIntField(7);
	moneyType = dbo.GetIntField(8);
	speedupSec = dbo.GetIntField(9);

	/*LOG(LOG_ERROR,__FILE__,__LINE__,"roleID[%d] " ,roleID);
	LOG(LOG_ERROR,__FILE__,__LINE__,"lockMoney[%d] " , lockMoney);
	LOG(LOG_ERROR,__FILE__,__LINE__,"billedMoney[%f] " , billedMoney);*/

	//���㲹��Ŀ۷Ѵ���,���۷ѽ��,������������,��������ӵľ���
  elapse = time(NULL) - begTime;
  if(elapse<0)
  {
		RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"elapse errro![%d] ", elapse);
		goto EndOf_Process;
	}

  //����ʱ���У׼
	if(elapse > (desSec - speedupSec))
	  elapse = desSec - speedupSec;
	if(elapse < finishSec)
		elapse = finishSec;

	//����ʱ���У��
	if(elapse<0)
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"elapse  errro! elapse[%d]", elapse );
		goto EndOf_Process;
	}

	//���㲹��� �۷Ѵ���
	sec2Bill = elapse - finishSec;
  addBillNum = sec2Bill / 60;
  if(sec2Bill % 60)
     addBillNum++;   //����Ŀ۷Ѵ���

  //����ľ���,�۳��Ľ�Ǯ
  currLevel = pRole->Level();
	currExp = pRole->Exp();
	vipLevl = pRole->VIP();
	startLevel = currLevel;

  for(int i = 0; i < addBillNum; i++)
  {
      iRet = CalcCostPerMinite(roleID, currLevel, currExp, tmpCost, tmpExp, tmpLevel, finalExp,vipLevl);
      if(iRet)
      {
        RetCode = ERR_APP_OP;
      	LOG(LOG_ERROR,__FILE__,__LINE__,"CalcCostPerMinite  errro! " );
				goto EndOf_Process;
      }
			addCost += tmpCost;
			addExp += tmpExp;
			currLevel = tmpLevel;
			currExp = finalExp;
  }


  if(2 == moneyType)
  {
  	if(0 == addBillNum)
		{
		  addCost = 0.0;
		}
		else
		{
			if(addBillNum <= 10)
			  addCost = addCost / addBillNum * 30 /300;
			else
			  addCost = addCost / addBillNum * 60 /300;

		}

     addExp = (int)(addExp*1.1);
     sprintf(strTmp,"Gold = Gold +"); //���ؽ��
  }
  else
  {
     sprintf(strTmp,"BindMoney = BindMoney +"); //��������
  }


	//�������߹һ���

	sprintf(szSql,"update OffLineUpdate set\
	               FinishedSec = FinishedSec + %d,\
	               BilledMoney = BilledMoney + %f,\
	               BilledNum = BilledNum + %d,\
	               IsFinish = 1 \
	               where UpdateID = %d;",
	               elapse,addCost,addBillNum,updateID);
	iRet = dbo.ExceSQL(szSql);
	LOG(LOG_ERROR,__FILE__,__LINE__,"iRet[%d] " , iRet);
	if(iRet != 0)
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found or  errro! ,szSql[%s] " , szSql);
		goto EndOf_Process;
	}

	//�˻ؽ�Ǯ

	retMoney = (UInt32)(lockMoney - billedMoney - addCost);
	//LOG(LOG_ERROR,__FILE__,__LINE__,"retMoney[%d] " , retMoney);

	if(0 != retMoney)
	{
		  sprintf(szSql,"update RoleMoney set %s %d\
										where RoleID = %d;",strTmp,retMoney,roleID);
			//LOG(LOG_ERROR,__FILE__,__LINE__,"[%s] " , szSql);

			iRet = dbo.ExceSQL(szSql);
			if(iRet != 0)
			{
			    RetCode = ERR_SYSTEM_DBERROR;
		      LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found or  errro! ,szSql[%s] " , szSql);
					goto EndOf_Process;
			}
	}


	 //�һ����عر�
	pRole->IsOfflineUpdate(0);

	sprintf(szSql,"update Role set IsOfflineUpdate = 0 where RoleID = %d;",roleID);
	iRet = dbo.ExceSQL(szSql);
	if(iRet != 0)
	{
		RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"Excute SQL failed ! szSql[%d]", szSql);
		goto EndOf_Process;
	}

	//�����������ӵľ��鲻Ϊ 0   ,���ӽ�ɫ����
	if(addExp)
	{
	    iRet = _mainSvc->GetCoreData()->RoleExpAdd(roleID, addExp);
			if(iRet < 0)
			{
			  	RetCode = ERR_APP_OP;
		      LOG(LOG_ERROR,__FILE__,__LINE__,"UpdateRoleAttr RolID[%d] Failed ! " , roleID);
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

	LOG(LOG_ERROR,__FILE__,__LINE__,"StopUpdate: RetCode[%d]" , RetCode);
	s<<RetCode;
	if( 0 == RetCode )
	{
		//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
	}
	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

	DEBUG_PRINTF( "ack pkg=======, \n" );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );

 	//��Ǯ֪ͨ
	if(0 == RetCode)
	{
		_mainSvc->GetBagSvc()->NotifyMoney(roleID);
	}

	//����֮����ٹһ��Ŀ۷ѷ����仯
	if(0 == RetCode)
	{
	   RolePtr tmpRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
	 	 if(tmpRole->ID() == 0)
	 	 {
	 	    LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole error! roleID[%d]", roleID );
	 	    return ;
	 	 }
	   lastLevel = tmpRole->Level();
	   if(startLevel < lastLevel)
	   {
	   			UInt32 castTenMinute = 0;
	   			iRet = SpeedTenMiniteCast(roleID,castTenMinute);
	   			if(iRet)
	   			{
	   			  LOG(LOG_ERROR,__FILE__,__LINE__,"SpeedTenMiniteCast failed !" );
	   			  return ;
	   			}
	        NotifySpeedUpCast(roleID,castTenMinute);

	   }
	}

}


//[MsgType:1605] ʣ��һ���ʱ��
void OffLineUpdateSvc::ProcessLeftUpdateHour(Session& session, Packet& packet)
{

	char szSql[1024];
	DataBuffer	serbuffer(1024);
	UInt32 roleID = packet.RoleID;
	int iRet,RetCode = 0;
	string strDate;
	DateTime tmpDate;
	UInt32 totalSec = 0;
	Int32 leftUpdateHour = 0;


	Connection con;
	DBOperate dbo;

	//���л���
	Serializer s(packet.GetBuffer());
	if( s.GetErrorCode()!= 0 )
	{
	 RetCode = ERR_SYSTEM_SERERROR;
	 LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
	 goto EndOf_Process;
	}

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

  //��ȡ���ڵ�ʱ��
  strDate = tmpDate.Now().StringDate();
  sprintf(szSql,"select sum(FinishedSec),sum(SpeedUpSec) from OffLineUpdate \
  								where RoleID = %d and BegDate = '%s';",roleID, strDate.c_str());
  iRet = dbo.QuerySQL(szSql);
  if(iRet < 0)
  {
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"DB op error !");
		goto EndOf_Process;
  }
  else if(iRet == 1)  				//����û�йһ���¼
  {
    leftUpdateHour = 24;
  }
  else											 //�����йһ���¼
  {
       totalSec += dbo.GetIntField(0) + dbo.GetIntField(1);
       leftUpdateHour = (24 * 3600 - totalSec) / 3600;
       if(leftUpdateHour < 0)
       {
         	RetCode = ERR_APP_DATA;
					LOG(LOG_ERROR,__FILE__,__LINE__,"DB op error !");
					goto EndOf_Process;
       }

       // LOG(LOG_ERROR,__FILE__,__LINE__,"tmpHour[%d]" , tmpHour);
       // LOG(LOG_ERROR,__FILE__,__LINE__,"totalSec[%d]" , totalSec);
  }

  LOG(LOG_DEBUG,__FILE__,__LINE__,"leftUpdateHour[%d]" , leftUpdateHour);

  EndOf_Process:
	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();
	p.CopyHeader(packet);
	p.Direction = DIRECT_C_S_RESP;
	p.PackHeader();

	//LOG(LOG_ERROR,__FILE__,__LINE__,"RetCode[%d]" , RetCode);
	//LOG(LOG_ERROR,__FILE__,__LINE__,"leftUpdateHour[%d]" , leftUpdateHour);
	s<<RetCode;
	if( 0 == RetCode )
	{
		//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
		s<<leftUpdateHour;
	}
	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

	DEBUG_PRINTF( "ack pkg=======, \n" );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );

}



//@brief ����ÿ���ӻ�þ���, Ŀ��ȼ�, ����
//@param	inputLevel	����Ľ�ɫ��ǰ�ȼ�
//@param	inputExp		����Ľ�ɫ��ǰ����
//@param	retCost		��Ҫ�Ļ���
//@param	retAddExp		���ӵľ���
//@param	retFinalLevel		���ռ���,	�ôμ���֮���ɫӦ�������ĵȼ�
//@param	retFinalExp			���վ���, �ôμ���֮���ɫӦ�������ľ���, �� retFinalLevel ��Ӧ
//@return	0 �ɹ�  ��0 ʧ��
int OffLineUpdateSvc::CalcCostPerMinite(UInt32 RoleID, UInt32 inputLevel, UInt32 inputExp, double& retCost, UInt32& retAddExp, UInt32& retFinalLevel, UInt32& retFinalExp,Byte vipLevl)
{
  char szSql[1024];
  UInt32 iRet = 0;
  int iflag = 0;
  UInt32 speedupGoldCast = 0;
	Connection con;
	DBOperate dbo;

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//�ü���һ�ÿ���ӻ�õľ���
	sprintf(szSql,"select ExpPerMinute from ExpDescOfflineUpdate \
									where Level = %d;",inputLevel);
	iRet = dbo.QuerySQL(szSql);
	if(iRet != 0)
	{
	   LOG(LOG_ERROR,__FILE__,__LINE__,"[%s]" ,szSql);
		 return -1;
	}

	retAddExp= dbo.GetIntField(0);
	if(vipLevl >= 1 && vipLevl <= 6)
	{
		VipExtraExp(RoleID,vipLevl,retAddExp);
	}
	retCost = (double)(inputLevel * inputLevel) / 1000 * 5;

	//������
  retFinalExp = inputExp+retAddExp;						//�ô��������ۻ�����
	UInt32 currLevl = inputLevel;
	UInt32 maxExp = 0;

	while(1)
	{
		sprintf(szSql,"select MaxExp \
										from LevelDesc \
										where Level = %d;", currLevl);
		iRet = dbo.QuerySQL(szSql);

		if(1==iRet)
		{
		   LOG(LOG_DEBUG,__FILE__,__LINE__,"DB data not found !" );
			 return -1;
		}
		if(iRet<0)
		{
		   LOG(LOG_ERROR,__FILE__,__LINE__,"DB error !" );
			 return -1;
		}
		maxExp = dbo.GetIntField(0);
//	  LOG(LOG_DEBUG,__FILE__,__LINE__,"maxExp[%d]", maxExp );

		if(retFinalExp<maxExp)
			break;

		retFinalExp -= maxExp;
		++currLevl;

	}

  retFinalLevel = currLevl;

	return 0;

}


//@brief �����þ���, Ŀ��ȼ�, ����
//@return	0 �ɹ�  ��0 ʧ��
int OffLineUpdateSvc::CalcCostAndGainsBySilver( UInt32 RoleID, Byte hangType, double& costSum, UInt32& expSum, UInt32& desLevel)
{
	UInt32 iMinite = 0;
	int iRet = 0;

  //�����������õķ�����
	iMinite  = 60 *hangType;

	//��ȡ��ǰ�ȼ��;���
	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(RoleID);
	if(0 == pRole->ID())
	{
	   LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole[%d] failed !" ,RoleID);
		 return -1;
	}

  UInt32 begLevel = pRole->Level();
	UInt32 begExp = pRole->Exp();
	Byte vipLevl = pRole->VIP();

	//�����Ӽ��㻨�Ѽ�����
	costSum = 0;		//�ۻ�����
	expSum = 0;			//�ۻ�����
	UInt32 currLevel = begLevel;
	UInt32 currExp = begExp;
	UInt32 finalExp = 0;
	for( int i = 0; i < iMinite; i++ )
	{
		double tmpCost = 0.0;
		UInt32 tmpExp = 0;
	  iRet = CalcCostPerMinite( RoleID,currLevel, currExp, tmpCost, tmpExp, desLevel, finalExp,vipLevl);
	  if(iRet)
	  {
	     LOG(LOG_ERROR,__FILE__,__LINE__,"CalcCostPerMiniteByCache error !" );
			 return -1;
	  }

		expSum += tmpExp;
		costSum += tmpCost;
		currLevel = desLevel;
		currExp = finalExp;
	}

	return 0;
}

//@brief	S_C [MsgType:1601]�һ�����
void OffLineUpdateSvc::OnRoleLoginNotify(UInt32 roleID)
{
	 UInt32 desExp = 0;
	 UInt32 desLevel = 0;
	 UInt32 finishSec = 0;
	 UInt32 desSec = 0;
	 int iRet ;

    char szSql[1024];
	Connection con;
	DBOperate dbo;
	string strTime;
	DateTime nowTime;
	Byte isFinish = 0;
	Byte offlineState = 0;

	Byte isOfflineUpdate = 0;

	 //��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());



  //��ȡ��ɫ��Ϣ
  //	��ʱ��ɫ��δ�����ͼ,���滹û������
  sprintf(szSql,"select IsOfflineUpdate \
  							from Role \
  							where RoleID = %d;",roleID);
  iRet = dbo.QuerySQL(szSql);
  if(iRet)
  {
  	LOG(LOG_ERROR,__FILE__,__LINE__,"DB op error!!" );
  	return;
  }

  isOfflineUpdate = dbo.GetIntField(0);

	//LOG(LOG_ERROR,__FILE__,__LINE__,"isOfflineUpdate[%d]",isOfflineUpdate);
  //��ȡ�һ������Ƿ���
  //	û�йһ�,�����κδ���
  if(0 == isOfflineUpdate)
  {
     return ;
  }


  LOG(LOG_ERROR,__FILE__,__LINE__,"isOfflineUpdate[%d]",isOfflineUpdate);

  iRet = AddBill(roleID,desExp, desLevel, finishSec, desSec );
  if(iRet)
  {
     LOG(LOG_ERROR,__FILE__,__LINE__,"AddBill error!!" );
  	 return ;
  }

	NotifyStopUpdate(roleID,desExp, desLevel,finishSec,desSec);

}

//@brief	��ɫ���� ����۷�
//				������ɫδ�����ͼǰ���������
//@return 0 �ɹ�  ��0 ʧ��
int OffLineUpdateSvc::AddBill( UInt32 roleID, UInt32& desExp, UInt32& desLevel, UInt32& finishSec, UInt32& desSec )
{
	char szSql[1024];
	int iRet = 0,isFinish = 0;
	UInt32 elapse = 0,begTime = 0,billNum = 0,billedNum = 0,addFinishSec = 0;
	UInt32 speedupSec = 0,updateID = 0;
	Byte isOfflineUpdate = 0,vipLevl = 0;
	UInt32 retLevel,now = 0;
	Int32	sec2Bill = 0;				//��Ҫ����۷ѵ�ʱ��		��λ ��
	Connection con;
	DBOperate dbo;

	 //��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());


  //��ȡ DB �Ĺһ���¼
  sprintf(szSql,"select BegTime,FinishedSec,DesSec,DesExp,DesLevel,BilledNum,SpeedUpSec,UpdateID from OffLineUpdate\
  								where RoleID = %d and IsFinish = 0;",roleID);

	iRet = dbo.QuerySQL(szSql);

	if(iRet == 1)
	{
	 	 LOG(LOG_DEBUG,__FILE__,__LINE__,"DB empty set!!" );
  	 return 0;
	}

	if(iRet != 0)
	{
	 	 LOG(LOG_ERROR,__FILE__,__LINE__,"DB op error!!" );
  	 return -1;
	}

	begTime = dbo.GetIntField(0);
	finishSec = dbo.GetIntField(1);
	desSec = dbo.GetIntField(2);
	desExp = dbo.GetIntField(3);
	desLevel = dbo.GetIntField(4);
	billedNum = dbo.GetIntField(5);
	speedupSec = dbo.GetIntField(6);
	updateID = dbo.GetIntField(7);

  //����ʱ���
  elapse = time(NULL) - begTime;
  if(elapse<0)
  {
		LOG(LOG_ERROR,__FILE__,__LINE__,"elapse error!!elapse[%d]", elapse);
		return -1;
  }

  //����ʱ���У׼
	if(elapse > (desSec - speedupSec))
	  elapse = desSec - speedupSec;
	if(elapse < finishSec)
		elapse = finishSec;

	//����ʱ���У��
	if(elapse<0)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"elapse  errro! elapse[%d]", elapse );
		return -1;
	}

	//�ж��Ƿ�һ�����
	if(elapse >= (desSec - speedupSec - finishSec))
	{
		//�һ��Ѿ�����
		isFinish = 1;
		isOfflineUpdate = 0;  //�һ�����
	}
	else
	{
		//�һ�δ����,������������ȵ�
	  isFinish = 0;
	  isOfflineUpdate = 1;
	}

	//���㲹��� �۷Ѵ���
	sec2Bill = elapse - finishSec;
	LOG(LOG_ERROR,__FILE__,__LINE__,"sec2bill[%d]",sec2Bill);
	billNum = sec2Bill / 60;
	if(sec2Bill % 60)
	  billNum++;

	//��ȡ����ǰ�ȼ��;���
	sprintf(szSql,"select Exp,Level,IsVIP\
  							from Role \
  							where RoleID = %d;", roleID);
  iRet = dbo.QuerySQL(szSql);
  if(iRet != 0)
  {
  	LOG(LOG_ERROR,__FILE__,__LINE__,"DB op error!!" );
  	return -1;
  }

	UInt32 begExp = dbo.GetIntField(0);
	UInt32 begLevel = dbo.GetIntField(1);
	vipLevl = dbo.GetIntField(2);


	//���㲹��۷�,���侭��
	UInt32 addExpSum = 0;
	double addCostSum = 0;
	UInt32 currLevel = begLevel;
	UInt32 currExp = begExp;
	UInt32 finalExp = 0;

	for(int i = 0; i < billNum; i++)
	{
		double tmpCost = 0.0;
		UInt32 tmpExp = 0;
		iRet = CalcCostPerMinite(roleID, currLevel, currExp, tmpCost, tmpExp, retLevel,finalExp,vipLevl);
		if(iRet)
		{
				LOG(LOG_ERROR,__FILE__,__LINE__,"CalcCostPerMinite error!!" );
				return -1;
		}

		addExpSum += tmpExp;
		addCostSum += tmpCost;
		currLevel = retLevel;
		currExp = finalExp;
	}

	//LOG(LOG_ERROR,__FILE__,__LINE__,"billNum[%d]", billNum );

	//����۷Ѵ�����Ϊ0,�һ�δ����
	finishSec += billedNum * 60;
	finishSec += speedupSec;
	if(billNum)
	{
		  //���� �һ���¼
	  sprintf(szSql,"update OffLineUpdate set\
			  								IsFinish = %d,\
			  								BilledNum = BilledNum + %d,\
			  								BilledMoney = BilledMoney + %f,\
			  								FinishedSec = FinishedSec + %d\
	  								where RoleID = %d and IsFinish = 0;",\
	  								isFinish,billNum,addCostSum, billNum*60 ,roleID);
//	  LOG(LOG_ERROR,__FILE__,__LINE__,"szSql[%s]",szSql);
	  iRet = dbo.ExceSQL(szSql);
	  if(iRet != 0)
	  {
	     LOG(LOG_ERROR,__FILE__,__LINE__,"DB op error!!" );
	  	 return -1;
	  }

	  //���½�ɫ����
	  //	��ʱ��ɫ��δ�����ͼ,���滹û������, ֱ���޸� DB
	  sprintf(szSql,"update Role set Exp = Exp + %d\
	  								where RoleID = %d;", addExpSum,roleID);
	  iRet = dbo.ExceSQL(szSql);
	  if(iRet != 0)
	  {
	     LOG(LOG_ERROR,__FILE__,__LINE__,"DB op error !" );
	  	 return -1;
	  }
	}

	//���½�ɫ�������߹һ�״̬
  if(isOfflineUpdate == 0)
  {
    sprintf(szSql,"update Role set IsOfflineUpdate = %d\
	  								where RoleID = %d;", isOfflineUpdate,roleID);
	  iRet = dbo.ExceSQL(szSql);
	  if(iRet != 0)
	  {
	     LOG(LOG_ERROR,__FILE__,__LINE__,"DB op error !" );
	  	 return -1;
	  }

	  sprintf(szSql,"update OffLineUpdate set IsFinish = 1\
	  								where UpdateID = %d;",updateID);
	  iRet = dbo.ExceSQL(szSql);
	  if(iRet != 0)
	  {
	     LOG(LOG_ERROR,__FILE__,__LINE__,"DB op error !" );
	  	 return -1;
	  }
  }


	return 0;
}


//����10���ӵĿ۷� ���� 0 �ɹ� ��0 ʧ��
int  OffLineUpdateSvc::SpeedTenMiniteCast(UInt32 roleID,UInt32 &speedTenMinute)
{
  int iRet = 0;
	double TmpCost = 0;
	UInt32 TmpExp = 0;
	UInt32 TmpLevel = 0;
	iRet = CalcCostAndGainsBySilver( roleID, 1, TmpCost, TmpExp,TmpLevel);
	if(iRet)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"CalcCostAndGainsBySilver error !" );
		return -1;
	}

	speedTenMinute = (int)ceil(TmpCost / 300 * 2 * 5 /6);

	return 0;
}

//vip �һ��ӳ�
int OffLineUpdateSvc::VipExtraExp(UInt32 roleID,Byte vipLevl,UInt32 &tmpExp)
{
	if(vipLevl < 1 || vipLevl > 6)
	{
		return -1;
	}

	if(1 == vipLevl)
	{
		tmpExp = (int)(1.2 * tmpExp);
	}
	else if(2 == vipLevl)
	{
		tmpExp = (int)(1.3 * tmpExp);
	}
	else if(3 == vipLevl)
	{
		tmpExp = (int)(1.4 * tmpExp);
	}
	else
	{
		tmpExp = (int)(1.5 * tmpExp);
	}

	return 0;

}


void  OffLineUpdateSvc::NotifyStopUpdate(UInt32 roleID,UInt32 desExp, UInt32 desLevl,UInt32 finishSec,UInt32 desSec)
{
  List<UInt32>lrid;
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1601;
	p.UniqID = 214;
	p.PackHeader();
	lrid.push_back(roleID);

	s<<desExp;
	s<<desLevl;
	s<<finishSec;
	s<<desSec;
	p.UpdatePacketLength();

	//LOG(LOG_ERROR,__FILE__,__LINE__,"finishSec [%d]" ,finishSec);
	//LOG(LOG_ERROR,__FILE__,__LINE__,"desSec [%d]" ,desSec);

	if( _mainSvc->Service()->Broadcast(lrid ,&serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
	}

	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}


//[MsgType:1602]�һ�����
 void OffLineUpdateSvc::NotifySpeedUpCast(UInt32 roleID, UInt32 speedupCast)
 {
  List<UInt32>lrid;
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1602;
	p.UniqID = 214;
	p.PackHeader();

	lrid.push_back(roleID);

	s<<speedupCast;

	p.UpdatePacketLength();

	if( _mainSvc->Service()->Broadcast(lrid ,&serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
	}

	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
 }

