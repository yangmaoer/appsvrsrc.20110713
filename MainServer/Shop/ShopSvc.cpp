#include "ShopSvc.h"
#include "MainSvc.h"
#include "DBOperate.h"
#include "CoreData.h"
#include "Role.h"
#include "ArchvShop.h"
#include "./Bag/BagSvc.h"
#include "./Task/TaskSvc.h"
#include "ArchvBagItemCell.h"

ShopSvc::ShopSvc(void* service, ConnectionPool * cp)
{
	_mainSvc = (MainSvc*)(service);
	_cp = cp;

}

ShopSvc::~ShopSvc()
{

}
//���ݰ���Ϣ
void ShopSvc::OnProcessPacket(Session& session,Packet& packet)
{
	DEBUG_PRINTF1( "C_S req pkg-------MsgType[%d] \n", packet.MsgType );
	DEBUG_SHOWHEX( packet.GetBuffer()->GetReadPtr()-PACKET_HEADER_LENGTH, packet.GetBuffer()->GetDataSize()+PACKET_HEADER_LENGTH, 0, __FILE__, __LINE__ );

	switch(packet.MsgType)
	{
		case 1701:
			ProcessselectShopItem(session,packet);
		break;
		case 1702:
      ProcessBuyItem(session,packet);
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
void ShopSvc::ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode)
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


void ShopSvc::ProcessselectShopItem(Session& session,Packet& packet)
{
	//��ѯ������Ʒ����
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con,conSub;
	DBOperate dbo,dboSub;

	Int16 iRet = 0;
	UInt32 roleID = packet.RoleID;
	List<ArchvShopItem> shopItem;
	ArchvShopItem item1;

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

	conSub=_cp->GetConnection();
	dboSub.SetHandle(conSub.GetHandle());

	DEBUG_PRINTF( "C_S ProcessShopItem sucess!!!!!!!!\n" );

	//��ѯȫ����
	sprintf( szSql, "select ItemID,Category,NowPrice from ShopItem;");
	iRet=dbo.QuerySQL(szSql);
	if(iRet==0)
	{
		while(dbo.HasRowData())
		{
			item1.ItemID=dbo.GetIntField(0);
			item1.Category=dbo.GetIntField(1);
			item1.NowPrice=dbo.GetIntField(2);
			if(item1.Category==7)
			{
				sprintf( szSql, "select leftNum from specialItem where ItemID=%d;",item1.ItemID);
				iRet=dboSub.QuerySQL(szSql);
				if(iRet==0)
				{
					item1.Num=dboSub.GetIntField(0);
				}
			}

			shopItem.push_back(item1);
			dbo.NextRow();
		}
	}
	else
	{

		RetCode = ERR_SYSTEM_DBNORECORD;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		goto EndOf_Process;
	}

	//LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data what to get not found,szSql[%d %d] " , PackNum,GetRoleCellNum(roleID));
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
		s<<shopItem;

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


void ShopSvc::ProcessBuyItem(Session& session,Packet& packet)
{


	UInt32	RetCode = 0;
	DataBuffer	serbuffer(1024);
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	Int16 iRet = 0;
	UInt16 num=0;
	UInt16 cellType=0;
	UInt16 itemType=0;
	UInt16 IsStack=0;
	UInt16 Bind=0;
	UInt32 Dur=0;
	UInt32 price=0;
	UInt32 EntityID=0;
	UInt32 CellIndex=0;
	UInt32 leftnum=0;

	List<UInt16> cell;
	List<ItemCell> lic;
	ItemCell lic1;
	List<UInt16>::iterator itor;

	UInt32 roleID = packet.RoleID;
	UInt32 itemID=0;
	UInt16 count=0;
	UInt32 numcell=1;
	UInt16 Rarity=0;
	UInt32 BuyType=0;
	Byte type=0;
	Byte flag=0;

	//���л���
	Serializer s(packet.GetBuffer());
	s>>type>>itemID>>num;
	LOG(LOG_ERROR,__FILE__,__LINE__,"type[%d]--itemid[%d]--num[%d]",type,itemID,num);
	//�õ���ƷID������
	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	//��ȡDB����
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());
	//��֤�۸�

	sprintf( szSql, "select NowPrice from ShopItem where ItemID=%d and Category=%d;",itemID,type );
	iRet = dbo.QuerySQL(szSql);
	if(iRet==0)
	{
		price=dbo.GetIntField(0);
	}
	else
	{
		RetCode = ERR_SYSTEM_DBNORECORD;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found or erro,szSql[%s] " , szSql);
		goto EndOf_Process;
	}
	if(type==6)
	{
		//��ʾ��ȯ����
		iRet=DropGift(roleID,price*num);
		if(iRet==1)
		{
			//û����ô����������ȯ
			RetCode = NO_MUCH_GIFT;
			LOG(LOG_ERROR,__FILE__,__LINE__,"Not so many Gift!! ");
			goto EndOf_Process;
		}
		else if(iRet==-1)
		{
			//����
			RetCode = ERR_APP_DATA;
			LOG(LOG_ERROR,__FILE__,__LINE__,"there are some erro!! ");
			goto EndOf_Process;
		}

	}
	else if(type==7)
	{
		//��ʾ��������
		sprintf( szSql, "select ItemID,leftNum from specialItem where ItemID=%d;",itemID);

		iRet = dbo.QuerySQL(szSql);
		if(iRet!=0)
		{
			RetCode = ERR_SYSTEM_DBNORECORD;
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
			goto EndOf_Process;
		}

		else
		{
			leftnum=dbo.GetIntField(1);
		}

		if(leftnum<num)
		{
			//��������
			RetCode = ERR_SYSTEM_DBNORECORD;
			LOG(LOG_ERROR,__FILE__,__LINE__,"Not so many count!!!! ");
			goto EndOf_Process;
		}

	}
if(type!=6)
{
		// type Ϊ5��ʾvip�ۿ�
		if (type == 5)
		{
			RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(roleID);
			sprintf( szSql, "select Discount from VipDiscountDesc where VipLevel = %d;",pRole->VIP());

			iRet = dbo.QuerySQL(szSql);
			if( iRet != 0 )
			{
				RetCode = ERR_SYSTEM_DBERROR;
				LOG(LOG_ERROR,__FILE__,__LINE__,",szSql[%s] ", szSql);
				goto EndOf_Process;
				}
			int discount = 0;
			while (dbo.HasRowData())
			{
				discount = dbo.GetIntField(0);
				dbo.NextRow();
			}
			if (discount != 0)
			{
				price = ((price * discount /100) > 0) ? (price * discount /100) :1;
			}
		}
		iRet=_mainSvc->GetBagSvc()->DropGold(roleID,price*num);
		if(iRet==-1)
		{
			//����
			RetCode = NO_MUCH_GOLD;
			LOG(LOG_ERROR,__FILE__,__LINE__,"Not so many Gold!! ");
			goto EndOf_Process;
		}
		else if(iRet==0)
		{
			//����
			RetCode = ERR_SYSTEM_DBNORECORD;
			LOG(LOG_ERROR,__FILE__,__LINE__,"there are some erro!! ");
			goto EndOf_Process;
		}
}
	//���SQL��䣬��ѯ����������Ʒ��Ϣ����ѯ����������Ʒ���ͺ���Ʒ��Ԫ������
	sprintf( szSql, "select Rarity,IsStack,Bind,CostMoneyType,CostBuy, Durability,ItemType,CellType from Item where ItemID = %d;",itemID );

	iRet = dbo.QuerySQL(szSql);
	if( 1 == iRet)
	{
		RetCode = ERR_SYSTEM_DBNORECORD;
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
		goto EndOf_Process;
	}
	if( iRet < 0 )
	{
		RetCode = ERR_SYSTEM_DBERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,",szSql[%s] ", szSql);
		goto EndOf_Process;
 	}
 	//������������ɱ���, IsStack �Ƿ�ѵ�   ������Bind   Durability�;ö�
 	if(iRet==0)
	{
		if(dbo.RowNum()==1)
		while(dbo.HasRowData())
		{
			Rarity=dbo.GetIntField(0);

			IsStack=dbo.GetIntField(1);
			Bind=dbo.GetIntField(2);
			BuyType=dbo.GetIntField(3);
			//price=dbo.GetIntField(4);
			Dur=dbo.GetIntField(5);
			itemType= dbo.GetIntField(6);
			cellType=dbo.GetIntField(7);

			//��¼����һ����¼
			dbo.NextRow();
		}
	}
	//��֪����Ʒ��һЩ�ǲ��߱�ʵ��ID�ģ������ǽ��ж�ʵ��ID����в������ݵĴ���
	//�ж��Ƿ����ʵ��ID�������Ƿ���1�����Ƿ���0��int isEntity(int itemtype);�ú�����û�п�ʼ��д����
	if(itemType==2)//�߱�ʵ��ID�Ļ�����ʵ�����룬 ���ĵĻ��������ֵ�����޸�
	{
		flag=0;
		if(IsStack==1)
		{
			RetCode = ERR_SYSTEM_DBNORECORD;
			LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data error ,szSql[%s] " , szSql);
			goto EndOf_Process;
		}

		if(_mainSvc->GetBagSvc()->Ifhassomany(roleID,cell,num)!=1)
		{
			//��������
			RetCode = ERR_SYSTEM_DBNORECORD;
			LOG(LOG_ERROR,__FILE__,__LINE__," Bag has not so many room !" );
			goto EndOf_Process;
		}
		lic1.bindStatus=Bind;
 		lic1.durability=Dur;
 		lic1.EntityID=EntityID;
 		lic1.ItemID=itemID;


	}
	else
	{
		if(IsStack==0)
		{//�����Զѵ�
		flag=1;
				if(_mainSvc->GetBagSvc()->Ifhassomany(roleID,cell,num)!=1)
				{
				//����û����ô��ռ�
					RetCode = ERR_SYSTEM_DBERROR;
					LOG(LOG_ERROR,__FILE__,__LINE__,"Bag has no Room to insert,szSql[%s] ", mysql_error(con.GetHandle()), szSql);
					goto EndOf_Process;
				}
		}
		else
		{
				flag=2;
				numcell=num/TOPSTACK_NUM;//һ����ô�����Ԫ
				if(num%TOPSTACK_NUM!=0)
				{
					numcell=numcell+1;
				}

				if(_mainSvc->GetBagSvc()->Ifhassomany(roleID,cell,numcell)==0)
				{
					//����û����ô��ռ�
					RetCode = ERR_SYSTEM_DBERROR;
					LOG(LOG_ERROR,__FILE__,__LINE__,"Bag has no Room to insert,szSql[%s] ", mysql_error(con.GetHandle()), szSql);
					goto EndOf_Process;
				}

	 	}
		sprintf( szSql, "select CdTime from Item where ItemID= %d;", itemID);
		iRet = dbo.QuerySQL(szSql);
		if(iRet==0)
		{
			lic1.cdTime=dbo.GetIntField(0);
		}
 		lic1.bindStatus=Bind;
		lic1.durability=Dur;
		lic1.EntityID=EntityID;
		lic1.ItemID=itemID;
 	}
	//�����ݿ�������в�������!
	for( itor = cell.begin(); itor != cell.end(); itor++ )
	{
	if(flag==0)
	{
		sprintf( szSql,"insert into Entity(ItemID,Durability,BindStatus) values(%d,%d,%d);",itemID,Dur,Bind);

				iRet = dbo.ExceSQL(szSql);
				if( iRet < 0 )
				{
					RetCode = ERR_SYSTEM_DBERROR;
					LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
					goto EndOf_Process;
		 		}

				EntityID=dbo.LastInsertID();

				sprintf(szSql, "insert into Bag(RoleID,CellType,CellIndex,ItemType,ItemID,EntityID,Num)values(%d,%d,%d,%d,%d,%d,1);",roleID,cellType,*itor,itemType,itemID,EntityID);
				iRet=dbo.ExceSQL(szSql);

				if( 1 == iRet )
				{
					RetCode = ERR_SYSTEM_DBNORECORD;
					LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found ,szSql[%s] " , szSql);
					goto EndOf_Process;
				}
				if( iRet < 0 )
				{
					RetCode = ERR_SYSTEM_DBERROR;
					LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
					goto EndOf_Process;
				}

				lic1.num=1;
				lic1.celIndex=*itor;
				lic1.EntityID=EntityID;
				lic.push_back(lic1);
	}
	if(flag==2)
	{
				if(numcell==1)
				{
					//��������Ϊ���������
					if(num%TOPSTACK_NUM==0)
					{
						count=TOPSTACK_NUM;
					}
					else
					{
						count=num%TOPSTACK_NUM;
					}
				}
				else
				{
					count=TOPSTACK_NUM;
				}

				sprintf(szSql, "insert into Bag(RoleID,CellType,CellIndex,ItemType,ItemID,EntityID,Num)values(%d,%d,%d,%d,%d,%d,%d);",roleID,cellType,*itor,itemType,itemID,EntityID,count);
				iRet=dbo.ExceSQL(szSql);

				if( 1 == iRet )
				{
					RetCode = ERR_SYSTEM_DBNORECORD;
					LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found ,szSql[%s] " , szSql);
					goto EndOf_Process;
				}
				if( iRet < 0 )
				{
					RetCode = ERR_SYSTEM_DBERROR;
					LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
					goto EndOf_Process;
				}

				lic1.num=count;
				lic1.celIndex=*itor;
				lic.push_back(lic1);
		}
		else if(flag==1)
		{
				sprintf(szSql, "insert into Bag(RoleID,CellType,CellIndex,ItemType,ItemID,EntityID,Num)values(%d,%d,%d,%d,%d,%d,1);",roleID,cellType,*itor,itemType,itemID,EntityID);
				iRet=dbo.ExceSQL(szSql);

				if( 1 == iRet )
				{
					RetCode = ERR_SYSTEM_DBNORECORD;
					LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL data not found ,szSql[%s] " , szSql);
					goto EndOf_Process;
				}
				if( iRet < 0 )
				{
					RetCode = ERR_SYSTEM_DBERROR;
					LOG(LOG_ERROR,__FILE__,__LINE__,"ExceSQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
					goto EndOf_Process;
				}

				lic1.num=1;
				lic1.celIndex=*itor;
				lic.push_back(lic1);
		}
		numcell--;
	}
	_mainSvc->GetTaskSvc()->OnBagItemAddOrDelete(roleID,itemID);


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
		//s<<lic;
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
		if(type!=6)
		{
			_mainSvc->GetBagSvc()->NotifyMoney(roleID);
		}

		if(type==7)
		{
			UpdateDBspecialItem(itemID,num);
		}
			UpdateDBItemSellCount(itemID,type,num);
 	}
	return;


}


//�����̳��г��۵���Ʒ������
int ShopSvc::UpdateDBItemSellCount(UInt32 ItemID,Byte type,UInt32 num)
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	Int16 iRet = 0;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());


	sprintf(szSql, "update ShopItem set ItemSellTolalNum=ItemSellTolalNum+%d where ItemID =%d and Category=%d;",num,ItemID,type);
	iRet=dbo.ExceSQL(szSql);

	if(iRet!=0)
	{
			LOG(LOG_ERROR,__FILE__,__LINE__,"szSql[%s] ", mysql_error(con.GetHandle()), szSql);
	}

	return 0;
}
//@return 1��ȯ����
//@return -1 ����
//@return 0�ɹ�

int ShopSvc::DropGift(UInt32 roleID,UInt32 num)
{
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	Int16 iRet = 0;
	UInt32 Gift=0;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());


	sprintf(szSql, "select Gift from RoleMoney where RoleID=%d",roleID);
	iRet=dbo.QuerySQL(szSql);
	if(iRet==0)
	{
		while(dbo.HasRowData())
		{
			Gift=dbo.GetIntField(0);
			dbo.NextRow();
		}
	}
	else
	{
		//ch
		LOG(LOG_ERROR,__FILE__,__LINE__,"szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
	}

	if(Gift>num)
	{

		sprintf(szSql, "Update RoleMoney set Gift=Gift-%d where RoleID=%d",num,roleID);
		iRet=dbo.ExceSQL(szSql);
		if(iRet!=0)
		{

			LOG(LOG_ERROR,__FILE__,__LINE__,"szSql[%s] ", mysql_error(con.GetHandle()), szSql);
 	    return -1;
		}

	}
	else
	{
		//û���㹻�����ȯ
		LOG(LOG_ERROR,__FILE__,__LINE__," Not so many Gift");
		return 1;
	}

	if(Gift>num)
	{
		Gift=Gift-num;
		NotifyGift(roleID,Gift);
		//S-C��ȯ
	}
	return 0;
}


int ShopSvc::UpdateDBspecialItem(UInt32 ItemID,UInt32 num)
{
	UInt32	RetCode = 0;
	char szSql[1024];
	Connection con;
	DBOperate dbo;
	Int16 iRet = 0;
	UInt32 LeftNum=0;
	List<ItemList> lis;
	ItemList item;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());


	sprintf(szSql, "Update specialItem set leftNum=leftNum-%d where ItemID=%d and leftNum>%d;",num,ItemID,num);
		iRet=dbo.ExceSQL(szSql);
		if(iRet!=0)
		{

			LOG(LOG_ERROR,__FILE__,__LINE__,"szSql[%s] ", mysql_error(con.GetHandle()), szSql);
 	    return -1;
		}


	sprintf(szSql, "select leftNum from specialItem where ItemID=%d",ItemID);

	iRet=dbo.QuerySQL(szSql);
	if(iRet==0)
	{
		while(dbo.HasRowData())
		{
			LeftNum=dbo.GetIntField(0);
			dbo.NextRow();
		}
	}
	else
	{
		//ch
		LOG(LOG_ERROR,__FILE__,__LINE__,"szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return -1;
	}
	item.ItemID=ItemID;
	item.num=LeftNum;
		lis.push_back(item);
	NotifyspecialItemNum(lis);

	return 0;
}

void ShopSvc::NotifyGift(UInt32 roleID,UInt32 Gift)
{
		DataBuffer	serbuffer(1024);
		Serializer s( &serbuffer );
		Packet p(&serbuffer);
		serbuffer.Reset();
		p.Direction = DIRECT_S_C_REQ;
		p.SvrType = 1;
		p.SvrSeq = 1;
		p.MsgType = 803;
		p.UniqID = 251;
		p.PackHeader();
		List<UInt32> lrid;
		s<<Gift;
		p.UpdatePacketLength();
		lrid.push_back(roleID);
		if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!");
		}
		DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
		DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );

}
void ShopSvc::NotifyspecialItemNum(List<ItemList>& lis )
{
		ItemList item;
		DataBuffer	serbuffer(1024);
		Serializer s( &serbuffer );
		Packet p(&serbuffer);
		serbuffer.Reset();
		p.Direction = DIRECT_S_C_REQ;
		p.SvrType = 1;
		p.SvrSeq = 1;
		p.MsgType = 1701;
		p.UniqID = 251;
		p.PackHeader();
		list<UInt32> lrid;
		_mainSvc->GetCoreData()->GetRoleIDs(lrid);

		s<<lis;
		p.UpdatePacketLength();

		if(_mainSvc->Service()->Broadcast(lrid, &serbuffer))
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"Broadcast error!!");
		}
		DEBUG_PRINTF2( "S_C req pkg ----- MsgType[%d],lrid.size[%d]  \n", p.MsgType, lrid.size() );
		DEBUG_SHOWHEX( p.GetBuffer()->GetReadPtr(), p.GetBuffer()->GetDataSize(), 0, __FILE__, __LINE__ );
}

