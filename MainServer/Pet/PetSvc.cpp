#include "MainSvc.h"
#include "DBOperate.h"
#include "PetSvc.h"
#include "ArchvPet.h"
#include "ArchvPK.h"
#include "ArchvBagItemCell.h"
#include "ArchvBagItemCell.h"
#include "ArchvRole.h"
#include "CoreData.h"
#include "Pet.h"
#include "../Bag/BagSvc.h"
#include "../Task/TaskSvc.h"
#include "../Avatar/AvatarSvc.h"
#include "../Avatar/ArchvAvatar.h"

PetSvc::PetSvc(void *service, ConnectionPool * cp)
{
	_mainSvc = (MainSvc*)(service);
	_cp = cp;

}

PetSvc::~PetSvc()
{
	//
}

//���ݰ���Ϣ
void PetSvc::OnProcessPacket(Session& session,Packet& packet)
{
	DEBUG_PRINTF1( "C_S req pkg-------MsgType[%d] \n", packet.MsgType );
	DEBUG_SHOWHEX( packet.GetBuffer()->GetReadPtr()-PACKET_HEADER_LENGTH, packet.GetBuffer()->GetDataSize()+PACKET_HEADER_LENGTH, 0, __FILE__, __LINE__ );

	switch(packet.MsgType)
	{
			case 1002://��ѯ������ϸ��Ϣ
			ProcessPetDetailInfo(session,packet);
			break;

			case 1005://����ж��װ�� 
			ProcessPetEquipGetOff(session,packet);
			break;

			case 1006://���ﴩ��װ��
			ProcessPetEquipPutOn(session,packet);
			break;

			case 1007://���ﵱǰ���� 
			ProcessPetIsUse(session, packet);
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
void PetSvc::ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode)
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

//[msgtype:1002] ��ѯ������ϸ��Ϣ
void PetSvc::ProcessPetDetailInfo(Session& session, Packet& packet)
{
	char szSql[1024];
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(8196);
	UInt32 roleID = packet.RoleID;
	UInt32 petID=0;
	int iRet;
	int hasEquity = 1;
	char name[64+1]={0};
	List<PetEquip> PetEquipList;
	PetInfo PetInfoObject;
//	PetBonus PetBonusObjects;
//	PetEquip PetEquipobject;
	Connection con;
	DBOperate dbo;
	Serializer s(packet.GetBuffer());
	s>>petID;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}
	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());
	//��ѯ������Ϣ	
	sprintf(szSql, "select Exp, MaxExp, HP, MP, MaxHP, MaxMP, AttackPowerHigh,AttackPowerLow,Defence, MDefence, CritRate, AddPoint, \
	Strength, Intelligence, Agility, MoveSpeed, HitRate, DodgeRate, AttackSpeed, HPRegen, MPRegen, Level, PetName from Pet where RoleID=%d \
									and PetID=%d;",roleID,petID);
	iRet=dbo.QuerySQL(szSql);
	if(iRet==0 )
	{
		while(dbo.HasRowData())
		{	
			PetInfoObject.Exp = dbo.GetIntField(0);
			PetInfoObject.MaxExp = dbo.GetIntField(1);
			PetInfoObject.HP = dbo.GetIntField(2);
			PetInfoObject.MP = dbo.GetIntField(3);
			PetInfoObject.MaxHP= dbo.GetIntField(4);
			PetInfoObject.MaxMP= dbo.GetIntField(5);
			PetInfoObject.AttackPowerHigh = dbo.GetIntField(6);
			PetInfoObject.AttackPowerLow = dbo.GetIntField(7);
			PetInfoObject.Defence = dbo.GetIntField(8);
			PetInfoObject.MDefence = dbo.GetIntField(9);
			PetInfoObject.CritRate = dbo.GetIntField(10);
			PetInfoObject.AddPoint = dbo.GetIntField(11);
			PetInfoObject.Strength = dbo.GetIntField(12);
			PetInfoObject.Intelligence = dbo.GetIntField(13);
			PetInfoObject.Agility = dbo.GetIntField(14);
			PetInfoObject.MoveSpeed =	dbo.GetIntField(15);
			PetInfoObject.HitRate = dbo.GetIntField(16);
			PetInfoObject.DodgeRate = dbo.GetIntField(17);
			PetInfoObject.AttackSpeed = dbo.GetIntField(18);
			PetInfoObject.HPRegen  = dbo.GetIntField(19);
			PetInfoObject.MPRegen = dbo.GetIntField(20);
			PetInfoObject.Level  = dbo.GetIntField(21);
			strcpy(name,dbo.GetStringField(22));
			PetInfoObject.PetName  = name;

			dbo.NextRow();
		}
	}
	if(iRet == 1)
	{
	  RetCode = ERR_SYSTEM_DBNORECORD;
		LOG(LOG_ERROR,__FILE__,__LINE__,"Role[%d] has no pet[%d] ! " ,roleID,petID);
		goto EndOf_Process;
	}
	if(iRet < 0)
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found or	errro! ,szSql[%s] " , szSql);
		goto EndOf_Process;
	}
	LOG(LOG_DEBUG,__FILE__,__LINE__,"================ PetInfo ==========");
	LOG(LOG_DEBUG,__FILE__,__LINE__,"Exp[%d]",PetInfoObject.Exp);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"MaxExp[%d]",PetInfoObject.MaxExp);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"HP[%d] ",PetInfoObject.HP);
  LOG(LOG_DEBUG,__FILE__,__LINE__,"MP[%d] ",PetInfoObject.MP);
  LOG(LOG_DEBUG,__FILE__,__LINE__,"MaxHP[%d] ",PetInfoObject.MaxHP);
  LOG(LOG_DEBUG,__FILE__,__LINE__,"MaxMP[%d] ",PetInfoObject.MaxMP);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"AttackPowerHigh[%d] ",PetInfoObject.AttackPowerHigh);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"Defence[%d] ",PetInfoObject.Defence);

	LOG(LOG_DEBUG,__FILE__,__LINE__,"MDefence[%d]",PetInfoObject.MDefence);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"CritRate[%d]",PetInfoObject.CritRate);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"AddPoint[%d] ",PetInfoObject.AddPoint);
  LOG(LOG_DEBUG,__FILE__,__LINE__,"Strength[%d] ",PetInfoObject.Strength);
  LOG(LOG_DEBUG,__FILE__,__LINE__,"Intelligence[%d] ",PetInfoObject.Intelligence);
  LOG(LOG_DEBUG,__FILE__,__LINE__,"Agility[%d] ",PetInfoObject.Agility);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"MoveSpeed[%d] ",PetInfoObject.MoveSpeed);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"HitRate[%d] ",PetInfoObject.HitRate);


	LOG(LOG_DEBUG,__FILE__,__LINE__,"DodgeRate[%d] ",PetInfoObject.DodgeRate);
  LOG(LOG_DEBUG,__FILE__,__LINE__,"AttackSpeed[%d] ",PetInfoObject.AttackSpeed);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"HPRegen[%d] ",PetInfoObject.HPRegen);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"MPRegen[%d] ",PetInfoObject.MPRegen);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"Level[%d] ",PetInfoObject.Level);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"PetName: [%s] ",PetInfoObject.PetName.c_str());

/*
	
  //��ѯ�ӳ���Ϣ
 	sprintf(szSql, "select MPRegen,HPRegen,MaxMP, MaxHP, MovSpeed, Agility, \
		              Intelligence, Strength, AttackPowerHigh, AttackSpeed, \
		              Defence, MDefence,CritRate,HitRate, DodgeRae  \
		              from PetBonus \
		              where PetID=%d ;",petID);
		              
	iRet=dbo.QuerySQL(szSql);
	if(iRet==0 )
	{
		while(dbo.HasRowData())
		{
			PetBonusObjects.MPRegen=dbo.GetIntField(0);
			PetBonusObjects.HPRegen=dbo.GetIntField(1);
			PetBonusObjects.MaxHP=dbo.GetIntField(2);
			PetBonusObjects.MaxMP = dbo.GetIntField(3);
			PetBonusObjects.MoveSpeed = dbo.GetIntField(4);
			PetBonusObjects.Agility = dbo.GetIntField(5);
			PetBonusObjects.Intelligence = dbo.GetIntField(6);
			PetBonusObjects.Strength = dbo.GetIntField(7);
			PetBonusObjects.attackPower= dbo.GetIntField(8);
			PetBonusObjects.AttackSpeed= dbo.GetIntField(9);
			PetBonusObjects.Defence = dbo.GetIntField(10);
			PetBonusObjects.MDefence = dbo.GetIntField(11);
			PetBonusObjects.CritRate = dbo.GetIntField(12);
			PetBonusObjects.HitRate = dbo.GetIntField(13);
			PetBonusObjects.DodgeRate = dbo.GetIntField(14);
			
			dbo.NextRow();
		}
	}
	if(iRet == 1)
	{
	  RetCode = ERR_SYSTEM_DBNORECORD;
		LOG(LOG_ERROR,__FILE__,__LINE__,"Role[%d] pet[%d] PetBonus not exist !" ,roleID,petID);
		goto EndOf_Process;
	}
	if(iRet < 0)
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found or	errro! ,szSql[%s] " , szSql);
		goto EndOf_Process;
	}

	LOG(LOG_DEBUG,__FILE__,__LINE__,"================ PetBonus ==========");
	LOG(LOG_DEBUG,__FILE__,__LINE__,"MPRegen[%d]",PetBonusObjects.MPRegen);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"HPRegen[%d]",PetBonusObjects.HPRegen);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"MaxHP[%d]",PetBonusObjects.MaxHP);
  LOG(LOG_DEBUG,__FILE__,__LINE__,"MaxMP[%d]",PetBonusObjects.MaxMP);
  LOG(LOG_DEBUG,__FILE__,__LINE__,"MoveSpeed[%d]",PetBonusObjects.MoveSpeed);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"Agility[%d]",PetBonusObjects.Agility);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"Intelligence[%d]",PetBonusObjects.Intelligence);
  LOG(LOG_DEBUG,__FILE__,__LINE__,"Strength[%d]",PetBonusObjects.Strength);
  
	LOG(LOG_DEBUG,__FILE__,__LINE__,"attackPower[%d]",PetBonusObjects.attackPower);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"AttackSpeed[%d]",PetBonusObjects.AttackSpeed);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"Defence[%d]",PetBonusObjects.Defence);
  LOG(LOG_DEBUG,__FILE__,__LINE__,"MDefence[%d]",PetBonusObjects.MDefence);
  LOG(LOG_DEBUG,__FILE__,__LINE__,"CritRate[%d]",PetBonusObjects.CritRate);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"HitRate[%d]",PetBonusObjects.HitRate);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"DodgeRate[%d]",PetBonusObjects.DodgeRate);


  //װ������
	sprintf( szSql, "select PetEquip.EquipIndex, PetEquip.ItemID,PetEquip.EntityID,Entity.Durability, \
	Entity.BindStatus from PetEquip,Entity where PetEquip.EntityID=Entity.EntityID and PetEquip.ItemID <> 0 and PetID=%d;",petID);
	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		//to do: �߼���Ҫ�ı䣬�������û��װ��
		//RetCode = ERR_SYSTEM_DBNORECORD;
		hasEquity = 0;
		LOG(LOG_DEBUG,__FILE__,__LINE__,"RoleID[%d] PetID[%d] EquipAttribute not existe !" ,roleID,petID);
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
		PetEquipobject.EquipIndex = dbo.GetIntField(0);
		PetEquipobject.ItemID = dbo.GetIntField(1);
		PetEquipobject.EntityID= dbo.GetIntField(2);
		PetEquipobject.Durability=dbo.GetIntField(3);
		PetEquipobject.BindState=dbo.GetIntField(4);
		PetEquipList.push_back(PetEquipobject);


	LOG(LOG_DEBUG,__FILE__,__LINE__,"================ PetEquip ==========");
	LOG(LOG_DEBUG,__FILE__,__LINE__,"EquipIndex[%d]",PetEquipobject.EquipIndex);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"ItemID[%d]",PetEquipobject.ItemID);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"EntityID[%d]",PetEquipobject.EntityID);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"Durability[%d]",PetEquipobject.Durability);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"BindState[%d]",PetEquipobject.BindState);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"===============  ======================");
	
		//��¼����һ����¼
		dbo.NextRow();
	} 

*/	

EndOf_Process:
	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();
	p.CopyHeader(packet);
	p.Direction = DIRECT_C_S_RESP;
	p.PackHeader();

	LOG(LOG_DEBUG,__FILE__,__LINE__,"RetCode[%d]",RetCode);
	s<<RetCode;
	s<<petID;
	UInt16 numEquity = PetEquipList.size();
	if (0 == RetCode) {
		//RetCode Ϊ0 �Ż᷵�ذ���ʣ������
		s << PetInfoObject;
//		s << PetBonusObjects;
//		s << PetEquipList;
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

//[mystype:1006] ���ﴩ��װ��
void PetSvc::ProcessPetEquipPutOn(Session& session,Packet& packet)
{
	DataBuffer serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	
	List<ItemCell> licBag; //����֪ͨ����
	ItemCell licBagItem;// ����֪ͨ����
	UInt32 isNotifyBag = 0;//�Ƿ���Ҫ����֪ͨ

	List<ItemCell>licPetEquip;
	ItemCell licPetEquipItem;

	Byte equipIndex = 0;
	UInt16 EquipIndex = 0;
	UInt32 petID = 0;
	UInt16 celIndex;
	
	ArchvroleBonuschange il;
	UInt32 roleID = packet.RoleID;

	List <ArchvroleBonuschange> Bon1,Bon2;
	Pet pet;

	UInt32	RetCode = 0, itemid = 0,entityid = 0;
	UInt16 durability = 0;
	Byte itemtype = 0, bagCelltype = 0,bagItemtype = 0,bindState = 0;
	ItemCell bagItemCell;
	int hasEquity = 1; //Ĭ����װ��
	
	//���л���
	Serializer s(packet.GetBuffer());
	s>>petID>>celIndex>>equipIndex;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//��ѯ�����и�λ���Ƿ���װ��
	sprintf(szSql,"select CellType,ItemType from Bag \
								 where RoleID = %d and CellIndex = %d;",roleID,celIndex);
	iRet = dbo.QuerySQL(szSql);
	if(iRet == 1)
	{
	  RetCode = ERR_SYSTEM_DBNORECORD;
		LOG(LOG_DEBUG,__FILE__,__LINE__,"role[%d] Bag CellIndex[%d] has no item !" ,roleID,celIndex);
		goto EndOf_Process;
	}
	if(iRet < 0)
	{
	  RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"DB op error ! " );
		goto EndOf_Process;
	}

  bagCelltype = dbo.GetIntField(0);
  bagItemtype = dbo.GetIntField(1);
  EquipIndex = equipIndex;
	if(bagCelltype != 1)  //����װ��
	{
	  RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"bagCellType is [%d]",bagCelltype);
		goto EndOf_Process;
	}

  LOG(LOG_DEBUG,__FILE__,__LINE__,"bagCelltype[%d]",bagCelltype);


  
	//��ʼ������
	iRet = pet.DB2CacheBonus(petID,_cp);
	if(iRet)
	{
		RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"InitPetCache  error!" );
		goto EndOf_Process;
	}

	LOG(LOG_DEBUG,__FILE__,__LINE__,"iRet[%d]",iRet);
	
	//Ŀǰֻ֧��ͷ1������2����3�ͽ�10
	if (equipIndex==1 || equipIndex==2 || equipIndex == 3 || equipIndex == 10)
	{
	    //��ȡ����ָ��λ��װ������Ϣ
		sprintf(szSql, "select ItemID,ItemType,EntityID from PetEquip where PetID=%d and EquipIndex=%d", petID, equipIndex);
		iRet=dbo.QuerySQL(szSql);
		if(iRet < 0)
		{
			RetCode = ERR_SYSTEM_DBERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"Execute sql not found ! szSql[%s]",szSql);
			goto EndOf_Process;
		}
		if(iRet == 1)
		{
		  //RetCode = ERR_SYSTEM_SERERROR;
			hasEquity = 1;
			LOG(LOG_DEBUG,__FILE__,__LINE__,"the pet has not one equip in %d!", equipIndex );
		}
		while(dbo.HasRowData())
		{
			itemid = dbo.GetIntField(0);
			itemtype = dbo.GetIntField(1);
			entityid = dbo.GetIntField(2);
			dbo.NextRow();
		}


		//��ѯ��Ʒ���;öȺͰ�״̬
		if(itemid != 0)               //��������װ��
		{
				sprintf(szSql,"select Durability,BindStatus from Entity\
											where ItemID = %d and EntityID = %d;",itemid,entityid);
				iRet = dbo.QuerySQL(szSql);
				if(iRet < 0)
				{ 
				RetCode = ERR_SYSTEM_DBERROR;
				LOG(LOG_ERROR,__FILE__,__LINE__,"Excute sql not found szSql[%s]" ,szSql);
				goto EndOf_Process;

				}
				if(iRet == 1)
				{
				RetCode = ERR_SYSTEM_DBNORECORD;
				LOG(LOG_ERROR,__FILE__,__LINE__,"role[%d] itemID[%d] entityID[%d] not existe !" ,roleID,itemid,entityid);
				goto EndOf_Process;
				}

				durability = dbo.GetIntField(0);
				bindState = dbo.GetIntField(1);
		}
		

		//��ȡ������λ��װ������Ϣ
		iRet = _mainSvc->GetBagSvc()->RoleSelectCell(roleID, celIndex,bagItemCell);
		if(iRet)
		{
			RetCode = ERR_APP_OP;
			LOG(LOG_ERROR,__FILE__,__LINE__,"RoleSelectCell error ! RoleID[%d]" ,roleID);
			goto EndOf_Process;
		}
		
		if (itemid !=0)  //��װ������ж�³���װ��
		{
				sprintf(
					szSql,
					"update Bag set CellType = 1,	ItemType = %d,EntityID = %d,Num = 1 where RoleID = %d and CellIndex = %d;",
					itemtype, entityid, roleID, celIndex);
			iRet = dbo.ExceSQL(szSql);
			if (iRet != 0) {
				RetCode = ERR_SYSTEM_DBERROR;
				LOG(LOG_ERROR, __FILE__, __LINE__,
						"ExceSQL data not found,szSql[%s] ", szSql);
				goto EndOf_Process;
			}

				//�������Լӳɷ����仯(����)
				GetPetItemBonus(itemid,Bon1);
				GetPetBonusoff(petID,Bon1,pet);

				//����Ʒ�ƶ�������
				isNotifyBag = 1; 
		}

		//���³���װ����
		if (hasEquity) //
		{
			sprintf(
					szSql,
					"update PetEquip set ItemType=%d, ItemID=%d,EntityID=%d where PetID=%d and EquipIndex=%d;",
					bagItemtype, bagItemCell.ItemID, bagItemCell.EntityID,
					petID, equipIndex);
		} else {
			sprintf(
					szSql,
					"insert PetEquip(PetID,EquipIndex,ItemType,ItemID,EntityID) value(%d,%d,%d,%d,%d)",
					petID, equipIndex, bagItemtype, bagItemCell.ItemID,
					bagItemCell.EntityID);
		}

		iRet = dbo.ExceSQL(szSql);
		if(iRet != 0)
		{
			RetCode = ERR_SYSTEM_DBERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s] " , szSql);
			goto EndOf_Process;
		}
	
		//�ɹ��Ժ����װ��
		if(itemid == 0)
		{
			sprintf( szSql, "delete from Bag where CellIndex=%d and RoleID=%d;",celIndex,roleID);
			iRet = dbo.ExceSQL(szSql);
			if(iRet < 0)
			{
			   RetCode = ERR_SYSTEM_DBERROR;
				 LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s] " , szSql);
				 goto EndOf_Process;
			}
			if(iRet == 1)
			{
			   RetCode = ERR_SYSTEM_DBNORECORD;
				 LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s] " , szSql);
				 goto EndOf_Process;
			}
		}

    if(isNotifyBag)
    {
	    licBagItem.celIndex=celIndex;
	    licBagItem.ItemID=itemid;
			licBagItem.EntityID= entityid;
			licBagItem.cdTime = 0;
			licBagItem.num=1;
			licBagItem.durability = durability;
			licBagItem.bindStatus = bindState;
			
			licBag.push_back(licBagItem);
    }
	  
		licPetEquipItem.celIndex= EquipIndex; 
		licPetEquipItem.ItemID=bagItemCell.ItemID;
		licPetEquipItem.EntityID= bagItemCell.EntityID;
		licPetEquipItem.cdTime = 0;
		licPetEquipItem.num=1;
		licPetEquipItem.durability = bagItemCell.durability;
		licPetEquipItem.bindStatus = bagItemCell.bindStatus;
		
		licPetEquip.push_back(licPetEquipItem);

	}
	else
	{
		RetCode = ERR_APP_DATA;
 		LOG(LOG_ERROR,__FILE__,__LINE__,"the equipIndex is not exit! ");
		goto EndOf_Process;
	}
	
	//�������Ա仯,д��DB
	Bon1.clear(); //����ϴε�����
	GetPetItemBonus(bagItemCell.ItemID,Bon1);
	GetPetBonusin(petID,Bon1,pet);

	//��ȡpet�����ԣ�S-C��ǰ̨��ʾ
	for(int i=1;i<=16;i++)
	{
		il.BonusAttrID=i;
		il.Num=PetGetNewBonus( pet,i);
		Bon2.push_back(il);
		LOG(LOG_DEBUG,__FILE__,__LINE__,"Pet XXXXXXXXX ID=%d, Num=%d! ", i, il.Num);
	}

  EndOf_Process:

	//��Ӧ������
	Packet p(&serbuffer);
	s.SetDataBuffer(&serbuffer);
	serbuffer.Reset();
	p.CopyHeader(packet);
	p.Direction = DIRECT_C_S_RESP;
	p.PackHeader();	

	LOG(LOG_DEBUG,__FILE__,__LINE__,"RetCode[%d]",RetCode);
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

  if((0 == RetCode) && isNotifyBag)
  {
		//����֪ͨ
		LOG(LOG_DEBUG,__FILE__,__LINE__,"Pet PutOn Equip ===== ");
    _mainSvc->GetBagSvc()->NotifyBag(roleID,licBag);
  }
	if( 0 == RetCode )
	{
		LOG(LOG_DEBUG,__FILE__,__LINE__,"Pet PutOn Equip ===== ");
		 pet.Cache2Bonus();//����DB
		 NotifyPetEquipChange(roleID,licPetEquip);
		 NotifyPetBonus(roleID,Bon2);	
	}
						
}

//[mystype:1005]����ж��װ��
void PetSvc::ProcessPetEquipGetOff(Session& session,Packet& packet)
{
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	UInt32 petID;
	List<ItemCell> lic;
	ItemCell lic1;//������ȡ�Ķ���
	List<ItemCell> lic2;
	ItemCell lic22;//������ȡ�Ķ���
	
	UInt16 celIndex = 0;
	Byte equipIndex = 0;
	UInt16 EquipIndex = 0;
	
	ArchvroleBonuschange il;
	UInt32 roleID = packet.RoleID;
	List <ArchvroleBonuschange> Bon1,Bon2;
	UInt16 ItemType = 0;
	UInt32 EntityID = 0,ItemID = 0;
	
	Pet pet;
	
	//���л���
	Serializer s(packet.GetBuffer());
	s>>petID>>equipIndex;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//�жϸ�λ���Ƿ���װ��
	sprintf(szSql,"select ItemID,ItemType,EntityID from PetEquip\
									where PetID = %d and EquipIndex = %d;",petID,equipIndex);
	iRet = dbo.QuerySQL(szSql);
	if(iRet < 0)
	{
	  RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"DB op error !");
		goto EndOf_Process;
	}
	if(iRet == 1)
	{
	  RetCode = ERR_SYSTEM_DBNORECORD;
		LOG(LOG_ERROR,__FILE__,__LINE__,"role[%d] pet[%d] exquipIndex[%d] has no Equip !",roleID,petID,equipIndex);
		goto EndOf_Process;
	}

	ItemID=dbo.GetIntField(0);
	ItemType=dbo.GetIntField(1);	
	EntityID=dbo.GetIntField(2);


  LOG(LOG_DEBUG,__FILE__,__LINE__,"====== GetOff to bag =====");
	
	LOG(LOG_DEBUG,__FILE__,__LINE__,"ItemID[%d]",ItemID);
	
	LOG(LOG_DEBUG,__FILE__,__LINE__,"ItemType[%d]",ItemType);
	LOG(LOG_DEBUG,__FILE__,__LINE__,"EntityID[%d]",EntityID);
	
	if(ItemID == 0)
	{
	  RetCode = ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"	Pet equipIndex[%d] has no equip !",equipIndex);
		goto EndOf_Process;
	}

	EquipIndex = equipIndex;
	
  //�жϱ����Ƿ��пռ�
  celIndex = _mainSvc->GetBagSvc()->IfhascellIndex(roleID);
  if(0 == celIndex)
  {
    RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"Bag has no cell ! RoleID[%d]",roleID);
		goto EndOf_Process;
  }
  
	//��ʼ������
	iRet = pet.DB2CacheBonus( petID,_cp);
	if(iRet)
	{
		RetCode = ERR_APP_OP;
		LOG(LOG_ERROR,__FILE__,__LINE__,"the ItemID=0" );
		goto EndOf_Process;
	}		
	
	//װ���Ƶ�����
	sprintf( szSql, "insert into \
	             Bag(RoleID,CellIndex,CellType,ItemType,ItemID,EntityID,Num)\
	             values(%d,%d,1,%d,%d,%d,1);",\
	                 roleID,celIndex,ItemType,ItemID,EntityID);
	iRet = dbo.ExceSQL(szSql);
	if(iRet != 0)
	{
		RetCode = ERR_SYSTEM_DBNORECORD;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s] " , szSql);
		goto EndOf_Process;
	}
	
	//�ɹ��Ժ���³���װ��
	sprintf( szSql, "update PetEquip set ItemType=0,ItemID=0,EntityID=0\
	                 where PetID=%d and EquipIndex=%d;",petID,equipIndex);
	iRet = dbo.ExceSQL(szSql);
	if(iRet!=0)
	{
	  RetCode = ERR_SYSTEM_DBNORECORD;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s] " , szSql);
		goto EndOf_Process;
	}

	//��ȡ�;öȺͰ�״̬
	sprintf( szSql, "select Durability,BindStatus from Entity\
									 where EntityID= %d;", EntityID);
	iRet=dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		RetCode = ERR_SYSTEM_DBNORECORD;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
		goto EndOf_Process;
	}
	if(iRet < 0)
	{
	  RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
		goto EndOf_Process;
	}

	lic1.celIndex=celIndex;
	lic1.ItemID=ItemID;
	lic1.EntityID=EntityID;
	lic1.cdTime = 0;
	lic1.num=1;

	lic22.celIndex = EquipIndex;
	lic22.ItemID=ItemID;
	lic22.EntityID=EntityID;
	lic22.cdTime = 0;
	lic22.num=0;
	
	lic22.durability=lic1.durability=dbo.GetIntField(0);
	lic22.bindStatus=lic1.bindStatus=dbo.GetIntField(1);
	
  lic.push_back(lic1);
  lic2.push_back(lic22);

	//================���Ա仯
	GetPetItemBonus(ItemID,Bon1);
	GetPetBonusoff(petID,Bon1,pet);

	for(int i=1;i<=16;i++)
	{
		il.BonusAttrID=i;
		il.Num=PetGetNewBonus( pet, i);
		Bon2.push_back(il);
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

	if( 0 == RetCode )
	{
			LOG(LOG_DEBUG,__FILE__,__LINE__,"Pet GetOffS Equip ===== ");
	  pet.Cache2Bonus();//����DB
	 _mainSvc->GetBagSvc()->NotifyBag(roleID,lic);
	 NotifyPetEquipChange(roleID,lic2);
	 NotifyPetBonus(roleID,Bon2);
	} 
	
}

void PetSvc::NotifyPetAttrChange(UInt32 petID)
{
  List<UInt32> lrid;
	char szSql[1024];
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1001;
	p.UniqID = 219;
	p.PackHeader();		

	Connection con;
	DBOperate dbo;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	sprintf(szSql, "select Exp, MaxExp, HP, MP, MaxHP, MaxMP, AttackPowerHigh,AttackPowerLow,Defence, MDefence, CritRate, AddPoint, \
	Strength, Intelligence, Agility, MoveSpeed, HitRate, DodgeRate, AttackSpeed, HPRegen, MPRegen, Level, PetName , RoleID from Pet where  \
									PetID=%d;",petID);

	int iRet = dbo.QuerySQL(szSql);
	if(iRet==1)
	{//
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL Not find[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
	}
	if(iRet<0)
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);	
	}

	PetInfo PetInfoObject;	
	char name[64+1]={0};
	int roleID = 0;
	while (dbo.HasRowData())
	{
		PetInfoObject.Exp = dbo.GetIntField(0);
			PetInfoObject.MaxExp = dbo.GetIntField(1);
			PetInfoObject.HP = dbo.GetIntField(2);
			PetInfoObject.MP = dbo.GetIntField(3);
			PetInfoObject.MaxHP= dbo.GetIntField(4);
			PetInfoObject.MaxMP= dbo.GetIntField(5);
			PetInfoObject.AttackPowerHigh = dbo.GetIntField(6);
			PetInfoObject.AttackPowerLow = dbo.GetIntField(7);
			PetInfoObject.Defence = dbo.GetIntField(8);
			PetInfoObject.MDefence = dbo.GetIntField(9);
			PetInfoObject.CritRate = dbo.GetIntField(10);
			PetInfoObject.AddPoint = dbo.GetIntField(11);
			PetInfoObject.Strength = dbo.GetIntField(12);
			PetInfoObject.Intelligence = dbo.GetIntField(13);
			PetInfoObject.Agility = dbo.GetIntField(14);
			PetInfoObject.MoveSpeed =	dbo.GetIntField(15);
			PetInfoObject.HitRate = dbo.GetIntField(16);
			PetInfoObject.DodgeRate = dbo.GetIntField(17);
			PetInfoObject.AttackSpeed = dbo.GetIntField(18);
			PetInfoObject.HPRegen  = dbo.GetIntField(19);
			PetInfoObject.MPRegen = dbo.GetIntField(20);
			PetInfoObject.Level  = dbo.GetIntField(21);
			strcpy(name,dbo.GetStringField(22));
			PetInfoObject.PetName  = name;
			roleID = dbo.GetIntField(23);
		dbo.NextRow();
	
	}

	
	lrid.push_back(roleID);
	s<< PetInfoObject;
	
	p.UpdatePacketLength();

	if( _mainSvc->Service()->Broadcast( lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
	}

	DEBUG_PRINTF1( "S_C req pkg ----- MsgType[%d] \n", p.MsgType);
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}

void PetSvc::GetPetBonusoff(UInt32 petID,List<ArchvroleBonuschange>& Bon,Pet& pet)
{
	List<ArchvroleBonuschange>::iterator itor;
	for( itor=Bon.begin(); itor != Bon.end(); itor++ )
	{
		PetBonusFunc(petID,itor->BonusAttrID,-(itor->Num),pet);
	}
}

void PetSvc::GetPetBonusin(UInt32 petID,List<ArchvroleBonuschange>& Bon,Pet& pet)
{
	List<ArchvroleBonuschange>::iterator itor;
	for( itor=Bon.begin(); itor != Bon.end(); itor++ )
	{
		PetBonusFunc(petID,itor->BonusAttrID,itor->Num,pet);
	}
}

//[MsgType:2302] �������
void PetSvc::ProcessPetIsUse(Session& session, Packet& packet)
{
	char szSql[1024];
	UInt32	RetCode = 0;
	UInt32 petID=0;
	DataBuffer	serbuffer(1024);
	UInt32 roleID= packet.RoleID;
	Connection con;
	DBOperate dbo;
	int iRet;
	PetbrifInfo petinfo;
	
	Serializer s(packet.GetBuffer());
	s>>petID;
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}
	
	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//�жϳ����Ƿ����
	sprintf(szSql,"select IsUse,PetType,PetName,Level from Pet \
								where PetID = %d and RoleID = %d;",petID,roleID);
	iRet = dbo.QuerySQL(szSql);
	if(iRet < 0)
	{
		RetCode=ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s]", szSql);
		goto EndOf_Process;
	}
	if(iRet == 1)
	{
	  RetCode=ERR_SYSTEM_DBNORECORD;
		LOG(LOG_ERROR,__FILE__,__LINE__,"role[%d] has no pet[%d] !",roleID,petID);
		goto EndOf_Process;
	}

  petinfo.petID = petID;
  petinfo.IsOut = dbo.GetIntField(0);
  petinfo.PetType = dbo.GetIntField(1);
  petinfo.PetName = dbo.GetStringField(2);
  petinfo.Level = dbo.GetIntField(3);
	if(petinfo.IsOut)
	{
	  RetCode=ERR_APP_DATA;
		LOG(LOG_ERROR,__FILE__,__LINE__,"role[%d]  pet[%d] is using !",roleID,petID);
		goto EndOf_Process;
	}

	//��ǰ�Ƿ��г��ﴦ�ڼ���״̬,ÿ��ֻ�ܼ���һ������
	sprintf(szSql,"update Pet set IsUse = 0\
									where RoleID = %d and IsUse = 1;",roleID);
	iRet = dbo.QuerySQL(szSql);
	if(iRet != 0)
	{
	  RetCode=ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s]", szSql);
		goto EndOf_Process;
	}

	//�������
	sprintf(szSql,"update Pet set IsUse = 1 \
								where PetID = %d and RoleID = %d;",petID,roleID);
	iRet = dbo.ExceSQL(szSql);
	if(iRet != 0)
	{
			RetCode=ERR_SYSTEM_DBERROR;
			LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error szSql[%s] ",szSql);
			goto EndOf_Process;
	}

  petinfo.IsOut = 1;
	
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
	if(RetCode==0)
	{
	  //s-c,�������Ľ�ɫ���ͳ��Ｄ��
	  LOG(LOG_DEBUG,__FILE__,__LINE__," NotifyPetOut !");
	  NotifyPetOut(roleID,petinfo);


		//�����������״̬
		_mainSvc->GetTaskSvc()->OnCallPet(roleID, petinfo.PetType);
	}
	
}

void PetSvc::PetBonusFunc(UInt32 petID,UInt32 BonusAttrID,Int32 num,Pet& pet)
{
	char szSql[1024];
	UInt32 iRet;
	Connection con;
	DBOperate dbo;
	ArchvroleBonuschange Bon;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());
	if( 1 == BonusAttrID)
	{
		pet.AddMaxHpBonus(num);
	}

	else if( 2 == BonusAttrID)
	{
		pet.AddMaxMpBonus(num);
	}

 else if( 3 == BonusAttrID)
	{
		pet.AttackPowerHighBonus(num);
	}

	else if( 4 == BonusAttrID)
	{
		pet.AddDefenceBonus(num);
	
	}
	else if( 5 == BonusAttrID)
	{
		pet.AddMDefenceBonus(num);
	}
	else if( 6 == BonusAttrID)
	{
		pet.AddCritRateBonus(num);
	
	}
	else if( 7 == BonusAttrID)
	{
		pet.AddStrengthBonus(num);
	}
	else if( 8 == BonusAttrID)
	{
		pet.AddIntelligenceBonus(num);
	}
	else if( 9 == BonusAttrID)
	{
		pet.AddAgilityBonus(num);
		
	}
	else if( 10 == BonusAttrID)
	{
		pet.AddMoveSpeedBonus(num);
	}
	else if( 11 == BonusAttrID)
	{
		pet.AddHitRateBonus(num);
	}
	else if( 12 == BonusAttrID)
	{
		pet.AddDodgeRateBonus(num);
	}
	else if( 13 == BonusAttrID)
	{
		pet.AddAttackSpeedBonus(num);
	}
	else if( 14 == BonusAttrID)
	{
		pet.AddHpRegenBonus(num);
		
	}
	else if( 15 == BonusAttrID)
	{
		pet.AddMpRegenBonus(num);
		
	}
	else
	{
		
		LOG(LOG_ERROR,__FILE__,__LINE__,"the BonusAttrID is not exit! ");
	}
		
}

void PetSvc::NotifyPetSkillChange(UInt32 RoleID,List<ArchvSkill>&lskill)
{
	int iRet = 0;
	char szSql[1024];
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1004;
	p.UniqID = 217;
	List<UInt32> lrid;
	p.PackHeader();

	lrid.push_back(RoleID);
	s<<lskill;		

	LOG(LOG_DEBUG,__FILE__,__LINE__,"==== NotifyPetSkillChange ====");
	for(List<ArchvSkill>::iterator itor = lskill.begin(); itor != lskill.end(); itor++)
	{
	   LOG(LOG_DEBUG,__FILE__,__LINE__,"skillID[%d]",itor->skillID);
	   LOG(LOG_DEBUG,__FILE__,__LINE__,"skillID[%d]",itor->skillLevel);
	}
  
	p.UpdatePacketLength();

	if( _mainSvc->Service()->Broadcast( lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
	}
	
	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}

void PetSvc::NotifyPetEquipChange(UInt32 RoleID,List<ItemCell>& lic)
{
	List<UInt32> lrid;
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1004;//����װ�����
	p.UniqID = 252;
	p.PackHeader();
	lrid.push_back(RoleID);
	s<<lic;
	p.UpdatePacketLength();

	LOG(LOG_DEBUG,__FILE__,__LINE__,"NotifyPetEquipChange ======" );

	if( _mainSvc->Service()->Broadcast( lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
	}
	
	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}

//1006 ���Ｄ�����Χ������ɫ���͸�֪ͨ
void PetSvc::NotifyPetOut(UInt32 &RoleID,PetbrifInfo& petinfo)
{
	List<UInt32> lrid;
	UInt32 mapID=_mainSvc->GetCoreData()->ProcessGetRolePtr(RoleID)->MapID();
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;

	//mapID������RoleID
	_mainSvc->GetCoreData()->GetMapRoleIDs(mapID,lrid);
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1006;
	p.UniqID = 252;
	p.PackHeader();

	s<<RoleID;
	s<<petinfo;
	p.UpdatePacketLength();
	
	if( _mainSvc->Service()->Broadcast(lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
	}
	
	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}

//1002����ӳ����Ա��
void PetSvc::NotifyPetBonus(UInt32 RoleID,List<ArchvroleBonuschange> &k)
{
	List<UInt32> lrid;
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1002;
	p.UniqID = 214;
	p.PackHeader();
	lrid.push_back(RoleID);
	s<<k;

	p.UpdatePacketLength();
	LOG(LOG_DEBUG,__FILE__,__LINE__,"NotifyPetBonus ======" );
	if( _mainSvc->Service()->Broadcast( lrid, &serbuffer))
	{
		LOG(LOG_DEBUG,__FILE__,__LINE__,"Broadcast error!!" );
	}

	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );

}


//[MsgType:1005]��ó���֪ͨ
void PetSvc::NotifyGetPet(UInt32 &roleID,PetBrief petBrief)
{
  List<UInt32> lrid;
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1005;    //��Ϣ���ʹ���
	p.UniqID = 214;
	p.PackHeader();
	
	lrid.push_back(roleID);
	s<<petBrief;

	p.UpdatePacketLength();
	
	if( _mainSvc->Service()->Broadcast( lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
	}

	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );

}

//���ﾭ������֪ͨ��MsgType����
void PetSvc::NotifyPetAddExp(UInt32 &roleID,UInt32 petID,UInt32 &curExp)
{ 
  List<UInt32> lrid;
	DataBuffer	serbuffer(8196);
	Serializer s( &serbuffer );
	Packet p(&serbuffer);
	serbuffer.Reset();
	p.Direction = DIRECT_S_C_REQ;
	p.SvrType = 1;
	p.SvrSeq = 1;
	p.MsgType = 1007;    //��Ϣ���ʹ���
	p.UniqID = 214;
	p.PackHeader();
	
	lrid.push_back(roleID);
	s<<petID<<curExp;

	p.UpdatePacketLength();
	
	if( _mainSvc->Service()->Broadcast( lrid, &serbuffer))
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!" );
	}

	DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
	DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );

}

UInt32 PetSvc::PetGetNewBonus( Pet& pet, UInt32 l)
{
	UInt32 v;
	switch(l)
	{
		case 1:
			v=pet.MaxHpBonus();
			break;
		case 2:
			v=pet.MaxMpBonus();
			break;
		case 3:
			v = pet.AttackPowerHighBonus();
			break;
		case 4:
			v= pet.AttackPowerLowBonus();
			break;
		case 5:
			v= pet.DefenceBonus();
			break;
		case 6:
			v = pet.MDefenceBonus();
			break;
		case 7:
			v = pet.CritRateBonus();
			break;
		case 8:
			v = pet.StrengthBonus();
			break;
		case 9:
			v= pet.IntelligenceBonus();
			break;
		case 10:
			v= pet.AgilityBonus();
			break;
		case 11:
			v=pet.MovSpeedBonus();
			break;
		case 12:
			v=pet.HitRateBonus();
			break;
		case 13:
			v= pet.DodgeRateBonus();
			break;
		case 14:
			v= pet.AttackSpeedBonus();
			break;
		case 15:
			v=pet.MaxHpBonus();
			break;
		case 16:
			v=pet.MaxMpBonus();
			break;
		default:
		LOG(LOG_ERROR,__FILE__,__LINE__,"Type error ");
		break;
	}
		return v;	
}

//��ȡ����װ��������
void PetSvc::GetPetItemBonus(UInt32 ItemID,List<ArchvroleBonuschange>& Bon)
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());
	ArchvroleBonuschange bon1;
	
	sprintf( szSql, "select BonusAttr,BonusType,BonusValue from ItemBonus where ItemID=%d;",ItemID);
	iRet=dbo.QuerySQL(szSql);
	if(iRet!=0)
	{//��ѯ�����ݲ�����
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL NOT found[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return ;
	}
	else 
	{
		while(dbo.HasRowData())
		{
			bon1.BonusAttrID=dbo.GetIntField(0);
			bon1.Num=dbo.GetIntField(2);

			Bon.push_back(bon1);
			dbo.NextRow();
		}
	}
	return ;
}


//����ϳ� ������0 �ɹ��� ��0 ʧ��
//@param roleID  ��ɫID
//@param petID   ����ID
//return lpetBrief �����Ҫ��Ϣ
int PetSvc::OnPetComposite(UInt32 &roleID, UInt32 &petType)
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	int iRet = 0;
	int petCnt = 0;
	UInt32 petID = 0,petKind = 0,equipIndex = 0,newPetSkillID = 0;
	string petName;
	UInt32 petLevl = 0,attackScope = 0, bulletSpeed = 0;
	PetInfo petinfo;
	PetBonus petbonus;
	PetEquip petequip;
	PetBrief petbrief;

	//��ȡ���ݿ�����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());
  //��ȡ��ǰ��ɫӵ�еĳ��������ÿ����ɫ���ӵ��6������
  sprintf(szSql,"select PetID from Pet where RoleID = %d;",roleID);
  iRet = dbo.QuerySQL(szSql);
  if(iRet < 0)
  {
    LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s] ",szSql);
    return -1;
  }
  if(iRet == 1)
  {
    LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s] ",szSql);
    return -1;
  }

  while(dbo.HasRowData())
  {
     petCnt++;
     dbo.NextRow();
  }

  //���ﳬ��6��
  if(petCnt >= PET_MAX_NUM)
  {
    LOG(LOG_ERROR,__FILE__,__LINE__,"total of pet is [%d] more than [%d]",petCnt,PET_MAX_NUM);
    return -1;
  }

  //��ѯ�ϳɳ������Ϣ
  sprintf(szSql,"select PetKind,    PetName,        Level,\
                        Exp,        MaxExp,	        AddPoint\
                        Strength,   Intelligence,   Agility\
                        MoveSpeed,  HP,             MP\
                        MaxHp,      MaxMp,          HPRegen\
                        MPRegen,    AttackPowerHigh,AttackPowerLow\
                        AttackScope,AttackSpeed,    BulletSpeed\
                        Defence ,   MDefence,       CritRate\
                        HitRate,    DodgeRate\
                        from PetDesc where PetType = %d;",petType);
  iRet = dbo.QuerySQL(szSql);
  if(iRet < 0)
  {
    LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s]",szSql);
    return -1;
  }
  if(iRet == 1)
  {
    LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s]",szSql);
    return -1;
  }

  petKind = dbo.GetIntField(0);
  petName = dbo.GetStringField(1);
  petLevl = dbo.GetIntField(2);
  petinfo.Exp = dbo.GetIntField(3);
  petinfo.MaxExp = dbo.GetIntField(4);
  petinfo.AddPoint = dbo.GetIntField(5);
  petinfo.Strength = dbo.GetIntField(6);
  petinfo.Intelligence = dbo.GetIntField(7);
  petinfo.Agility = dbo.GetIntField(8);
  petinfo.MoveSpeed = dbo.GetIntField(9);
  petinfo.HP = dbo.GetIntField(10);
  petinfo.MP = dbo.GetIntField(11);
  petinfo.MaxHP = dbo.GetIntField(12);
  petinfo.MaxMP= dbo.GetIntField(13);
  petinfo.HPRegen = dbo.GetIntField(14);
  petinfo.MPRegen = dbo.GetIntField(15);
  petinfo.AttackPowerHigh = dbo.GetIntField(16);
  petinfo.AttackPowerLow = dbo.GetIntField(17);
  attackScope = dbo.GetIntField(18);
  petinfo.AttackSpeed = dbo.GetIntField(19);
  bulletSpeed = dbo.GetIntField(20);
  petinfo.Defence = dbo.GetIntField(21);
  petinfo.MDefence = dbo.GetIntField(22);
  petinfo.CritRate = dbo.GetIntField(23);
  petinfo.HitRate = dbo.GetIntField(24);
  petinfo.DodgeRate = dbo.GetIntField(25);
  petinfo.Level = petLevl;
  petinfo.PetName = petName;

  //��pet������¼
  sprintf(szSql,"insert Pet  ( PetType,\
														   PetKind,\
														   PetName,\
														   RoleID,\
														   Level,\
														   Exp,\
														   MaxExp,\
														   AddPoint,\
														   Strength,\
														   Intelligence,\
														   Agility,\
														   MoveSpeed,\
														   HP,\
														   MP,\
														   MaxHP,\
														   MaxMP,\
														   HPRegen,\
														   MPRegen,\
														   AttackPowerHigh,\
														   AttackPowerLow,\
														   AttackScope,\
														   AttackSpeed,\
														   BulletSpeed,\
														   Defence,\
														   MDefence,\
														   CritRate,\
														   HitRate,\
														   DodgeRate,\
														   IsUse\
														 )\
					value(%d,%d,%s,%d,%d,%d,%d,%d,%d,%d,\
					      %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,\
					      %d,%d,%d,%d,%d,%d,%d,%d,0);",\
							 petType,petKind,petName.c_str(),roleID,petLevl,petinfo.Exp,\
							 petinfo.MaxExp,petinfo.AddPoint,petinfo.Strength,petinfo.Intelligence,\
							 petinfo.Agility,petinfo.MoveSpeed,petinfo.HP,petinfo.MP,petinfo.MaxHP,\
							 petinfo.MaxMP,petinfo.HPRegen,petinfo.MPRegen,petinfo.AttackPowerHigh,\
							 petinfo.AttackPowerLow,attackScope,petinfo.AttackSpeed,bulletSpeed,\
							 petinfo.Defence,petinfo.MDefence,petinfo.CritRate,petinfo.HitRate,\
							 petinfo.DodgeRate);
							 
 iRet = dbo.ExceSQL(szSql);
 if(iRet != 0)
 {
    LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s]",szSql);
    return -1;
 }
 petID = dbo.LastInsertID();

 //��petbonus������¼
 sprintf(szSql,"insert PetBonus(\
																 PetID,\
															   MPRegen,\
															   HPRegen,\
															   MaxMP,\
															   MaxHP,\
															   MovSpeed,\
															   Agility,\
															   Intelligence,\
															   Strength,\
															   AttackPowerHigh,\
															   AttackPowerLow,\
															   AttackSpeed,\
															   Defence,\
															   MDefence,\
															   CritRate,\
															   HitRate,\
															   DodgeRae\
 																)\
 											value(%d,%d,%d,%d,%d,%d,%d,%d,\
 											      %d,%d,%d,%d,%d,%d,%d,%d,%d);",\
 											petID,petinfo.MPRegen,petinfo.HPRegen,petinfo.MaxMP,\
 											petinfo.MaxHP,petinfo.MoveSpeed,petinfo.Agility,\
 											petinfo.Intelligence,petinfo.Strength,petinfo.AttackPowerHigh,\
 											petinfo.AttackPowerLow,petinfo.AttackSpeed,petinfo.Defence,\
 											petinfo.MDefence,petinfo.CritRate,petinfo.HitRate,petinfo.DodgeRate);
 iRet = dbo.ExceSQL(szSql);
 if(iRet != 0)
 {
    LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s]",szSql);
    return -1;
 }

 //��PetEquip������¼��װ��λ�� 1,2,3,10�ֱ��ʾ ͷ�������أ���
 for(int i = 0; i < 4; ++i)
 {
 	 equipIndex = i + 1;
   if(i == 4)
   {
      equipIndex = 10;
   } 
   
   sprintf(szSql,"insert PetEquip(PetID,EquipIndex,ItemType,ItemID,EntityID)\
 														value(%d,%d,0,0,0)",petID,equipIndex);
 	 iRet = dbo.ExceSQL(szSql);
 	 if(iRet != 0)
 	 {
 	   LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s]",szSql);
     return -1;
 	 }
 }

 //��ѯ�ºϳɳ���ļ�����Ϣ
 sprintf(szSql,"select SkillID from PetSkillDesc where PetType = %d;",petType);
 iRet = dbo.QuerySQL(szSql);
 if(iRet < 0)
 {
   LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s]",szSql);
   return -1;
 }
 if(iRet == 1)
 {
   LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s]",szSql);
   return -1;
 }

 newPetSkillID = dbo.GetIntField(0);
 
 //��PetSkill������¼
 sprintf(szSql,"insert PetSkill(PetID,SkillID,SkillLev)\
 															value(%d,%d,0);",petID,newPetSkillID);
 iRet = dbo.ExceSQL(szSql);
 if(iRet != 0)
 {
   LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s]",szSql);
   return -1;
 }

 petbrief.petId = petID;
 petbrief.petLevel = petLevl;
 petbrief.petName = petName;
 petbrief.petType = petType;
 //lpetBrief.push_back(petbrief);
 
 //֪ͨ�ϳɳ���
 NotifyGetPet(roleID,petbrief);
 
 return 0; //���� 0 �ɹ�
  
}
	
//���ﶪ�������� 0 �ɹ����� 0 ʧ��
int PetSvc::OnPetAbandon(UInt32 &roleID,UInt32 &petID)
{
	char szSql[1024];
	int iRet;
	Connection con;
	DBOperate dbo;
	UInt32 tmpRoleID = 0;

	//��ȡ���ݿ�����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

  //ɾ��Pet���¼
  sprintf(szSql,"delete from Pet where PetID = %d and RoleID = %d;",petID,roleID);
  iRet = dbo.ExceSQL(szSql);
  if(iRet != 0)
  {
    LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s]",szSql);
    return -1;
  }

  //ɾ��PetBonus���¼
  sprintf(szSql,"delete from PetBonus where PetID = %d;",petID);
  iRet = dbo.ExceSQL(szSql);
  if(iRet != 0)
  { 
    LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s]",szSql);
    return -1;
  }

  //ɾ��PetEquip���¼
  sprintf(szSql,"delete from PetEquip where PetID = %d;");
  iRet = dbo.ExceSQL(szSql);
  if(iRet != 0)
  {
    LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s]",szSql);
    return -1;
  }

  //ɾ��PetSkill���¼
  sprintf(szSql,"delete from PetSkill where PetID = %d;",petID);
  iRet = dbo.ExceSQL(szSql);
  if(iRet != 0)
  {
    LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found,szSql[%s]",szSql);
    return -1;
  }

  return 0;  //�ɹ����� 0
  
}

