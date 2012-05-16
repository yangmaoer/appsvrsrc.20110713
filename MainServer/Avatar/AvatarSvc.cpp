#include "AvatarSvc.h"
#include "MainSvc.h"
#include "DBOperate.h"
#include "ArchvAvatar.h"
#include "CoreData.h"
#include "Role.h"
#include "ArchvBagItemCell.h"
#include "../Bag/BagSvc.h"
#include "../Task/TaskSvc.h"
#include "./Bag/ArchvBag.h"

#define TOPCELL_NUM 25

AvatarSvc::AvatarSvc(void* service, ConnectionPool * cp)
{
	_mainSvc = (MainSvc*)(service);
	_cp = cp;

}

AvatarSvc::~AvatarSvc()
{
}

void AvatarSvc::OnProcessPacket(Session& session,Packet& packet)
{

DEBUG_PRINTF1( "C_S req pkg-------MsgType[%d] \n", packet.MsgType );
	DEBUG_SHOWHEX( packet.GetBuffer()->GetReadPtr()-PACKET_HEADER_LENGTH, packet.GetBuffer()->GetDataSize()+PACKET_HEADER_LENGTH, 0, __FILE__, __LINE__ );

	switch(packet.MsgType)
	{
		case 501: //
			ProcessGetRolesAvatar(session,packet);
			break;

		case 502: //
			ProcessUpdateAvatar(session,packet);
			break;

		case 503: //
			ProcessGetRolesBriefAvatar(session,packet);
			break;
		case 504://
			ProcessGetRolesEquipPos(session,packet);
			break;

	//	case 505://��ʱû�����
//			 ProcessRolesEquipChange(session,packet);
	//		break;

		case 506://
			ProcessRolesEquipGetoff(session,packet);
			break;

	   case 507:// �������˲���
	       ProcessTestByWangLian(session,packet);
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
void AvatarSvc::ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode)
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



//װ������У��
	//return  0 �ɹ�   ��0 ʧ��


void AvatarSvc::ProcessGetRolesAvatar(Session& session,Packet& packet)
{
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	UInt32 entityID;

	List<UInt32> listRoleID;
	List<UInt32>::iterator itor;
	char szCat[1024];
	char szTmp[128];
	ArchvAvatarDesc aad;
	List<ArchvAvatarDesc> laad;
	int iCount = 0;


	//���л���
	Serializer s(packet.GetBuffer());
	s>>listRoleID;
	//=========================���Ե�
	//_mainSvc->Get
//	_mainSvc->GetCoreData()->ProcessGetRole(Role).IfTheExpToMax();
	//=============================
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//����У��
	if( listRoleID.size() == 0 )
	{
		RetCode = ERR_SYSTEM_PARAM;
		LOG(LOG_ERROR,__FILE__,__LINE__,"param error!" );
		goto EndOf_Process;
	}

	//��ȡ����
	iCount = 0;
	sprintf( szCat, "  Equip.ItemID<>0 and Equip.RoleID in ( " );
	for( itor = listRoleID.begin(); itor != listRoleID.end(); itor++ )
	{
		if( ++iCount < listRoleID.size() )
			sprintf( szTmp, "%d, ", *itor );
		else
			sprintf( szTmp, "%d) order by RoleID,EquipIndex; ", *itor );

		strcat( szCat, szTmp );
	}

	sprintf( szSql, "select Equip.RoleID,Equip.EquipIndex, \
											Equip.ItemID,Equip.EntityID, Entity.Durability, Entity.BindStatus \
									from Equip,Entity where Equip.EntityID=Entity.EntityID and "  );
	strcat( szSql, szCat );

	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		//RetCode = ERR_SYSTEM_DBNORECORD;
		//LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
		goto EndOf_Process;
	}
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
 	}

	while(dbo.HasRowData())
	{
		aad.roleId = dbo.GetIntField(0);
		aad.equipIndex = dbo.GetIntField(1);
		aad.itemID= dbo.GetIntField(2);
		aad.entityID=dbo.GetIntField(3);
		aad.durability=dbo.GetIntField(4);
		aad.bindStatus=dbo.GetIntField(5);


		laad.push_back(aad);

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
		s<<laad;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

	DEBUG_PRINTF1( "C_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );

}





void AvatarSvc::ProcessUpdateAvatar(Session& session,Packet& packet)
{
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	char szTmp[128];
	Connection con,conn;
	DBOperate dbo,dbo1;
	int iRet = 0;
	int flag=0;
	List<ItemCell> lic;
	List<ItemCell> lic2;//�����洢���ϵ�װ��
	ItemCell lic1,lic22;//������ȡ�Ķ���
	List<UInt32>::iterator itor1;
	List<ArchvroleBonuschange>::iterator itor2;
	ArchvroleBonuschange il;
	UInt32 ItemID=0,ItemID1=0;
	UInt32 EntityID=0,EntityID1=0;
	UInt16 ItemType=0,ItemType1=0;
	UInt16 celIndex=0,num=0,bind=0,dur;
	Byte equipIndex=0;
	List <ArchvroleBonuschange> Bon1,Bon2,Bon3;
	List <ArchvroleEuip> Ebon,Ebon1;
	List<UInt32> l,l1;
	UInt32 roleID = packet.RoleID;
	Byte postion;
	UInt32 count;
	UInt16 flag1=0;
	UInt16 MinLevel;
	Byte	equipType = 0;
	List<Byte> pos;
	UInt32 wepID=0,coatID=0,iD=0;
	List<Byte>::iterator itor;
	UInt16 ProID;
	//List<Byte> lic23;


	//���л���
	Serializer s(packet.GetBuffer());
	s>>celIndex>>equipIndex;

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




	LOG(LOG_ERROR,__FILE__,__LINE__,"type[%d],type[%d] ",celIndex,equipIndex);

	//����У�飬�ж��϶����ǲ���װ��
	sprintf( szSql, "select Bag.ItemType,Bag.ItemID,Bag.EntityID,Bag.Num, \
												Entity.Durability,Entity.BindStatus from Bag left join Entity on \
												 Bag.EntityID=Entity.EntityID where \
												Bag.RoleID= %d and Bag.CellIndex=%d;", roleID,celIndex);
	iRet = dbo.QuerySQL(szSql);
 	if(0==iRet)
 	{
	 		if(dbo.RowNum()==1)
	 		{
			 			while(dbo.HasRowData())
						{
							ItemType=dbo.GetIntField(0);
							ItemID=dbo.GetIntField(1);
							EntityID=dbo.GetIntField(2);
							num=dbo.GetIntField(3);
							dur=dbo.GetIntField(4);
							bind=dbo.GetIntField(5);
							dbo.NextRow();
						}

			}
			lic22.bindStatus=bind;
			lic22.durability=dur;
			lic22.EntityID=EntityID;
			lic22.ItemID=ItemID;
			lic22.num=1;

	}
	else
	{
			RetCode = ERR_SYSTEM_DBERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			goto EndOf_Process;
 	}

	if( ItemType!=2)
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"error data, equipType[%d]", equipType);
		goto EndOf_Process;
	}


	//����У��,�ж�Ŀ��λ���Ƿ��ܷŸ�װ��
	sprintf( szSql, "select MinLevel,ProID,EquipPos from Item where ItemID=%d;",ItemID);
	iRet=dbo.QuerySQL(szSql);
	if(dbo.HasRowData())
	{
		MinLevel=dbo.GetIntField(0);//�ȼ�����
		ProID=dbo.GetIntField(1);//ְҵ����
		count=dbo.GetIntField(2);
	}
	if(MinLevel>pRole->Level())
	{
		//�ȼ�����
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"you level is not enough !! itemlev is %d !!your lev is %d" ,MinLevel,pRole->Level());
		goto EndOf_Process;
	}
	if(ProID!=0)
	{
		//ְҵ�ж�
		for(;ProID>0;ProID=ProID/10)
		{
			if(ProID%10==pRole->ProID())
			{
				flag1=1;
				break;
			}
		}
		if(flag1==0)
		{
			//ְҵ������
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"the pro can not use it");
			goto EndOf_Process;
		}
	}
	//	if(_mainSvc->GetCoreData()->ProcessGetRolePtr(roleID).)
	//--------------------------
	//û����֤�ȼ�
	//===========================
	//�������ˣ���ʼװ��װ��
	//�����������de����װ����Ҫ����װ���ܴ���λ�����
	//sprintf( szSql, "select EquipIndex,ItemType,ItemID,EntityID from Equip where RoleID= %d and ItemID=0 and EquipIndex in(", roleID);
	while(count%100<=13&&count>0)
	{
		postion=(Byte)(count%100);
		pos.push_back(postion);
		count=count/100;
	}
	if(equipIndex==0)
	{
	//Ҫ����0�͵��Լ��ж�λ��
		flag=1;
	}
	else
	{
		for( itor=pos.begin(); itor != pos.end(); itor++ )
		{
			if(equipIndex==*itor)
			{
				flag=1;
				break;
			}
		}
	}


	//λ���ж����
	if(flag==0)
	{
		RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"error data, equipIndex[%d]", equipIndex);
		goto EndOf_Process;
	//װ�����ܷŸ�λ��
	}


	else 	if(flag==1)
	{
			if(equipIndex==0)
			{
				sprintf( szSql, "select EquipIndex,ItemType, \
					ItemID,EntityID from Equip where RoleID= %d \
					and ItemID=0 and EquipIndex in(", roleID);
				for( itor=pos.begin(); itor != pos.end(); itor++ )
				{
					sprintf( szTmp, "%d, ", *itor );
					strcat( szSql, szTmp );
				}
				sprintf( szTmp, "15);");
				strcat( szSql, szTmp);
			}
			else
			{
				sprintf( szSql, "select EquipIndex,ItemType,ItemID,EntityID \
				from Equip where RoleID= %d and \
					ItemID=0 and EquipIndex =%d",roleID,equipIndex);
			}

		  iRet=dbo.QuerySQL(szSql);
			//��ѯ��װ����λ���Ƿ���װ��
		  if( 1 == iRet )
		  {
				  //û�в鵽����,Ŀ��ȫ������װ����
						if(equipIndex==0)
						{
							itor=pos.begin();
							//postion=*itor;
							equipIndex=*itor;
						}
						sprintf( szSql, "select Equip.ItemType, \
								Equip.ItemID,Equip.EntityID,Entity.Durability, \
								Entity.BindStatus from Equip,Entity where Equip.EntityID=Entity.EntityID \
								and Equip.RoleID=%d and Equip.EquipIndex=%d;",roleID,equipIndex);
						iRet=dbo.QuerySQL(szSql);
						if(0==iRet)
						{
							if(dbo.HasRowData())
							{
								ItemType1=dbo.GetIntField(0);
								ItemID1=dbo.GetIntField(1);
								EntityID1=dbo.GetIntField(2);
								lic1.durability=dbo.GetIntField(3);
								lic1.bindStatus=dbo.GetIntField(4);
							}
								lic1.ItemID=ItemID1;
								lic1.EntityID=EntityID1;
								lic1.num=1;
								lic1.celIndex=celIndex;
					}
					else
					{

							RetCode = DIRECT_S_S_RESP;
							LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error or not found[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
							goto EndOf_Process;
					}


				sprintf( szSql, "update Equip set ItemType=%d,ItemID=%d, \
				EntityID=%d where RoleID=%d and EquipIndex=%d;",ItemType,ItemID,
																								EntityID,roleID,equipIndex);
				iRet = dbo.ExceSQL(szSql);
				if(1==iRet)
				{
					RetCode = DIRECT_S_S_RESP;
					LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
					goto EndOf_Process;
				}
				sprintf( szSql, "update Bag set CellType=1,ItemType=%d, \
									ItemID=%d,EntityID=%d,Num=1 where RoleID=%d and \
												CellIndex=%d;",ItemType1,ItemID1,EntityID1,roleID,celIndex);
				iRet=dbo.ExceSQL(szSql);
				if(1==iRet)
				{
					LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
				}

		 }
			else if( iRet < 0 )
			{
				RetCode = DIRECT_S_S_RESP;
				LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
				goto EndOf_Process;
		 	}
			else if(iRet==0)
		 	{//Ŀ��û����Ʒ

			 		if(equipIndex==0)
					{
						equipIndex=(Byte)dbo.GetIntField(0);
					}
			 		sprintf( szSql, "update Equip set ItemType=%d,ItemID=%d, \
			 											EntityID=%d where RoleID=%d and \
			 							EquipIndex=%d;",ItemType,ItemID,EntityID,roleID,equipIndex);
			 		iRet=dbo.ExceSQL(szSql);
					if( 1 == iRet )
					{
						LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found ,szSql[%s] " , szSql);
					}
				//���������ǰ�װ���ӱ�������DEL��
					sprintf( szSql, "delete from Bag where RoleID=%d \
												and CellIndex=%d;",roleID,celIndex);
			 		iRet=dbo.ExceSQL(szSql);

					if( 1 == iRet )
					{
						LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found ,szSql[%s] " , szSql);
					}
			 }
	//���˾�OK��?
	//	entityID,itemID ����ӦroleID��У��
	}
	//װ������У��
  //ж��װ��
  //����װ��


  if(equipIndex==8||equipIndex==3)
		{
				sprintf( szSql, "select RoleID,EquipIndex, \
									ItemID \
										from Equip \
										where RoleID = %d \
									 and ItemID<>0 and EquipIndex in( 8,3);", roleID);
		iRet = dbo.QuerySQL(szSql);
		if( iRet < 0 )
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		}

		//ȡ������¼
		while(dbo.HasRowData())
		{
				if(0==iRet)
				{
					iD=dbo.GetIntField(1);
					if(iD==8)
					{
						wepID=dbo.GetIntField(2);
					}
					if(iD==3)
					{
						coatID=dbo.GetIntField(2);
					}

					//LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL[%d]     ,[%d] ", adb.wpnItemID,adb.flag);
				}
				dbo.NextRow();
			}
		}

  lic22.celIndex=equipIndex;
  if(lic1.num!=0)
  {
  	lic.push_back(lic1);//������
  }
  lic2.push_back(lic22);//װ����

  			//================���Ա仯��ж��

		//l.clear;
		//========================
		//==========����

			if(GetJustItemBonus(lic22.ItemID,Bon1,l)==0)
			{
				if(lic1.num!=0)
				{
						GetJustItemBonus(lic1.ItemID,Bon2,l);
						GetRoleBonusoff(roleID,Bon2);

				}
					GetRoleBonusin(roleID,Bon1);

			}


		for(int i=1;i<=16;i++)
		{
			il.BonusAttrID=i;
			il.Num=RoleGetNewBonus(roleID,i,pRole);
			Bon3.push_back(il);
		}
		if(lic1.num!=0)
		{
			_mainSvc->GetTaskSvc()->OnBagItemAddOrDelete(roleID,lic1.ItemID);
		}
		_mainSvc->GetTaskSvc()->OnBagItemAddOrDelete(roleID,lic22.ItemID);


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
		s<<equipIndex;
		s<<celIndex;
	}


	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF( "ack pkg=======, \n" );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );
	//�����㲥
	if( 0 == RetCode )
	{

		if(equipIndex==8||equipIndex==3)
		{
			//Avatar ֪ͨ
			NotifyAvatarUpdate(roleID,wepID,coatID);

			//��װ���¼�,�����������״̬
		}
			_mainSvc->GetTaskSvc()->OnRoleDress(roleID, ItemID);

		if(lic.size()!=0)
		{
			_mainSvc->GetBagSvc()->NotifyBag(roleID,lic);
		}
		NotifyEquipUpdate(roleID,lic2);//װ��֪ͨ
		NotifyBonus(roleID,Bon3);
		}


}

UInt32 AvatarSvc::UseItem(UInt32 roleID,UInt32 ItemID,UInt32 EntityID,UInt32 EquipPos,UInt16 celIndex)
{//��ֹ 20110510 ,ֻ֧�ִ�װ�� by Steve
	Byte postion;
	list<Byte> pos;
	list<Byte>::iterator itor;
	Byte flag=0;
	UInt32 RetCode=0;
	char szSql[1024];
	char szTmp[512];
	int iRet=0;
	Connection con;
	DBOperate dbo;
	Byte equipIndex=0;

	List<ItemCell> lic;
	List<ItemCell> lic2;//�����洢���ϵ�װ��
	ItemCell ic1,ic2;//������ȡ�Ķ���
	List<UInt32>::iterator itor1;
	List<ArchvroleBonuschange>::iterator itor2;
	ArchvroleBonuschange il;
	List <ArchvroleBonuschange> Bon1,Bon2,Bon3;
	List <ArchvroleEuip> Ebon,Ebon1;
	List<UInt32> l,l1;

	UInt32 wepID=0,coatID=0,iD=0;

	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);

	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());


LOG(LOG_ERROR,__FILE__,__LINE__,"roleID[%d], ItemID[%d], EntityID[%d], EquipPos[%d], celIndex[%d]  ", roleID, ItemID, EntityID, EquipPos, celIndex );

	UInt32 tmpPos = EquipPos;
	while(tmpPos%100<=13&&tmpPos>0)
	{
		postion=(Byte)(tmpPos%100);
		pos.push_back(postion);
		tmpPos=tmpPos/100;
	}

	//��Ʒʵ�����
	sprintf( szSql, "select Durability,BindStatus from Entity where EntityID=%d;",EntityID);
	iRet=dbo.QuerySQL(szSql);
	if(iRet!=0)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return ERR_APP_DATA;
	}
	ic2.bindStatus=(Byte)dbo.GetIntField(0);
	ic2.durability=dbo.GetIntField(1);

	ic2.EntityID=EntityID;
	ic2.ItemID=ItemID;
	ic2.num=1;

	//��װ���Ƿ��Ѿ�������
	sprintf( szSql, "select EquipIndex,ItemType,ItemID,EntityID \
									from Equip \
									where RoleID= %d \
									  and EntityID = %d ", roleID, EntityID );
	iRet=dbo.QuerySQL(szSql);
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
	}
	else if(iRet==0)
	{//װ���Ѿ�������,�����κδ���
		return 0;
	}
	else if( 1 == iRet )
	{//��װ��û�д�����
	 	//����Ӧ�ô���λ��

LOG(LOG_ERROR,__FILE__,__LINE__,"equipIndex[%d] ", equipIndex );
		equipIndex = *(pos.begin());

LOG(LOG_ERROR,__FILE__,__LINE__,"equipIndex[%d] ", equipIndex );

		//����װ����
		sprintf( szSql, "update Equip set ItemType=2,ItemID=%d,EntityID=%d where RoleID=%d and EquipIndex=%d;",ItemID,EntityID,roleID,equipIndex);
		iRet = dbo.ExceSQL(szSql);
		if(1==iRet)
		{
			RetCode = DIRECT_S_S_RESP;
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			return RetCode;
		}

		//��װ���ӱ�������DEL��
		sprintf( szSql, "delete from Bag where RoleID=%d and CellIndex=%d;",roleID,celIndex);
		iRet=dbo.ExceSQL(szSql);
		if( 1 == iRet )
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found ,szSql[%s] " , szSql);
		}

	}

	//װ������У��
	//ж��װ��
	//����װ��
	if(equipIndex==8||equipIndex==3)
	{
		sprintf( szSql, "select RoleID,EquipIndex, \
							ItemID \
								from Equip \
								where RoleID = %d \
							 and ItemID<>0 and EquipIndex in( 8,3);", roleID);
		iRet = dbo.QuerySQL(szSql);
		if( iRet < 0 )
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		}

		//ȡ������¼
		while(dbo.HasRowData())
		{
			if(0==iRet)
			{
				iD=dbo.GetIntField(1);
				if(iD==8)
				{
					wepID=dbo.GetIntField(2);
				}
				if(iD==3)
				{
					coatID=dbo.GetIntField(2);
				}

				//LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL[%d]     ,[%d] ", adb.wpnItemID,adb.flag);
			}
			dbo.NextRow();
		}
	}



	ic2.celIndex = equipIndex;
	ic1.celIndex = celIndex;
	ic1.ItemID = ItemID;
	ic1.EntityID = EntityID;
	ic1.num = 0;	//ɾ����Ʒ

	lic.push_back(ic1);//������
	lic2.push_back(ic2);//װ����



	if(GetJustItemBonus(ic2.ItemID,Bon1,l)==0)
	{
		if(ic1.num!=0)
		{
				GetJustItemBonus(ic1.ItemID,Bon2,l);
				GetRoleBonusoff(roleID,Bon2);

		}
		GetRoleBonusin(roleID,Bon1);
	}

	for(int i=1;i<=16;i++)
	{
		il.BonusAttrID=i;
		il.Num=RoleGetNewBonus(roleID,i,pRole);
		Bon3.push_back(il);
	}

	_mainSvc->GetTaskSvc()->OnBagItemAddOrDelete(roleID,ic1.ItemID);
	_mainSvc->GetTaskSvc()->OnBagItemAddOrDelete(roleID,ic2.ItemID);

	if(equipIndex==8||equipIndex==3)
	{

LOG(LOG_ERROR,__FILE__,__LINE__,"notify________  roleID[%d], wepID[%d] coatID[%d] ", roleID, wepID, coatID );
		//Avatar ֪ͨ
		NotifyAvatarUpdate(roleID,wepID,coatID);
	}

	//��װ���¼�,�����������״̬
	_mainSvc->GetTaskSvc()->OnRoleDress(roleID, ItemID);


LOG(LOG_ERROR,__FILE__,__LINE__,"equipIndex[%d],lic.size()[%d],lic2.size()[%d] ",
	equipIndex, lic.size(),lic2.size() );

	if(lic.size()!=0)
	{
		_mainSvc->GetBagSvc()->NotifyBag(roleID,lic);
	}
	NotifyEquipUpdate(roleID,lic2);//װ��֪ͨ
	NotifyBonus(roleID,Bon3);

	return 0;
}

void AvatarSvc::ProcessGetRolesBriefAvatar(Session& session,Packet& packet)
{
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);

	List<UInt32> listRoleID;
	List<ArchvAvatarDescBrief> ladb;

	//���л���
	Serializer s(packet.GetBuffer());
	s>>listRoleID;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//��ȡDB����
	GetEquipBrief(listRoleID,ladb);


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
		s<<ladb;
	}

	p.UpdatePacketLength();

	//����Ӧ������
	if( session.Send(&serbuffer) )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
	}

DEBUG_PRINTF1( "C_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );

}



void AvatarSvc::NotifyEquipUpdate(UInt32 RoleID,List<ItemCell>& lic)
{

		List<UInt32> lrid;
		DataBuffer	serbuffer(8196);
		Serializer s( &serbuffer );
		Packet p(&serbuffer);
		serbuffer.Reset();
		p.Direction = DIRECT_S_C_REQ;
		p.SvrType = 1;
		p.SvrSeq = 1;
		p.MsgType = 502;
		p.UniqID = 252;
		p.PackHeader();
		lrid.push_back(RoleID);
		s<<lic;

		p.UpdatePacketLength();

		if( _mainSvc->Service()->Broadcast( lrid, &serbuffer))
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
		}

		DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
		DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}

void AvatarSvc::NotifySingleBonus(UInt32 RoleID, UInt16 bonusAttrID, UInt32 num){
	ArchvroleBonuschange il;
	List<ArchvroleBonuschange> Bon;
	il.BonusAttrID=bonusAttrID;
	il.Num=num;
	Bon.push_back(il);

	NotifyBonus(RoleID, Bon);
}

void AvatarSvc::NotifyBonus(UInt32 RoleID,List<ArchvroleBonuschange> &k)
{
			List<UInt32> lrid;
			DataBuffer	serbuffer(8196);
			Serializer s( &serbuffer );
			Packet p(&serbuffer);
			serbuffer.Reset();
			p.Direction = DIRECT_S_C_REQ;
			p.SvrType = 1;
			p.SvrSeq = 1;
			p.MsgType = 302;
			p.UniqID = 214;
			p.PackHeader();
			lrid.push_back(RoleID);
			s<<k;

			p.UpdatePacketLength();

			if( _mainSvc->Service()->Broadcast( lrid, &serbuffer))
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
			}

			DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
			DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );

}





void AvatarSvc::NotifyAvatarUpdate( UInt32 roleID ,UInt32 wepID,UInt32 coatID)
{
	list<UInt32> lrid;
	RolePtr pRole=_mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
	UInt32 mapID=pRole->MapID();
	Byte ProID=pRole->ProID();
	_mainSvc->GetCoreData()->GetMapRoleIDs(mapID,roleID,lrid);

	//exclude me
	//lrid.remove(roleID);
	lrid.push_back(roleID);
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();

	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 501;
	p.UniqID = 0;

	p.PackHeader();

	s<<roleID<<ProID<<wepID<<coatID;

	p.UpdatePacketLength();

	if( _mainSvc->Service()->Broadcast( lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
	}

	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );

}
//504װ����ʾλ��
void AvatarSvc::ProcessGetRolesEquipPos(Session& session,Packet& packet)
{

		UInt32	RetCode = 0;
		DataBuffer	serbuffer(1024);
		char szSql[1024];
		Connection con;
		DBOperate dbo;
		int iRet = 0;
		UInt16 celIndex;
		//List<UInt32> listRoleID;
		//List<UInt32>::iterator itor;
	//	char szCat[1024];
		//char szTmp[128];
		UInt32 roleID = packet.RoleID;
		Byte equippos;
		List<Byte> eq;
		con = _cp->GetConnection();
		dbo.SetHandle(con.GetHandle());
		UInt32 count;


		//���л���
		Serializer s(packet.GetBuffer());
		s>>celIndex;
		if( s.GetErrorCode()!= 0 )
		{
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
			goto EndOf_Process;
		}
		sprintf( szSql, "select EquipPos from Item where ItemID in (select ItemID from Bag where RoleID= %d and CellIndex=%d);", roleID,celIndex);
		iRet = dbo.QuerySQL(szSql);

		if( 1 == iRet )
		{
			RetCode = ERR_SYSTEM_DBNORECORD;
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
			goto EndOf_Process;
		}
 		if(0==iRet)
 		{
 		//MinLevel=dbo.GetIntField(0);//�ȼ�����
			count=dbo.GetIntField(0);
 		}
 		while(count%100<=13&&count>0)
		{
		equippos=(Byte)(count%100);
		eq.push_back(equippos);
		count=count/100;
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
		s<<eq;
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

//506 ���ŵ�װ������
void AvatarSvc::ProcessRolesEquipGetoff(Session& session,Packet& packet)
{
		UInt32	RetCode = 0;
		DataBuffer	serbuffer(1024);
		char szSql[1024];
		Connection con;
		DBOperate dbo;
		int iRet = 0;
		List<ItemCell> lic;
		ItemCell lic1;//������ȡ�Ķ���
		List<ItemCell> lic2;
		ItemCell lic22;//������ȡ�Ķ���
		Byte equipIndex;
		UInt16 celIndex;
		UInt32 wepID=0,coatID=0,iD=0;
		List<Byte>::iterator itor;
		List<UInt32>::iterator itor1;
		List<ArchvroleBonuschange>::iterator itor2;
		ArchvroleBonuschange il;
		UInt32 roleID = packet.RoleID;
		List <ArchvroleBonuschange> Bon1,Bon2;
		List <ArchvroleEuip> Ebon;
		List<UInt32> l;
		UInt16 ItemType;

		con = _cp->GetConnection();
		dbo.SetHandle(con.GetHandle());

		//���л���
		Serializer s(packet.GetBuffer());
		s>>equipIndex>>celIndex;

		RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);

//		lic22.celIndex=equipIndex;
		if(celIndex==0)
		{
			celIndex=_mainSvc->GetBagSvc()->IfhascellIndex(roleID);
		}
		if(celIndex!=0)
		{
			sprintf( szSql, "select ItemType,ItemID,EntityID from Equip where RoleID=%d and EquipIndex=%d and ItemID<>0;",roleID,equipIndex);

			iRet=dbo.QuerySQL(szSql);
			if(iRet!=0)
			{
			//������
				RetCode = ERR_SYSTEM_DBNORECORD;
				LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found or erro,szSql[%s] " , szSql);
				goto EndOf_Process;
			}

			if(iRet==0)
			{
				ItemType=dbo.GetIntField(0);
				lic1.ItemID=dbo.GetIntField(1);
				lic1.EntityID=dbo.GetIntField(2);
				sprintf( szSql, "insert into Bag(RoleID,CellIndex,CellType,ItemType,ItemID,EntityID,Num) values(%d,%d,1,%d,%d,%d,1);",roleID,celIndex,ItemType,lic1.ItemID,lic1.EntityID);
				iRet = dbo.ExceSQL(szSql);
				if(iRet!=0)
				{
				//���ǿյĶ����ش���
					RetCode = ERR_SYSTEM_DBNORECORD;
					LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s] " , szSql);
					goto EndOf_Process;
				}
				if(iRet==0)
				{
					//�ɹ��Ժ���±���װ��
						sprintf( szSql, "update Equip set ItemType=0,ItemID=0,EntityID=0 where RoleID=%d and EquipIndex=%d;",roleID,equipIndex);
						iRet = dbo.ExceSQL(szSql);
						if(iRet!=0)
						{
								RetCode = ERR_SYSTEM_DBNORECORD;
								LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s] " , szSql);
								goto EndOf_Process;
						}
						lic1.celIndex=celIndex;
						lic1.num=1;


				}
			//ָ����Ŀ���λ�õ�
			//ж�°�Ŀ���ֵ��ȥ
		 }


		}
		else
		{
				RetCode = ERR_SYSTEM_DBNORECORD;
				LOG(LOG_ERROR,__FILE__,__LINE__," Bag has no room !" );
				goto EndOf_Process;
		}
		//================���Ա仯


		//���ж�µ�װ�����;öȵ���Ϣ
		if(lic1.EntityID!=0)
		{
			sprintf(szSql,"select Durability,BindStatus from Entity where EntityID=%d",lic1.EntityID);
			iRet=dbo.QuerySQL(szSql);
			if(iRet!=0)
			{
					LOG(LOG_ERROR,__FILE__,__LINE__," Item sql erro or others [%s]",szSql);
			}
			lic1.durability=dbo.GetIntField(0);
			lic1.bindStatus=dbo.GetIntField(1);
			lic.push_back(lic1);
		}
		if(GetJustItemBonus(lic1.ItemID,Bon1,l)==0)
		{
				GetRoleBonusoff(roleID,Bon1);

		}

			for(int i=1;i<=16;i++)
			{
				il.BonusAttrID=i;
				il.Num=RoleGetNewBonus(roleID,i,pRole);
				Bon2.push_back(il);
			}

		_mainSvc->GetTaskSvc()->OnBagItemAddOrDelete(roleID,lic1.ItemID);

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
			s<<celIndex;
		}


		p.UpdatePacketLength();

		//����Ӧ������
		if( session.Send(&serbuffer) )
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"session.Send error ");
		}

		DEBUG_PRINTF( "ack pkg=======, \n" );
		DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );

		if( 0 == RetCode )
		{

			 _mainSvc->GetBagSvc()->NotifyBag(roleID,lic);
			 //NotifyEquipUpdate(roleID,lic2);
			 NotifyBonus(roleID,Bon2);
			if(equipIndex==8||equipIndex==3)
			{
							sprintf( szSql, "select RoleID,EquipIndex, \
													ItemID \
													from Equip \
													where RoleID = %d \
												 and ItemID<>0 and EquipIndex in( 8,3);", roleID);
					iRet = dbo.QuerySQL(szSql);
					if( iRet < 0 )
					{
						LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
					}

					//ȡ������¼
					if(0==iRet)
					{
							while(dbo.HasRowData())
							{
								iD=dbo.GetIntField(1);
								if(iD==3)
								{
									coatID=dbo.GetIntField(2);
								}
								if(iD==8)
								{
									wepID=dbo.GetIntField(2);
								}
								dbo.NextRow();
								//LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL[%d]     ,[%d] ", adb.wpnItemID,adb.flag);
						 }
					}
			 NotifyAvatarUpdate(roleID,wepID,coatID);
			}
 		}

}

void AvatarSvc::GetRoleSkill(UInt32 roleID,List<ArchvSkill>& sk)
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	ArchvSkill skill;

	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());



	sprintf( szSql, "select SkillID,SkillLev from RoleSkill wher RoleID=%d and SkillID<300;", roleID);

	iRet=dbo.QuerySQL(szSql);

	if(iRet==0)
	{
		while(dbo.HasRowData())
		{
			skill.skillID=dbo.GetIntField(0);
			skill.skillLevel=dbo.GetIntField(1);
			sk.push_back(skill);
			dbo.NextRow();
		}
	}
	else
	{
		return ;
	}
}


//����װ����Ҫ��Ϣ
int AvatarSvc::GetEquipBrief(List<UInt32>& listRoleID , List<ArchvAvatarDescBrief>& ladb)
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;

	List<UInt32>::iterator itor;
	UInt32 iD=0;
	UInt32 EquipIndex,ItemType,MinLevel,ItemID;

	ArchvAvatarDescBrief adb;
	UInt32 currRoleID = 0;

	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());


	//����У��
	if( listRoleID.size() == 0 )
	{
		return -1;
	}

	//��ȡ����
	for( itor = listRoleID.begin(); itor != listRoleID.end(); itor++ )
	{

		currRoleID = *itor;

			sprintf( szSql, "select ProID from Role where RoleID=%d;", currRoleID);
			iRet = dbo.QuerySQL(szSql);
			adb.roleId=currRoleID;
			if(dbo.HasRowData())
			{
				adb.proID = dbo.GetIntField(0);
			}

		sprintf( szSql, "select RoleID,EquipIndex, \
										ItemID \
										from Equip \
										where RoleID = %d \
									 and ItemID<>0 and EquipIndex in( 8,3);", currRoleID);
		iRet = dbo.QuerySQL(szSql);
		if( 1 == iRet )
		{
			//�յ�ʱ��ֵΪO,һ��װ����û�У���ô��

			adb.wpnItemID=0;
			adb.coatID=0;
		}
		if( iRet < 0 )
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			return -1;
		}

		//ȡ������¼
		if(0==iRet)
		{
			adb.roleId=currRoleID;
			while(dbo.HasRowData())
			{
				iD=dbo.GetIntField(1);
				if(iD==8)
				{
					adb.wpnItemID=dbo.GetIntField(2);
				}
				if(iD==3)
				{
					adb.coatID=dbo.GetIntField(2);
				}
				dbo.NextRow();
			}

			//LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL[%d]     ,[%d] ", adb.wpnItemID,adb.flag);
		}
		ladb.push_back(adb);
	}
		return 0;

}
//�жϸ�ְҵ�Ƿ���������������һ����������ְҵ���ͣ��ڶ���������Ʒ�������ݣ�
//����0���ԣ�����1����

//����0����Ҫ����ȫ������1�Ļ�������Ҫ������l�������ص�����Ҫ������������
int AvatarSvc::IfneedToGet(UInt32 RoleID,List<UInt32>& l)
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	UInt32 l1=0;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());
	//��ѯ����װ���Ƿ�������ⷽʽ�ӳɵ�
	sprintf( szSql, "select ItemBonus.BonusAttr,ItemBonus.BonusType from Equip,ItemBonus where RoleID=%d and ItemBonus.ItemID=Equip.ItemID and ItemBonus.BonusType<>1 group by BonusAttr;",RoleID);
	iRet=dbo.QuerySQL(szSql);
	if(iRet==1)
	{
		return 1;
	}
	else if(iRet==0)
		{
			while(dbo.HasRowData())
			{
				l1=dbo.GetIntField(0);
				l.push_back(l1);
				dbo.NextRow();
			}
			return 0;
		}
		else
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"sdsadasdas mysql errror");
			return 1;
		}
}
//��ȡ����װ�������ԣ������ⷽʽ�ӳɵģ�����1��û�з���0
int AvatarSvc::GetJustItemBonus(UInt32 ItemID,List<ArchvroleBonuschange>& Bon,List<UInt32>& l)
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());
	ArchvroleBonuschange bon1;
	int count=0;
	UInt32 l1=0;
	sprintf( szSql, "select BonusAttr,BonusType,BonusValue from ItemBonus where ItemID=%d;",ItemID);
	iRet=dbo.QuerySQL(szSql);
		if(iRet==1)
		{//��ѯ�����ݲ�����
			return 0;
		}
		else if(iRet==0)
		{
				while(dbo.HasRowData())
				{
					if(dbo.GetIntField(1)!=1)
					{
						count=1;
						l1=dbo.GetIntField(0);
						l.push_back(l1);
						dbo.NextRow();
						continue;
					}
					bon1.BonusAttrID=dbo.GetIntField(0);
					bon1.Num=dbo.GetIntField(2);
					Bon.push_back(bon1);
					dbo.NextRow();
				}
					return count;
		}
		else
	 	{
			LOG(LOG_ERROR,__FILE__,__LINE__,"sdsadasdas mysql errror");
			return 0;
	 	}
}
void AvatarSvc::GetRoleBonusoff(UInt32 RoleID,List<ArchvroleBonuschange>& Bon)
{
	List<ArchvroleBonuschange>::iterator itor;
	for( itor=Bon.begin(); itor != Bon.end(); itor++ )
	{
		RoleBonus(RoleID,itor->BonusAttrID,-(itor->Num));
	}

}
void AvatarSvc::GetRoleBonusin(UInt32 RoleID,List<ArchvroleBonuschange>& Bon)
{
	List<ArchvroleBonuschange>::iterator itor;
	for( itor=Bon.begin(); itor != Bon.end(); itor++ )
	{
		RoleBonus(RoleID,itor->BonusAttrID,itor->Num);
	}
}
void AvatarSvc::RoleOtherBonus(UInt32 RoleID,List<UInt32>& l)
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
  ArchvroleBonuschange Bon;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	List<UInt32>::iterator itor;
	l.sort();
	int it=0;
	UInt32 k=0;
	Int32 p=0;
	for( itor=l.begin(); itor != l.end(); itor++ )
	{
				if(it!=*itor)
				{
						Bon.BonusAttrID=*itor;
						Bon.Num=0;
						p=0;
						sprintf( szSql, "select ItemBonus.BonusType,ItemBonus.BonusValue from Equip,ItemBonus where Equip.ItemID=ItemBonus.ItemID,ItemBonus.BonusAttr=%d;",*itor);
						iRet=dbo.QuerySQL(szSql);
						if(iRet==0)
						{
							while(dbo.HasRowData())
							{
								k=dbo.GetIntField(0);
								if(k==1)
								{
									Bon.Num=Bon.Num+dbo.GetIntField(1);
								}
								else
								{
									if(k==2)
									{
										p=p+dbo.GetIntField(1);
									}
								}
								dbo.NextRow();
							}
							//����ӳ�ֵ
						}
						else
						{
							LOG(LOG_ERROR,__FILE__,__LINE__,"sdsadasdas mysql errror");
						}

					it=*itor;
//e					k=RoleGetNewBonus(RoleID,*itor);
					k=Bon.Num-k+RoleGetinfo(RoleID,*itor)*p/100;
					RoleBonus(RoleID,*itor,k);

				}
	}
}


  //װ���;ö����
int AvatarSvc::OnEquipDurabilityLoss(UInt32 roleID,UInt32 lossDurability)
{
	char szSql[1024];
	char szTmp[200];
	char szDest[1024];
	int iRet = 0;
	UInt32 nCnt = 0;

	//ArchvEquipDurability roleDurability;
	//List<ArchvEquipDurability>licDurability;
	//List<ArchvEquipDurability>::iterator iter;

	List < ArchvroleBonuschange >Bon;
	ArchvroleBonuschange newBon;
	List<ArchvroleBonuschange>licNewBon;
	List<UInt32> lBonAttr;
	List<UInt32> licItemID;
	List<UInt32>::iterator iterNew;
	UInt32 nSize = 0;

	ItemCell item;
	List<ItemCell>licItem;
	List<ItemCell>::iterator iter;
	UInt32 durability = 0;

	Connection con;
	DBOperate dbo;


	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	LOG(LOG_ERROR,__FILE__,__LINE__,"OnEquipDurabilityLoss------");
	LOG(LOG_ERROR,__FILE__,__LINE__,"lossDurability[%d]",lossDurability);

	//��ý�ɫ��ǰ��װ��
	sprintf(szSql,"select Equip.EquipIndex,Equip.ItemID,Equip.EntityID,Item.CdTime,Entity.Durability,Entity.BindStatus\
	               from Equip,Entity,Item\
	               where RoleID = %d and Equip.ItemType = 2\
	               and Equip.EntityID = Entity.EntityID and Entity.ItemID = Item.ItemID ;",\
	               roleID);
	iRet = dbo.QuerySQL(szSql);
	if(iRet < 0)
	{
	  LOG(LOG_ERROR,__FILE__,__LINE__,"Query SQL failed ! szSql[%s]",szSql);
	  return -1;
	}
	if(iRet == 1)
	{
	 	LOG(LOG_ERROR,__FILE__,__LINE__,"Query SQL failed ! szSql[%s]",szSql);
	    return 0;
	}

	while(dbo.HasRowData())
	{
	  item.celIndex = dbo.GetIntField(0);
	  item.ItemID = dbo.GetIntField(1);
	  item.EntityID = dbo.GetIntField(2);
	  item.cdTime= dbo.GetIntField(3);
	  item.durability = dbo.GetIntField(4);


	  item.bindStatus = dbo.GetIntField(5);
	  item.num = 1;

	  if(item.durability > 0)
	  {
	  	 licItem.push_back(item);
	  }

	  dbo.NextRow();
	}

	nCnt = licItem.size();
	LOG(LOG_ERROR,__FILE__,__LINE__,"nCnt[%d]",nCnt);
	if(nCnt == 0)
	{
	   return 0;
	}

    sprintf(szDest," ");
    sprintf(szTmp," ");
	for(iter = licItem.begin(); iter != licItem.end(); iter++)
	{
	   if(iter->durability < lossDurability)
	   {
	     iter->durability = 0;
	   }
	   else
	   {
	     iter->durability -= lossDurability;
	   }

	   if(nCnt == 1)
	   {
	     sprintf(szTmp," %d)",iter->EntityID);
	   }
	   else
	   {
	     sprintf(szTmp,"%d,",iter->EntityID);
	   }

      strcat(szDest,szTmp);
	   nCnt--;
	}

	//����װ�����;ö����
	sprintf(szSql,"update Entity set Durability = if(Durability < %d,0,Durability - %d)\
	                where EntityID in (",lossDurability,lossDurability);
    strcat(szSql,szDest);

//	LOG(LOG_ERROR,__FILE__,__LINE__,"szSql[%s]",szSql);
	iRet = dbo.ExceSQL(szSql);
	if(iRet != 0)
	{
      LOG(LOG_ERROR,__FILE__,__LINE__,"Excute SQL error ! szSql[%s]",szSql);
	  return -1;
	}

    //////////////////////--------------------
	/*for(iter = licDurability.begin(); iter != licDurability.end(); iter++)
    {
       LOG(LOG_ERROR,__FILE__,__LINE__,"EquipIndex[%d]----Durability[%d]",iter->EquipIndex,iter->Durability);
    }*/
	//////////////////////-------------------------

	//װ���;öȱ仯֪ͨ
	NotifyEquipUpdate(roleID,licItem);

	//_mainSvc->GetBagSvc()->NotifyEquipDurabilityLoss(roleID,licDurability);


	//�;ö�Ϊ0��װ��ʧȥװ���ӳ�
    for(iter = licItem.begin(); iter != licItem.end(); iter++)
    {
       if(iter->durability == 0)
       {
          licItemID.push_back(iter->ItemID);
		  GetJustItemBonus(iter->ItemID, Bon, lBonAttr);
       }
    }

	// �����ɫװ���;ö�Ϊ0����ʧȥװ���ӳɡ�
	nSize = licItemID.size();
    if(nSize > 0 )
    {
		GetRoleBonusoff(roleID,Bon);

		//��ȡ��ɫ�µļӳ���Ϣ
		RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);

		if(pRole->ID() == 0)
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessGetRole Failed ! role[%d]",roleID);
			return -1;
		}
		for(int i = 1; i <= 16; i++)
		{
			newBon.BonusAttrID = i;
			newBon.Num = RoleGetNewBonus(roleID,i,pRole);
			licNewBon.push_back(newBon);
		}

		LOG(LOG_ERROR,__FILE__,__LINE__,"NotifyBonus-------");
		NotifyBonus(roleID, licNewBon); //��ɫ�ӳ�֪ͨ
    }

    return 0;
}

void AvatarSvc::RoleBonus(UInt32 RoleID,UInt32 BonusAttrID,Int32 num)
{
	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(RoleID);

	switch(BonusAttrID)
	{
		case 1:
			pRole->AddMaxHpBonus(num);
			break;
		case 2:
			pRole->AddMaxMpBonus(num);
			break;
		case 3:
			pRole->AddAttackPowerHighBonus(num);
			break;
		case 4:
			pRole->AddAttackPowerLowBonus(num);
			break;
		case 5:
			pRole->AddDefenceBonus(num);
			break;
		case 6:
			pRole->AddMDefenceBonus(num);
			break;
		case 7:
			pRole->AddCritRateBonus(num);
			break;
		case 8:
			pRole->AddStrengthBonus(num);
			break;
		case 9:
			pRole->AddIntelligenceBonus(num);
			break;
		case 10:
			pRole->AddAgilityBonus(num);
			break;
		case 11:
			pRole->AddMoveSpeedBonus(num);
			break;
		case 12:
			pRole->AddHitRateBonus(num);
			break;
		case 13:
			pRole->AddDodgeRateBonus(num);
			break;
		case 14:
			pRole->AddAttackSpeedBonus(num);
			break;
		case 15:
			pRole->AddHpRegenBonus(num);
			break;
		case 16:
			pRole->AddMpRegenBonus(num);
			break;
			default:
		LOG(LOG_ERROR,__FILE__,__LINE__,"Type erro ");
		break;

	}
}
UInt32 AvatarSvc::RoleGetNewBonus(UInt32 RoleID, UInt32 l, RolePtr& role) {
	assert(role);
	UInt32 v;
	switch (l) {
	case 1:
		v = role->MaxHpBonus();
		break;
	case 2:
		v = role->MaxMpBonus();
		break;
	case 3:
		v = role->AttackPowerHighBonus();
		break;
	case 4:
		v = role->AttackPowerLowBonus();
		break;
	case 5:
		v = role->DefenceBonus();
		break;
	case 6:
		v = role->MDefenceBonus();
		break;
	case 7:
		v = role->CritRateBonus();
		break;
	case 8:
		v = role->StrengthBonus();
		break;
	case 9:
		v = role->IntelligenceBonus();
		break;
	case 10:
		v = role->AgilityBonus();
		break;
	case 11:
		v = role->MovSpeedBonus();
		break;
	case 12:
		v = role->HitRateBonus();
		break;
	case 13:
		v = role->DodgeRateBonus();
		break;
	case 14:
		v = role->AttackSpeedBonus();
		break;
	case 15:
		v = (role->HpRegenBonus()) / 10;
		break;
	case 16:
		v = (role->MpRegenBonus()) / 10;
		break;
	default:
		LOG(LOG_ERROR, __FILE__, __LINE__, "Type erro ");
		break;
	}
	return v;
}
UInt32 AvatarSvc::RoleGetinfo(UInt32 RoleID,UInt32 l)
{
	UInt32 v;

	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(RoleID);

	switch(l)
	{
		case 1:
			v=pRole->MaxHp();
			break;
		case 2:
			v=pRole->MaxMp();
			break;
		case 3:
			v=pRole->AttackPowerHigh();
			break;
		case 4:
			v=pRole->AttackPowerLow();
			break;
		case 5:
			v=pRole->Defence();
			break;
		case 6:
			v=pRole->MDefence();
			break;
		case 7:
			v=pRole->CritRate();
			break;
		case 8:
			v=pRole->Strength();
			break;
		case 9:
			v=pRole->Intelligence();
			break;
		case 10:
			v=pRole->Agility();
			break;
		case 11:
			v=pRole->MoveSpeed();
			break;
		case 12:
			v=pRole->HitRate();
			break;
		case 13:
			v=pRole->DodgeRate();
			break;
		case 14:
			v=pRole->AttackSpeed();
			break;
		case 15:
			v=pRole->MaxHp();
			break;
		case 16:
			v=pRole->MaxMp();
			break;
			default:
		LOG(LOG_ERROR,__FILE__,__LINE__,"Type erro ");
		break;

	}
	return v;

}



//����û�е���
Int32 AvatarSvc::Updatebind(UInt32 RoleID,UInt32 EntityID)//���±�����λ�õİ�״̬
{
	char szSql[128];
	Connection con;
	DBOperate dbo;
	Int32 iRet;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());
	sprintf( szSql, "update Entity set BindStatus=1 where EntityID=%d;",EntityID);
	iRet=dbo.ExceSQL(szSql);
	if(iRet==0)
	{
		return 0;
	}
	else
	{
		return -1;
	}


}


//�������˲��ԣ��������ã�ɾ���������������Ӱ�� 507

void  AvatarSvc::ProcessTestByWangLian(Session& session,Packet& packet)
{
    UInt32 roleID = packet.RoleID;
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);

	UInt32 loss = 0;
	int iRet = 0;

	//���л���
	Serializer s(packet.GetBuffer());
	s>>loss;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

   LOG(LOG_ERROR,__FILE__,__LINE__,"ProcessTestByWangLian----");
  	iRet = OnEquipDurabilityLoss(roleID, loss);
	if(iRet)
    {
      RetCode = ERR_APP_OP;
	  LOG(LOG_ERROR,__FILE__,__LINE__,"OnEquipDurabilityLoss Failed !--");
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
		//s<<celIndex;
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


/*
//����ȫ����ȡ�����ݣ��ɹ�����0�����ɹ�����1
int AvatarSvc::GetItemBonus(UInt32 RoleID,List<UInt32>& l,List<ArchvroleBonuschange>& Bon,List<ArchvroleEuip>& Ebon)
{
	char szSql[1024];
	char szTmp[128];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	List<UInt32>::iterator itor;
	ArchvroleEuip Ebon1;
	ArchvroleBonuschange bon1;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());
	UInt32 Count=0;
	UInt32 Count1=0;
	sprintf( szSql, "select Equip.EquipIndex,Equip.ItemID,ItemBonus.BonusAttr,ItemBonus.BonusType,ItemBonus.BonusValue from Equip,ItemBonus where RoleID=%d and ItemBonus.ItemID=Equip.ItemID and BonusAttr in(",RoleID);
	for( itor=l.begin(); itor != l.end(); itor++ )
	{
		sprintf( szTmp, "%d, ", *itor );
		strcat( szSql, szTmp );
	}
	sprintf( szTmp, "-1) order by BonusAttr;" );
	strcat( szSql, szTmp );
	//sprintf( szSql, "select BonusAttr,BonusType,BonusValue from ItemBonus where ItemID =",);
	iRet=dbo.QuerySQL(szSql);
	if(iRet==1)
	{//��ѯ�����ݲ�����
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL NOT found[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return 1;
	}
	if(iRet<0)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return 1;
	}
	if(iRet==0)
	{
		if(dbo.HasRowData())
		{
			Count=dbo.GetIntField(3);
			if(Count==1)
			{
				bon1.BonusAttrID=dbo.GetIntField(2);
				bon1.Num=dbo.GetIntField(4);
			}
			else
			{
				Ebon1.BonusAttrID=dbo.GetIntField(3);
				Ebon1.Type=Count;
				Ebon1.Value=dbo.GetIntField(4);
			}
			dbo.NextRow();
		}
		while(dbo.HasRowData())
		{
			Count1=dbo.GetIntField(2);
			Count=dbo.GetIntField(3);
			if(Count==1)
			{
				if(bon1.BonusAttrID==Count1)
				{
					bon1.Num+=dbo.GetIntField(4);	//����һ��ֱ���ۼ�
				}
				else
				{
					Bon.push_back(bon1);
					bon1.BonusAttrID=dbo.GetIntField(2);
					bon1.Num=dbo.GetIntField(4);
				}
			}
			else
			{
				if(Ebon1.BonusAttrID==Count1)
				{
					Ebon1.Value+=dbo.GetIntField(4);
				}
				else
				{
					Ebon.push_back(Ebon1);
					Ebon1.BonusAttrID=Count1;
					Ebon1.Type=Count;
					Ebon.push_back(Ebon1);
				}
			}
			dbo.NextRow();
		}
		return 0;
	}
}

*/

//�������еĿ�װ��ְҵ����һ����������Ʒ�������ݡ�
//�ڶ��������ǿ���װ����Ʒ��ְҵ���ͷ���

/*
//�ж��Ƿ��Ѿ����ҹ�
Int32 AvatarSvc::CheckifhavaGet(UInt32 l,List <ArchvroleBonuschange>& bon)
{
	List<ArchvroleBonuschange>::iterator itor;
	for(itor=bon.begin(); itor != bon.end(); itor++)
	{
		if(l==itor->BonusAttrID )
		{
			return 1;
			break;
		}
	}
	return 0;
}
*/


/*
Int32 AvatarSvc::GetBonusList(UInt32 RoleID,UInt32 l,List<ArchvroleBonuschange>& bon)
{
		ArchvroleBonuschange i1;
			if(CheckifhavaGet(l,bon)==0)
			{
					i1.BonusAttrID=l;
					i1.Num=RoleGetNewBonus(RoleID,l);
					bon.push_back(i1);
					if(l==8)
					{
					//����
						if(CheckifhavaGet(1,bon)==0)
						{
							i1.BonusAttrID=1;
							i1.Num=RoleGetNewBonus(RoleID,1);
							bon.push_back(i1);
						}
						if(CheckifhavaGet(3,bon)==0)
						{
							i1.BonusAttrID=3;
							i1.Num=RoleGetNewBonus(RoleID,3);
							bon.push_back(i1);
							i1.BonusAttrID=4;
							i1.Num=RoleGetNewBonus(RoleID,4);
							bon.push_back(i1);
						}
						return 1;
					}
					else if(l==9)
					{
					//����
						if(CheckifhavaGet(2,bon)==0)
						{
							i1.BonusAttrID=2;
							i1.Num=RoleGetNewBonus(RoleID,2);
							bon.push_back(i1);
						}
						if(CheckifhavaGet(3,bon)==0)
						{
							i1.BonusAttrID=3;
							i1.Num=RoleGetNewBonus(RoleID,3);
							bon.push_back(i1);
							i1.BonusAttrID=4;
							i1.Num=RoleGetNewBonus(RoleID,4);
							bon.push_back(i1);
						}

						return 1;
					}
					else if(l==10)
					{
					//����
						if(CheckifhavaGet(5,bon)==0)
						{
							i1.BonusAttrID=5;
							i1.Num=RoleGetNewBonus(RoleID,5);
							bon.push_back(i1);
						}
						if(CheckifhavaGet(3,bon)==0)
						{
							i1.BonusAttrID=3;
							i1.Num=RoleGetNewBonus(RoleID,3);
							bon.push_back(i1);
							i1.BonusAttrID=4;
							i1.Num=RoleGetNewBonus(RoleID,4);
							bon.push_back(i1);
						}

						return 1;
					}
				}
	return 0;
}
*/
/*
UInt16  AvatarSvc::EquipGetoff(UInt32 roleID,Byte EquipIndex)
{//װ��δָ��λ��ж�µĴ���,

}
*/


/*
////MsgType:0505 ���ŵ�װ��λ�û���,Ŀǰû�������ַ�ʽ
void AvatarSvc::ProcessRolesEquipChange(Session& session,Packet& packet)
{
		UInt32	RetCode = 0;
		DataBuffer	serbuffer(1024);
		char szSql[1024];
		Connection con;
		DBOperate dbo;
		int iRet = 0;
		Byte equipIndex1,equipIndex2;
		UInt16 celIndex;
		//List<UInt32> listRoleID;
		List<Byte>::iterator itor;
	//	char szCat[1024];
		//char szTmp[128];
		UInt32 roleID = packet.RoleID;
		Byte flag=0;
		Byte equippos;
		List<Byte> eq1,eq2;
		con = _cp->GetConnection();
		dbo.SetHandle(con.GetHandle());
		UInt32 count;
		UInt16 count1;

		//���л���
		Serializer s(packet.GetBuffer());
		s>>equipIndex1>>equipIndex2;


		if( s.GetErrorCode()!= 0 )
		{
			RetCode = ERR_SYSTEM_SERERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
			goto EndOf_Process;
		}
		sprintf( szSql, "select EquipPos from Item where ItemID in (select ItemID from Equip where RoleID= %d and ItemID=%D and EquipIndex in (%d,%d));", roleID,equipIndex1,equipIndex2);
		iRet = dbo.QuerySQL(szSql);

		if( 1 == iRet )
		{
			RetCode = ERR_SYSTEM_DBNORECORD;
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
			goto EndOf_Process;
		}
 		if(0==iRet)
 		{
 			count1=dbo.RowNum();
 			count=dbo.GetIntField(0);
 			while(count%100<=13&&count>0)
			{
				equippos=(Byte)(count%100);
				eq1.push_back(equippos);
				count=count/100;
			}
			dbo.NextRow();
			if(count1==2)
				{
				count=dbo.GetIntField(0);
				while(count%100<=13&&count>0)
				{
					equippos=(Byte)(count%100);
					eq2.push_back(equippos);
					count=count/100;
				}
				}

 			//ֻ�ǰ�1���µ�2
 				for( itor=eq1.begin(); itor != eq1.end(); itor++ )
				{
					if(equipIndex2==*itor)
					{
						flag=1;
			//Ŀ���ܷ�
					}

				}
				if(flag==0)
				{
				//����ʧ��
					RetCode = ERR_SYSTEM_SERERROR;
					LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
					goto EndOf_Process;
				}
				sprintf( szSql, "updata Equip set EquipIndex=100 where RoleID=%d and EquipIndex=%d;",roleID, equipIndex2);
				iRet = dbo.ExceSQL(szSql);
				if(iRet==1)
				{
					RetCode = ERR_SYSTEM_DBNORECORD;
					LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s] " , szSql);
					goto EndOf_Process;
				}
				sprintf( szSql, "updata Equip set EquipIndex=%d where RoleID=%d and EquipIndex=%d;", equipIndex2,roleID,equipIndex1);
				iRet = dbo.ExceSQL(szSql);
				if(iRet==1)
				{
					RetCode = ERR_SYSTEM_DBNORECORD;
					LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s] " , szSql);
					goto EndOf_Process;
				}
				sprintf( szSql, "updata Equip set EquipIndex=%d where RoleID=%d and EquipIndex=100;", equipIndex1,roleID);
				iRet = dbo.ExceSQL(szSql);

				LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
				if(count1==1)
				{

					goto EndOf_Process;
 				}

 		}


		//�ж�Ŀ��λ���Ƿ����ܷŵ�λ��
		for( itor=eq2.begin(); itor != eq2.end(); itor++ )
		{
			if(equipIndex1==*itor)
			{
				flag=0;
			//2Ŀ���ܷ�equipIndex1�ܷ�Ŀ��λ��
			}
		}
		//sprintf( szSql, "select EquipPos from Item where ItemID in (select ItemID from Equip where RoleID= %d and EquipIndex in (%d,%d));", roleID,equipIndex1,equipIndex2);
		//iRet = dbo.ExceSQL(szSql);
		//��1�ŵ�2


		if(flag==0)
		{
			////2Ŀ���ܷ�equipIndex1�ܷ�Ŀ��λ��
			goto EndOf_Process;
		}
		else
		{
			celIndex=EquipGetoff(roleID,equipIndex2);
			////2Ŀ�겻�ܷ�equipIndex1�ܷ�Ŀ��λ��

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
		//s<<eq;
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
*/



