
#include "TradeSvc.h"
#include "MainSvc.h"
#include "DBOperate.h"
#include "CoreData.h"
#include "Role.h"
#include "ArchvTrade.h"
#include "../Bag/ArchvBag.h"
#include "../Bag/BagSvc.h"

TradeSvc::TradeSvc(void* service, ConnectionPool * cp)
{
	_mainSvc = (MainSvc*)(service);
	_cp = cp;

}

TradeSvc::~TradeSvc()
{

}
//���ݰ���Ϣ



/*----------------------��������--------------------------
	�û���������,c-s 1301 ,s-c 1301,�Է��ӵ�һ��s-c��1301������(roleID,name,type=1)
	�Է��ӵ�S-C��ѡ���Է���C-S 1302   ���ܻ��߾ܾ�
	CASE:�ܽ�S-C 1301(roleID,name,type=0)�������ߣ�֪ͨ�Է��ܽ��˽���
	case:����s-c 1301(roleID,name,type=2)��˫����֪ͨ˫��˭���㽨���˽���
	����һ����Ʒ������һ����Ʒ����Э��;




*///----------------------------------------------------------
void TradeSvc::OnProcessPacket(Session& session,Packet& packet)
{
DEBUG_PRINTF1( "C_S req pkg-------MsgType[%d] \n", packet.MsgType );
	DEBUG_SHOWHEX( packet.GetBuffer()->GetReadPtr()-PACKET_HEADER_LENGTH, packet.GetBuffer()->GetDataSize()+PACKET_HEADER_LENGTH, 0, __FILE__, __LINE__ );

	switch(packet.MsgType)
	{
		case 1301: //���뽻��
			ProcessRequestTrade(session,packet);
		break;
		case 1302://����Ӧ��
			ProcessAnswerTrade(session,packet);
		break;
		case 1303://��Ʒ����
			ProcessItemPast(session,packet);
		break;
		case 1304://��������
			ProcessLockTrade(session,packet);
		break;
		case 1305://�����Ǯ
			ProcessMoney(session, packet);
		break;
		case 1306://��Ʒ��Ϣ����
		  ProcessTrade(session,packet);
		  break;
	  case 1307://ȡ������
		  ProcessCancelTrade(session, packet);
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
void TradeSvc::ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode)
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

//[MsgType:1301] ��������
void TradeSvc::ProcessRequestTrade(Session& session,Packet& packet)
{
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);

	UInt32 roleID = packet.RoleID;
	UInt32 ID=0;  //�������Ľ�ɫID
	string Name;

		//���л���
	Serializer s(packet.GetBuffer());
	s>>ID;

	RolePtr role = _mainSvc->GetCoreData()->ProcessGetRolePtr(ID);

	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}
	if(ID ==roleID)
	{
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"Request role[%d] is itsself !",ID);
			goto EndOf_Process;
	}

	//���׽�ɫ�Ƿ�����
	if(role->ID() == 0)
	{
    RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"Request role[%d] is not online !",ID);
		goto EndOf_Process;
	}

	//��ȡ��ɫ��
	role=_mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
	if(role->ID()!=0)
	{
		 Name=role->Name();
	}
	else
	{
		 RetCode = ERR_SYSTEM_DBNORECORD;
		 LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole failed !");
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

	//LOG(LOG_ERROR,__FILE__,__LINE__,"RequestTrade :RetCode[%d]",RetCode);
	//LOG(LOG_ERROR,__FILE__,__LINE__,"roleID[%d]",roleID);
	//LOG(LOG_ERROR,__FILE__,__LINE__,"roleName[%s]",Name.c_str());

	//LOG(LOG_ERROR,__FILE__,__LINE__,"toRole[%d]",ID);
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
	{//S-C��������
	  //LOG(LOG_ERROR,__FILE__,__LINE__,"NotifyRequest Succeded !");
	  NotifyRequest(roleID,Name,ID);
	}
	return;

}

//[MsgType:1302]�����׵�Ӧ��
void TradeSvc::ProcessAnswerTrade(Session& session,Packet& packet)
{
		UInt32	RetCode = 0;
		DataBuffer	serbuffer(1024);

		UInt32 roleID = packet.RoleID;
		int iRet = 0;
		UInt32 ID=0;//Ӧ�����Ľ�ɫID
		Byte type=0;
		string ReqRoleName,RoleName;
		TradeItem roleItem,tradeRoleItem;
			//���л���
		Serializer s(packet.GetBuffer());
		s>>ID>>type;

		RolePtr role = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
		if( s.GetErrorCode()!= 0 )
		{
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
			goto EndOf_Process;
		}
		if(ID==roleID)
		{
			RetCode = ERR_APP_DATA;
			LOG(LOG_DEBUG,__FILE__,__LINE__,"ID is itsself ! ID[%d]",ID);
			goto EndOf_Process;
		}

		//typeУ�� 0 �ܾ��� 1 ͬ��
	  if(type != 0 && type != 1)
	  {
	    RetCode = ERR_APP_DATA;
			LOG(LOG_DEBUG,__FILE__,__LINE__,"type error ! type[%d]",type);
			goto EndOf_Process;
	  }

    //������ζ��ᷢS-C
		if(role->ID()==0)
		{
		   RetCode = ERR_APP_OP;
			 LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole failed !");
			 goto EndOf_Process;
		}
		roleItem = role->TradeInfo();
		RoleName=role->Name();

		//���ͬ�⽻�ײŲ�ѯ�Է���Ϣ
		if(1 == type)
		{
      //�Է���Ϣ����
		  role = _mainSvc->GetCoreData()->ProcessGetRolePtr(ID);
		  if(role->ID() == 0)
		  {
		    RetCode = ERR_APP_OP;
			 	LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole failed or role[%d] is offline !",ID);
			 	goto EndOf_Process;
		  }

		  tradeRoleItem = role->TradeInfo();
		  ReqRoleName = role->Name();

		  if((tradeRoleItem.isOnTrade != 1) || (tradeRoleItem.tradeRoleID != roleID))
      {
        tradeRoleItem.isOnTrade = 1;
		    tradeRoleItem.tradeRoleID = roleID;
      }
      else
      {
       	RetCode = ERR_APP_OP;
			 	LOG(LOG_DEBUG,__FILE__,__LINE__,"TradeInfo failed !");
			 	goto EndOf_Process;
      }
			//���öԷ���ɫ������Ϣ
		  iRet = _mainSvc->GetCoreData()->TradeInfo(tradeRoleItem, ID);
		  if(iRet)
		  {
		    RetCode = ERR_APP_OP;
			 	LOG(LOG_DEBUG,__FILE__,__LINE__,"TradeInfo failed !");
			 	goto EndOf_Process;
		  }



      if(roleItem.isOnTrade != 1 || roleItem.tradeRoleID != ID)
      {
         roleItem.isOnTrade = 1;
		     roleItem.tradeRoleID = ID;
      }

		  iRet = _mainSvc->GetCoreData()->TradeInfo(roleItem,roleID);
		  if(iRet)
		  {
		    RetCode = ERR_APP_OP;
			 	LOG(LOG_DEBUG,__FILE__,__LINE__,"TradeInfo failed !");
			 	goto EndOf_Process;
		  }
			LOG(LOG_ERROR,__FILE__,__LINE__,"roleItem.tradeRoleID[%d]--RoleID[%d]",roleItem.tradeRoleID,roleID);

		}


		EndOf_Process:
		//��Ӧ������
		Packet p(&serbuffer);
		s.SetDataBuffer(&serbuffer);
		serbuffer.Reset();

		p.CopyHeader(packet);
		p.Direction = DIRECT_C_S_RESP;
		p.PackHeader();

		//LOG(LOG_ERROR,__FILE__,__LINE__,"AnswerRequest :RetCode[%d]",RetCode);
		//LOG(LOG_ERROR,__FILE__,__LINE__,"ID[%d]--type[%d]",ID,type);
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
			if(type==0)
			{//�ܾ���������S-C
			    //LOG(LOG_ERROR,__FILE__,__LINE__,"Refuse NotifyRequestResul---type[%d]",type);
					NotifyRequestResult(roleID,RoleName,ID,2);
			}
			if(type == 1)
			{//ͬ�⽻��S-C
			  //LOG(LOG_ERROR,__FILE__,__LINE__,"Agree NotifyRequestResul---type[%d]",type);
					NotifyRequestResult(roleID,RoleName,ID,1);
					NotifyRequestResult(ID,ReqRoleName,roleID,1);
			}
		}
		return;
}


//[MsgType:1303]��Ʒ��Ϣ����
void TradeSvc::ProcessItemPast(Session& session,Packet& packet)
{
		UInt32	RetCode = 0;
		DataBuffer	serbuffer(2048);

		int iRet = 0;
		UInt32 roleID = packet.RoleID;
		UInt32 ID=0;//�������Ľ�ɫID
		UInt16 CellIndex=0;
		Byte type=0;

		ItemCell lic1;
		TradeItem roleTrade;
		List<ItemCell> lic;

		//���л���
		Serializer s(packet.GetBuffer());
		s>>ID>>CellIndex>>type;

		RolePtr role = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);

		if( s.GetErrorCode()!= 0 )
		{
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
			goto EndOf_Process;
		}

		if(ID == roleID)
		{
	        RetCode = ERR_APP_DATA;
		 	LOG(LOG_ERROR,__FILE__,__LINE__,"ID[%d] is itsself !",ID);
		 	goto EndOf_Process;
		}

		//У��Է���ID
		if(role->ID() == 0)
		{
		    RetCode = ERR_APP_DATA;
		 	LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole Failed or Role[%d] is offline !",ID);
		 	goto EndOf_Process;
		}

		roleTrade = role->TradeInfo();
		if(roleTrade.tradeRoleID != ID)
		{
		    RetCode = ERR_APP_DATA;
		 	LOG(LOG_ERROR,__FILE__,__LINE__,"roleTrade.tradeRoleID[%d] != ID[%d] !",roleTrade.tradeRoleID,ID);
		 	goto EndOf_Process;
		}

		//type У�� 0 ��ʾж�£�1 ��ʾ���Ͻ���
		if(type != 0 && type != 1)
		{
		  RetCode = ERR_APP_DATA;
		  LOG(LOG_DEBUG,__FILE__,__LINE__,"Deliver item type[%d] error !",type);
		  goto EndOf_Process;
		}

		iRet = _mainSvc->GetBagSvc()->RoleSelectCell(roleID,CellIndex,lic1);
		if(iRet)
		{
		    RetCode = ERR_APP_OP;
		    LOG(LOG_DEBUG,__FILE__,__LINE__,"GetBagSvc()->RoleSelectCell Error !");
			goto EndOf_Process;
		}

		//����Ʒ���ܽ���
		if(lic1.bindStatus == 1)
		{
		    RetCode = BIND_ITEM;
		    LOG(LOG_DEBUG,__FILE__,__LINE__,"role[%d] cellIndex[%d] item bindStatus is 1 !!",roleID,CellIndex);
			goto EndOf_Process;
		}

		//lic.push_back(lic1);

		/*LOG(LOG_ERROR,__FILE__,__LINE__,"lic1.ItemID[%d]",lic1.ItemID);
		LOG(LOG_ERROR,__FILE__,__LINE__,"lic1.celIndex[%d]",lic1.celIndex);
		LOG(LOG_ERROR,__FILE__,__LINE__,"lic1.EntityID[%d]",lic1.EntityID);*/

		EndOf_Process:
		//��Ӧ������
		Packet p(&serbuffer);
		s.SetDataBuffer(&serbuffer);
		serbuffer.Reset();

		p.CopyHeader(packet);
		p.Direction = DIRECT_C_S_RESP;
		p.PackHeader();

		s<<RetCode<<CellIndex;
		LOG(LOG_ERROR,__FILE__,__LINE__,"RetCode[%d]",RetCode);
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
			NotifyToRoleBagItem(ID,roleID,lic1,type);
		}
		return;

}

void TradeSvc::ProcessLockTrade(Session& session,Packet& packet)
{
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	UInt32 roleID = packet.RoleID;
	int iRet = 0;
	TradeItem roleItem;
	UInt32 ID=0;//�������Ľ�ɫID
	UInt32 CellIndex=0;
	UInt32 money = 0,gold = 0;
	List<UInt16> lic;
	List<UInt16>::iterator iter;
	List<UInt16>::iterator tst;
	List<ItemCell> licc;
		//���л���
	Serializer s(packet.GetBuffer());
	s>>ID>>money>>gold>>lic;

	RolePtr role = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);

	//LOG(LOG_ERROR,__FILE__,__LINE__,"role[%d]---money[%d]---Gold[%d]",roleID,money,gold);
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	if(ID == roleID)
	{
	  RetCode = ERR_APP_OP;
		LOG(LOG_DEBUG,__FILE__,__LINE__,"ID[%d] is itsself !",ID);
		goto EndOf_Process;
	}

  if(role->ID() == 0)
  {
    RetCode = ERR_APP_OP;
		LOG(LOG_DEBUG,__FILE__,__LINE__,"ProcessGetRole Failed !");
		goto EndOf_Process;
  }
	//��������
  roleItem = role->TradeInfo();

  if(roleItem.isOnTrade != 1 || roleItem.tradeRoleID != ID)
  {
     RetCode = ERR_APP_OP;
     LOG(LOG_ERROR,__FILE__,__LINE__,"ID[%d]--roleItem.tradeRoleID[%d]",ID,roleItem.tradeRoleID);
     goto EndOf_Process;
  }

  roleItem.isLockTrade = 1;
  roleItem.tradeMoney = money;
  roleItem.tradeGold = gold;
  for(iter = lic.begin();iter != lic.end(); iter++)
  {
    roleItem.tradeCellIndex.push_back(*iter);
  }

  iRet = _mainSvc->GetCoreData()->TradeInfo(roleItem,roleID);
  if(iRet)
  {
   	RetCode = ERR_APP_OP;
		LOG(LOG_DEBUG,__FILE__,__LINE__,"TradeInfo Failed !");
		goto EndOf_Process;
  }

  /*LOG(LOG_DEBUG,__FILE__,__LINE__,"role[%d] Lock CellIndex ----",roleID);
  for(tst =roleItem.tradeCellIndex.begin();tst != roleItem.tradeCellIndex.end(); tst++ )
  {
     LOG(LOG_DEBUG,__FILE__,__LINE__,"role[%d] tradeIndex[%d]",roleID,*tst);
  }*/
	EndOf_Process:
	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();

	p.CopyHeader(packet);
	p.Direction = DIRECT_C_S_RESP;
	p.PackHeader();

	LOG(LOG_ERROR,__FILE__,__LINE__,"RetCode[%d]",RetCode);
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
	{//��������
		NotifyLockTrade(roleID,ID);
	}
	return;

}

void TradeSvc::ProcessMoney(Session& session,Packet& packet)//��Ǯ
{
		UInt32	RetCode = 0;
		char szSql[1024];
		DataBuffer	serbuffer(1024);
		int iRet = 0;
		Connection con;
	  DBOperate dbo;

	  TradeItem roleTrade;

		UInt32 roleID = packet.RoleID;
		UInt32 ID=0;//�������Ľ�ɫID
		UInt32 Money=0,Gold=0;
		Byte ifSndMoney = 0;
			//���л���
		Serializer s(packet.GetBuffer());
		s>>ID>>Money>>Gold;

		RolePtr role = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
		if( s.GetErrorCode()!= 0 )
		{
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
			goto EndOf_Process;
		}

		con = _cp->GetConnection();
	  dbo.SetHandle(con.GetHandle());

		//У���ɫ
		if(ID == roleID)
		{
			RetCode = ERR_APP_DATA;
			LOG(LOG_ERROR,__FILE__,__LINE__,"ID[%d] is itsself !",ID);
			goto EndOf_Process;
		}

		if(role->ID() == 0)
		{
		  RetCode = ERR_APP_DATA;
			LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole Failed !");
			goto EndOf_Process;
		}

		roleTrade = role->TradeInfo();
		if(roleTrade.tradeRoleID != ID)
		{
		  RetCode = ERR_APP_DATA;
			LOG(LOG_ERROR,__FILE__,__LINE__,"roleTrade.tradeRoleID != ID !");
			goto EndOf_Process;
		}

    if(Money == 0 && Gold == 0)
    {
      ifSndMoney = 0;
      goto EndOf_Process;
    }
		else
		{
		  ifSndMoney = 1;
			  //У���Ǯ
			sprintf(szSql,"select Money >= %d, Gold >= %d from RoleMoney\
											where RoleID = %d;",Money,Gold,roleID);
			iRet = dbo.QuerySQL(szSql);
			if(iRet != 0)
			{
			  RetCode = ERR_SYSTEM_DBERROR;
				LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL not found ! szSql[%s]",szSql);
				goto EndOf_Process;
			}

			if(dbo.GetIntField(0) == 0 || dbo.GetIntField(1) == 0)
			{
			  RetCode = ERR_APP_OP;
				LOG(LOG_ERROR,__FILE__,__LINE__,"Role[%d] has no enough money !",roleID);
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

		LOG(LOG_ERROR,__FILE__,__LINE__,"RetCode[%d]",RetCode);
		/*LOG(LOG_ERROR,__FILE__,__LINE__,"roleID[%d]",roleID);
		LOG(LOG_ERROR,__FILE__,__LINE__,"ID[%d]",ID);
		LOG(LOG_ERROR,__FILE__,__LINE__,"roleTrade.tradeRoleID[%d]",roleTrade.tradeRoleID);

		LOG(LOG_ERROR,__FILE__,__LINE__,"money[%d]",Money);
		LOG(LOG_ERROR,__FILE__,__LINE__,"gold[%d]",Gold);*/

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
		   if(ifSndMoney == 1)//���׽�Ǯ�仯ʱ��֪ͨ
		   {
		     //LOG(LOG_ERROR,__FILE__,__LINE__,"NotifyToRoleMoney Succeded !");
		     NotifyToRoleMoney(roleID,ID,Money,Gold);
		   }

		}
		return;
}

void TradeSvc::ProcessTrade(Session& session,Packet& packet)
{
		UInt32	RetCode = 0;
		DataBuffer	serbuffer(2048);

		UInt32 roleID = packet.RoleID;
		UInt32 desRoleID=0;//�������Ľ�ɫID
		string roleName,desRoleName;
		int iRet = 0;

		RolePtr roleSelf;
		RolePtr destRole;
		TradeItem roleSelfInfo,desRoleInfo;
		Byte destRoleLock = 0,roleLock = 0;//��ɫ�Ƿ���������
	  Byte isClickTrade = 0; //�Է��Ƿ�������

		//���л���
		Serializer s(packet.GetBuffer());
		if( s.GetErrorCode()!= 0 )
		{
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
			goto EndOf_Process;
		}

		//��ȡ���׶���Ľ�ɫID
		roleSelf = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
		if(roleSelf->ID() == 0)
		{
		  RetCode = ERR_APP_DATA;
			LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole Failed !" );
			goto EndOf_Process;
		}

		roleSelfInfo = roleSelf->TradeInfo();
		roleName = roleSelf->Name();

		desRoleID = roleSelfInfo.tradeRoleID;




		//LOG(LOG_ERROR,__FILE__,__LINE__,"roleID[%d]",roleID);
		//LOG(LOG_ERROR,__FILE__,__LINE__,"desRoleID[%d]",desRoleID);

		destRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(desRoleID);
		if(destRole->ID() == 0)
		{
		  RetCode = ERR_APP_DATA;
			LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole Failed !" );
			goto EndOf_Process;
		}

	  desRoleInfo = destRole->TradeInfo();
	  desRoleName = destRole->Name();
	  isClickTrade = desRoleInfo.isTrade;

		//�Ƿ��Ѿ���������
	  if(roleSelfInfo.isLockTrade != 1 || desRoleInfo.isLockTrade != 1)
	  {
	    RetCode = ERR_APP_DATA;
			LOG(LOG_ERROR,__FILE__,__LINE__,"role[%d] or destRole[%d] not lock trade !",roleID,desRoleID);
			goto EndOf_Process;
	  }

	  //�Է��Ƿ��Ѿ��������
	  if(desRoleInfo.isTrade == 1)
	  {
	  	isClickTrade = 1;
	  	LOG(LOG_ERROR,__FILE__,__LINE__,"TradeItem-----");
	  	iRet = _mainSvc->GetBagSvc()->TradeItem(roleID,roleSelfInfo.tradeCellIndex,desRoleID,desRoleInfo.tradeCellIndex);
	  	if(iRet == 0)//˫����һ�������ռ䲻��
	  	{
	  	      RetCode = ERR_APP_DATA;
			  LOG(LOG_ERROR,__FILE__,__LINE__,"role[%d] or destRole[%d] Bag has no space !",roleID,desRoleID);
			  goto EndOf_Process;
	  	}
	  	if(iRet == -1)//����ʧ��
	  	{
	  	      RetCode = ERR_APP_DATA;
			  LOG(LOG_ERROR,__FILE__,__LINE__,"TradeItem error !");
			  goto EndOf_Process;
	  	}

			//���׵Ľ�Ǯ����
	  	iRet = TradeMoney(roleSelfInfo,desRoleInfo);
	  	if(iRet)
	  	{
	  	  RetCode = ERR_APP_DATA;
			  LOG(LOG_ERROR,__FILE__,__LINE__,"TradeItem error !");
			  goto EndOf_Process;
	  	}

	  	 //������ɺ���Ҫ��ʼ����ɫ�Ľ�����Ϣ
	  	iRet = InitTradeItem(roleID);
		if(iRet)
		{
			RetCode = ERR_APP_DATA;
			LOG(LOG_ERROR,__FILE__,__LINE__,"InitTradeItem error !");
			goto EndOf_Process;
		}

		iRet = InitTradeItem(desRoleID);
		if(iRet)
		{
			RetCode = ERR_APP_DATA;
			LOG(LOG_ERROR,__FILE__,__LINE__,"InitTradeItem error !");
			goto EndOf_Process;
		}


	  }
	  else
	  {
	    isClickTrade = 0; //�Է�û�е������
	    roleSelfInfo.isTrade = 1;
	    iRet = _mainSvc->GetCoreData()->TradeInfo(roleSelfInfo,roleID);
	    if(iRet)
	    {
	    	RetCode = ERR_APP_DATA;
			LOG(LOG_ERROR,__FILE__,__LINE__,"TradeInfo failed !");
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

		//LOG(LOG_ERROR,__FILE__,__LINE__,"ClickTrade :RetCode[%d]",RetCode);
		//LOG(LOG_ERROR,__FILE__,__LINE__,"isClickTrade[%d]",isClickTrade);

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
			if(isClickTrade == 1)
			{
			//LOG(LOG_ERROR,__FILE__,__LINE__,"NotifyTradeOk 	Succeded !");
			NotifyTradeOk(roleID,1,desRoleID,desRoleName);
			NotifyTradeOk(desRoleID,1,roleID,roleName);
			}
		}
		return;
}


// ȡ������ [MsgType:1307]
void TradeSvc::ProcessCancelTrade(Session& session,Packet& packet)
{
    UInt32	RetCode = 0;
	DataBuffer	serbuffer(2048);

	UInt32 roleID = packet.RoleID;
	UInt32 desRoleID=0,tmpRoleID = 0;//�������Ľ�ɫID
	string roleName;
	int iRet = 0;

	TradeItem roleInfo;

	//���л���
	Serializer s(packet.GetBuffer());
	s>>desRoleID;

    RolePtr role = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"desRoleID[%d]",desRoleID);
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

    if(role->ID() == 0)
    {
        RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole Failed !");
		goto EndOf_Process;
    }

	roleInfo = role->TradeInfo();
	LOG(LOG_ERROR,__FILE__,__LINE__,"roleInfo.tradeRoleID[%d]",roleInfo.tradeRoleID);

		//��ɫ�Ƿ��ڽ�����
    if(roleInfo.isOnTrade != 1)
    {
        RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"roleID[%d] is not ontrade !",roleID);
		goto EndOf_Process;
    }

    //tmpRoleID = roleInfo.tradeRoleID;
		/*if(desRoleID != tmpRoleID);
		{
		  RetCode = ERR_APP_DATA;
			LOG(LOG_ERROR,__FILE__,__LINE__,"desRoleID[%d] != roleInfo.tradeRoleID[%d]",desRoleID,roleInfo.tradeRoleID);
			goto EndOf_Process;
		}*/

	roleName = role->Name();

	iRet = InitTradeItem(roleID);
	if(iRet)
	{
	    RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"InitTradeItem error !");
		goto EndOf_Process;
	}
	iRet = InitTradeItem(desRoleID);
	if(iRet)
	{
	    RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"InitTradeItem error !");
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

	LOG(LOG_ERROR,__FILE__,__LINE__,"RetCode[%d]",RetCode);

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
	if(0 == RetCode)
	{
		//ȡ������֪ͨ
		LOG(LOG_ERROR,__FILE__,__LINE__,"NotifyCancelTrade--");
		NotifyCancelTrade(desRoleID,roleID,roleName,1);
	}
	return;
}


void TradeSvc::NotifyToRoleBagItem(UInt32 roleID,UInt32 ID,ItemCell lic,Byte type)
{
		DataBuffer	serbuffer(2048);
		Serializer s( &serbuffer );
		Packet p(&serbuffer);
		serbuffer.Reset();
		List<UInt32> lrid;
		p.Direction = DIRECT_S_C_REQ;
		p.SvrType = 1;
		p.SvrSeq = 1;
		p.MsgType = 1303;
		p.UniqID = 212;
		p.PackHeader();
		lrid.push_back(roleID);
		s<<ID<<type<<lic;
		p.UpdatePacketLength();
		if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!");
		}
		DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
		DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}


void TradeSvc::NotifyToRoleMoney(UInt32 roleID,UInt32 ID,UInt32 Money,UInt32 Gold)
{
		DataBuffer	serbuffer(1024);
		Serializer s( &serbuffer );
		Packet p(&serbuffer);
		serbuffer.Reset();
		List<UInt32> lrid;
		p.Direction = DIRECT_S_C_REQ;
		p.SvrType = 1;
		p.SvrSeq = 1;
		p.MsgType = 1304;
		p.UniqID = 212;
		p.PackHeader();
		lrid.push_back(ID);
		s<<roleID<<Money<<Gold;
		p.UpdatePacketLength();
		if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!");
		}
		DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
		DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}

//�������� [MsgType:1305]
void TradeSvc::NotifyLockTrade(UInt32 roleID,UInt32 toRoleID)
{
    DataBuffer	serbuffer(1024);
		Serializer s( &serbuffer );
		Packet p(&serbuffer);
		serbuffer.Reset();
		List<UInt32> lrid;
		p.Direction = DIRECT_S_C_REQ;
		p.SvrType = 1;
		p.SvrSeq = 1;
		p.MsgType = 1305;
		p.UniqID = 212;
		p.PackHeader();
		lrid.push_back(toRoleID);
		s<<roleID;
		p.UpdatePacketLength();
		if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!");
		}
		DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
		DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}

//[MsgType:1307]:ȡ������
void TradeSvc::NotifyCancelTrade(UInt32 toRoleID,UInt32 roleID,string roleName,Byte result)
{
    DataBuffer	serbuffer(1024);
		Serializer s( &serbuffer );
		Packet p(&serbuffer);
		serbuffer.Reset();
		List<UInt32> lrid;
		p.Direction = DIRECT_S_C_REQ;
		p.SvrType = 1;
		p.SvrSeq = 1;
		p.MsgType = 1307;
		p.UniqID = 212;
		p.PackHeader();
		lrid.push_back(toRoleID);
		s<<result<<roleID<<roleName;
		p.UpdatePacketLength();
		if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!");
		}
		DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
		DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}

void TradeSvc::NotifyTradeOk(UInt32 toRoleID,Byte TradeResult,UInt32 RoleID,string RoleName)
{
	DataBuffer	serbuffer(1024);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	List<UInt32> lrid;
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1306;
	p.UniqID = 212;
	p.PackHeader();
	s<<TradeResult<<RoleID<<RoleName;
	lrid.push_back(toRoleID);

	p.UpdatePacketLength();
	if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!");
	}
	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );

}

void TradeSvc::NotifyRequest(UInt32 roleID,string Name,UInt32 toRoleID)
{
  DataBuffer	serbuffer(1024);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	List<UInt32> lrid;
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1301;
	p.UniqID = 212;
	p.PackHeader();
	lrid.push_back(toRoleID);
	s<<roleID<<Name;
	p.UpdatePacketLength();
	if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!");
	}
	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}


//[MsgYpe:1302]���׽����type��ֵΪ1��ʱ��Ϊ�ܾ�����,Ϊ2 ͬ�⽻��
void TradeSvc::NotifyRequestResult(UInt32 roleID,string Name,UInt32 ToroleID,Byte type)
{
	DataBuffer	serbuffer(1024);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	List<UInt32> lrid;
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1302;
	p.UniqID = 212;
	p.PackHeader();
	lrid.push_back(ToroleID);
	s<<roleID<<Name<<type;
	p.UpdatePacketLength();
	if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!");
	}
	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );


}
int TradeSvc::TradeMoney(TradeItem& lic1,TradeItem& lic2)
{
		char szSql[1024];
		char szMoney[100];
		char szGold[100];
		Connection con;
		DBOperate dbo;
		int iRet=0;

		UInt32 money = lic1.tradeMoney - lic2.tradeMoney;
		UInt32 gold = lic1.tradeGold - lic2.tradeGold;
		UInt32 roleTradeMoney = lic1.tradeMoney;
		UInt32 roleTradeGold = lic1.tradeGold;
		UInt32 desTradeMoney = lic2.tradeMoney;
		UInt32 desTradeGold = lic2.tradeGold;

		con=_cp->GetConnection();
		dbo.SetHandle(con);

		if(gold==0 && money==0)
		{
			return 0;
		}
		else
		{
         sprintf(szSql,"update RoleMoney set\
                        Money = (Money + %d - %d),\
                        Gold = (Gold + %d - %d)\
                        where RoleID = %d;",\
                        desTradeMoney,roleTradeMoney,\
                        desTradeGold,roleTradeGold,\
                        lic2.tradeRoleID);
         iRet = dbo.ExceSQL(szSql);
		     if(iRet != 0)
		     {
		       LOG(LOG_ERROR,__FILE__,__LINE__,"Excute SQL Error ! szSql[%s]",szSql);
		       return -1;
		     }

          sprintf(szSql,"update RoleMoney set\
                         Money = (Money + %d - %d),\
                         Gold = (Gold + %d - %d)\
                         where RoleID = %d;",\
                         roleTradeMoney,desTradeMoney,\
                         roleTradeGold,desTradeGold,\
                         lic1.tradeRoleID);

		     iRet = dbo.ExceSQL(szSql);
		     if(iRet != 0)
		     {
		       LOG(LOG_ERROR,__FILE__,__LINE__,"Excute SQL Error ! szSq[%s]",szSql);
		       return -1;
		     }

		 }

		 LOG(LOG_ERROR,__FILE__,__LINE__,"roleID[%d]",lic2.tradeMoney);
     LOG(LOG_ERROR,__FILE__,__LINE__,"lic1.tradeMoney[%d]",lic1.tradeMoney);
     LOG(LOG_ERROR,__FILE__,__LINE__,"lic1.tradeGold[%d]",lic1.tradeGold);

     LOG(LOG_ERROR,__FILE__,__LINE__,"desRoleID[%d]",lic1.tradeRoleID);
		 LOG(LOG_ERROR,__FILE__,__LINE__,"lic2.tradeMoney[%d]",lic2.tradeMoney);
		 LOG(LOG_ERROR,__FILE__,__LINE__,"lic2.tradeGold[%d]",lic2.tradeGold);


		 LOG(LOG_ERROR,__FILE__,__LINE__,"NotifyMoney --!!!");

		//��Ǯ֪ͨ
		_mainSvc->GetBagSvc()->NotifyMoney(lic1.tradeRoleID);
		_mainSvc->GetBagSvc()->NotifyMoney(lic2.tradeRoleID);
		return 0;
}


//��ɫ�Ƿ������˽��� ����ֵ 0 Ϊ������1 �Ѿ������� -1 ʧ��
int TradeSvc::IsLockTrade(UInt32 roleID)
{
	int iRet = 0;
	TradeItem roleTradeInfo;

	RolePtr role = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
	if(role->ID() == 0)
	{
	  LOG(LOG_DEBUG,__FILE__,__LINE__,"ProcessGetRole Failed ! role[%d]",roleID);
	  return -1;
	}

	roleTradeInfo = role->TradeInfo();

	if(roleTradeInfo.isLockTrade == 1)
	{
	  return 1;
	}

	return 0;

}

//��ɫ���׽����󣬽�����Ϣ���óɳ�ʼ��״̬
//����ֵ 0 �ɹ����� 0 ʧ��
int TradeSvc::InitTradeItem(UInt32 roleID)
{
  int iRet = 0;
  TradeItem roleTradeInfo;
  roleTradeInfo.isLockTrade = 0;
  roleTradeInfo.isOnTrade = 0;
  roleTradeInfo.isTrade = 0;
  roleTradeInfo.tradeGold = 0;
  roleTradeInfo.tradeGold = 0;
  roleTradeInfo.tradeRoleID = 0;

  iRet = _mainSvc->GetCoreData()->TradeInfo(roleTradeInfo, roleID);
  if(iRet)
  {
    LOG(LOG_DEBUG,__FILE__,__LINE__,"Init TradeInfo Failed ! roleID[%d]",roleID);
    return -1;
  }

  return 0;

}


//��ɫ�Ƿ��ڽ����� ����ֵ 0 δ�ڽ��ף�1 ���ڽ��ף� -1 ʧ��
int TradeSvc::IsOnTrade(UInt32 roleID)
{
  int iRet = 0;
  TradeItem roleTradeInfo;

  RolePtr role = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
  if(role->ID() == 0)
  {
    LOG(LOG_DEBUG,__FILE__,__LINE__,"ProcessGetRole Failed !");
    return -1;
  }

  roleTradeInfo = role->TradeInfo();
  if(roleTradeInfo.isOnTrade == 1)
  {
    return 1;
  }

  return 0;
}



//test
void TradeSvc::Test(TradeItem trade,UInt32 roleID)
{

  List<UInt16>::iterator iter;
  LOG(LOG_ERROR,__FILE__,__LINE__,"------------roleID[%d]",roleID);
  LOG(LOG_ERROR,__FILE__,__LINE__,"isLockTrade[%d]",trade.isLockTrade);

  LOG(LOG_ERROR,__FILE__,__LINE__,"tradeMoney[%d]",trade.tradeMoney);
  LOG(LOG_ERROR,__FILE__,__LINE__,"tradeGold[%d]",trade.tradeGold);

  for(iter = trade.tradeCellIndex.begin();iter != trade.tradeCellIndex.end(); iter++)
  {
    LOG(LOG_ERROR,__FILE__,__LINE__,"cellIndex[%d]",*iter);
  }

}


