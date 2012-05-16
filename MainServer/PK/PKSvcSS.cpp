#include "PKSvcSS.h"
#include "DBOperate.h"
#include "DebugData.h"
#include "Packet.h"
#include "ArchvRole.h"
#include "MainSvc.h"
#include "ArchvPK.h"
#include "ArchvMap.h"
#include "CoreData.h"
#include "PKSvc.h"
#include "ArchvBagItemCell.h"
#include "Role.h"
#include  "Monster.h"
#include "./RoleInfo/RoleInfoSvc.h"
#include "../Task/TaskSvc.h"
#include "../Bag/BagSvc.h"
#include "../Pet/PetSvc.h"
#include <cmath>
#include <algorithm>
#include "./Avatar/AvatarSvc.h"


PKSvcSS::PKSvcSS(MainSvc * mainSvc, ConnectionPool *cp )
:_mainSvc(mainSvc)
,_cp(cp)
{
}

PKSvcSS::~PKSvcSS()
{
}

void PKSvcSS::OnProcessPacket(Session& session,Packet& packet)
{

DEBUG_PRINTF1( "S_S req pkg->->->->->->MsgType[%d]\n", packet.MsgType );
	DEBUG_SHOWHEX( packet.GetBuffer()->GetReadPtr()-PACKET_HEADER_LENGTH, packet.GetBuffer()->GetDataSize()+PACKET_HEADER_LENGTH, 0, __FILE__, __LINE__ );

	switch(packet.MsgType)
	{
	 	case 201: // pk����
			ProcessPKEnd(session,packet);
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
void PKSvcSS::ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode)
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

DEBUG_PRINTF1( "S_S ack pkg ----- MsgType[%d]  \n", packet.MsgType );
	DEBUG_SHOWHEX( serbuffer.GetReadPtr(), serbuffer.GetDataSize(), 0, __FILE__, __LINE__ );

}


//@brief	ɱ�������¼�����
void PKSvcSS::DealKillMonsterTask( list<UInt32>& roleID,UInt32 creaturetype,UInt32 num )
{

//DEBUG_PRINTF2( "DealKillMonsterTask ----- lpkei.size[%d], lcs.size[%d]  \n", lpkei.size(), lcs.size() );

	list<UInt32>::iterator it;
	for( it = roleID.begin(); it != roleID.end(); it++ )
	{
		_mainSvc->GetTaskSvc()->OnAfterKillMonster( *it, creaturetype, num);

		//��Ʒ�������,ɱ��������Ʒ���
		//_mainSvc->GetBagSvc()->GetItem( it2->roleID, 10, lic);
		//	_mainSvc->GetBagSvc()->RoleGetItem(it2->roleID,lic);
	}
}


//@brief	��pk��������ȡ��Ϣ
//	��ȡ����״̬�б�
//	��ȡpk����
/*
//@return ��
void PKSvcSS::GetInfoFromPkEnd( List<ArchvPKEndInfo>& lei, List<ArchvCreatureStatus>& lcs, list<PKEndInfo>& lpkei ,UInt32 mapID,Byte WinOpposition)
{
	List<ArchvPKEndInfo>::iterator it;
	bool hasRole1 = false;		//��Ӫ1 �н�ɫ
	bool hasRole2 = false;		//��Ӫ2 �н�ɫ
	Byte pkType = 0;
	UInt32 i;
	Monster monster;
	ArchvCreatureStatus cs;
	PKEndInfo pkei;

	for( it = lei.begin(); it != lei.end(); it++ )
	{
			//��Ӫ�жϣ�û�п��� 3��������4��ȫ�жԹ�ϵ
			if( 1 == it->creatureFlag )//�ǽ�ɫ
			{

				//lev=_mainSvc->GetCoreData()->ProcessGetRole(it->roleID).Level();
			//	levs.push_back(lev);
				if( 1 == it->opposition )
					hasRole1 = true;

				if( 2 == it->opposition )
					hasRole2 = true;
			}
			if(it->creatureFlag==2)//���������PKû�п���,1vs1
			{
				monster = _mainSvc->GetCoreData()->ProcessGetMonster(mapID,it->roleID);
			}
	}
	//pkType
	if( hasRole1 && hasRole2 )
		pkei.pkr.pkType = pkType = 2;			//PVP
	else
		pkei.pkr.pkType = pkType = 1;			//PVE

	//����״̬�Ļ�ȡ
	lcs.clear();
	for( it = lei.begin(); it != lei.end(); it++ )
	{
		//����״̬
		cs.creatureFlag = it->creatureFlag;
		cs.creatureType = it->creatureType;
		cs.ID = it->roleID;
		LOG(LOG_ERROR,__FILE__,__LINE__,"the creaturestatue ::roleID=%d {}status =%d",it->roleID,it->live);
		if( 1 == it->live )
			cs.status = 1;					//����,δս��
		else if( 2 == it->live )
			cs.status = 0;					//��

		lcs.push_back(cs);

		//��ɫ��Ϣ
		if( 1 == it->creatureFlag )
		{
			if(pkType==1)//PVE
			{
//				pkei.pkr.exp=GettheRandNum(monster.MinExp,monster.MaxExp);//����
	//			pkei.pkr.money=GettheRandNum(monster.MinMoney,monster.MaxMoney);//Ǯ
				pkei.pkr.glory=0;//����
				pkei.pkr.items =" ����ҩˮ*20;����ָ��*1 ";//��Ʒ�Ļ�ò�֪����ô��
			}
			else
			{//pvpֻ�ܻ������ֵ
				pkei.pkr.glory=0;//����
			}
 			//��ȡ��ɫpk������Ϣ
			pkei.roleID = it->roleID;
			pkei.pkei = *it;
			//���ݴ���
			i=_mainSvc->GetCoreData()->RoleExpAdd(it->roleID,pkei.pkr.exp);//�����Ƿ�����
			if(i==1)
			{
			//�����������������㲥
				_mainSvc->GetRoleInfoSvc()->Notifyinfo(it->roleID);
			}
			_mainSvc->GetBagSvc()->Getmoney(it->roleID,pkei.pkr.money);
			_mainSvc->GetCoreData()->RolePKset(it->roleID,it->hp,it->mp);
			_mainSvc->GetCoreData()->RoleAddGlory(it->roleID,pkei.pkr.glory);//����ֵ���Ż�
			//������Ʒ
			lpkei.push_back(pkei);
		}

	}


}

*/
void PKSvcSS::GetInfoFromPkEnd2( List<ArchvPKEndInfo>& lei, List<ArchvCreatureStatus>& lcs, list<PKEndInfo>& lpkei ,UInt32 mapID,UInt32 pkID)
{


		List<ArchvPKEndInfo>::iterator it;
		Byte flag=0;//Ĭ����pve
		Byte pkType = 0;
		UInt32 i;
		Monster monster;
		ArchvCreatureStatus cs;
		PKEndInfo pkei;
		list<UInt32> winers;//Ӯ��һ����ID
		list<UInt32> failer;
		list<UInt32>::iterator itor;
		list<RoleEquipToPk> roleEquiplist;
		list<PetInfoToPk> PetInfolist;
		ArchvCreatureStatus lscinfo;
		RoleEquipToPk roleEquip;
		PetInfoToPk PetInfo;
		PKBriefList pklist;
		//
		for(it=lei.begin();it!=lei.end();it++)//�ֿ���ɫ
		{
			if(it->opposition!=1)//����winer
			{
				//����
					//
					if(it->creatureFlag==1)//��ɫ
					{
						roleEquip.RoleID=it->roleID;
						lscinfo.creatureFlag=it->creatureFlag;
						lscinfo.creatureType=it->creatureType;
						lscinfo.ID=it->roleID;
						lscinfo.status=1;
						lcs.push_back(lscinfo);

						if(it->live==2)
						{
							roleEquip.type=0;//������
						}
						else
						{
							roleEquip.type=1;
						}
						roleEquiplist.push_back(roleEquip);//��������Ӯ�˵Ľ�ɫ���������;õ�������

						RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(it->roleID);

						pRole->RolePKset(it->hp,it->mp);


						winers.push_back(it->roleID);
					}
					else if(it->creatureFlag==4)//����
					{
						PetInfo.Hp=it->hp;
						PetInfo.Mp=it->mp;
						PetInfo.PetID=it->roleID;
						SetPetHPMPFullAfterPk(it->roleID);

					}
				//��ý�����

			}
			else
			{
				//����
				flag=1;//pvp,���˵���
					if(it->creatureFlag==1)//��ɫ
					{
						roleEquip.RoleID=it->roleID;
						roleEquip.type=2;//û��������
						roleEquiplist.push_back(roleEquip);



						roleEquip.RoleID=it->roleID;
						lscinfo.creatureFlag=it->creatureFlag;
						lscinfo.creatureType=it->creatureType;
						lscinfo.ID=it->roleID;
						lscinfo.status=1;
						lcs.push_back(lscinfo);
						failer.push_back(it->roleID);

					}
					else if(it->creatureFlag==4)//����
					{
						PetInfo.Hp=it->hp;
						PetInfo.Mp=it->mp;
						PetInfo.PetID=it->roleID;
					}


			}//end else

		}//end for



			PKEndInfo lrpk;
			List<ArchvPKEndInfo>::iterator itor4;
			if(winers.size()==0)
			{
				//PVE���н������鿴ս��

				pklist=_mainSvc->GetCoreData()->GetPkInfo(pkID);
				if(pkID!=pklist.pkID)
				{
					LOG(LOG_ERROR,__FILE__,__LINE__,"PKID erro! pkID is %d !!!!",pkID);
					//
				}
				//
				list<SenceMonster> monsters;

				for(itor=pklist.player2.begin();itor!=pklist.player2.end();itor++)
				{
					_mainSvc->GetCoreData()->GetSenceMonster(mapID,*itor,monsters);
					lscinfo.creatureFlag=2;
					lscinfo.creatureType=(*itor%100000000)/10000;
					lscinfo.ID=*itor;
					lscinfo.status=1;//��û��
					lcs.push_back(lscinfo);
				}
					for( itor4=lei.begin();itor4!=lei.end();itor4++)
					{
						if(itor4->opposition!=1&&itor4->creatureFlag==1)
						{
							lrpk.roleID=itor4->roleID;
							lrpk.pkei=*itor4;
							lrpk.pkei.live=2;//��Ȼ����
							lrpk.pkr.exp=0;
							lrpk.pkr.money=0;
							lrpk.pkr.pkType=1;//PVP

							lpkei.push_back(lrpk);
							break;
						}
						else
						{
							lrpk.roleID=itor4->roleID;
							lrpk.pkei=*itor4;
							lrpk.pkei.live=1;//��Ȼ����

							lrpk.pkr.exp=0;
							lrpk.pkr.money=0;
							lrpk.pkr.pkType=2;//PVP

							lpkei.push_back(lrpk);
						}
				}
		}


}


void PKSvcSS::GetInfoFromPkEnd1( List<ArchvPKEndInfo>& lei,List<ArchvCreatureStatus>& lcs,list<PKEndInfo>& lpkei,UInt32 mapID,UInt32 PkID)
{
	List<ArchvPKEndInfo>::iterator it;
	Byte flag=0;//Ĭ����pve
	Byte pkType = 0;
	UInt32 i;
	Monster monster;
	ArchvCreatureStatus cs;
	PKEndInfo pkei;
	list<UInt32> winers;//Ӯ��һ����ID
	list<UInt32> failer;
	list<UInt32>::iterator itor;
	list<RoleEquipToPk>	roleEquiplist;
	list<PetInfoToPk> PetInfolist;
	ArchvCreatureStatus lscinfo;
	RoleEquipToPk roleEquip;
	PetInfoToPk PetInfo;
	PKBriefList pklist;
	list<UInt32> winPets;
	//
	for(it=lei.begin();it!=lei.end();it++)//�ֿ���ɫ
	{
//		LOG(LOG_ERROR,__FILE__,__LINE__,"PKID OK!! creature ID is [%d]!",it->roleID);
		if(it->opposition==1)//������Ӫ1
		{
			//����
				//
				if(it->creatureFlag==1)//��ɫ
				{
					roleEquip.RoleID=it->roleID;
					lscinfo.creatureFlag=it->creatureFlag;
					lscinfo.creatureType=it->creatureType;
					lscinfo.ID=it->roleID;
					lscinfo.status=1;
					lcs.push_back(lscinfo);

					if(it->live==2)
					{
						roleEquip.type=0;//������
					}
					else
					{
						roleEquip.type=1;
					}
					roleEquiplist.push_back(roleEquip);//��������Ӯ�˵Ľ�ɫ���������;õ�������


					RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(it->roleID);

					pRole->RolePKset(it->hp,it->mp);


					winers.push_back(it->roleID);
				}
				else if(it->creatureFlag==4)//����
				{
					PetInfo.Hp=it->hp;
					PetInfo.Mp=it->mp;
					PetInfo.PetID=it->roleID;
					//SetPetHPMPFullAfterPk(it->roleID);

					winPets.push_back(it->roleID);
				}
			//��ý�����

		}
		else
		{
			//����
			flag=1;//pvp,���˵���
				if(it->creatureFlag==1)//��ɫ
				{
					roleEquip.RoleID=it->roleID;
					roleEquip.type=2;//û��������
					roleEquiplist.push_back(roleEquip);



					roleEquip.RoleID=it->roleID;
					lscinfo.creatureFlag=it->creatureFlag;
					lscinfo.creatureType=it->creatureType;
					lscinfo.ID=it->roleID;
					lscinfo.status=1;
					lcs.push_back(lscinfo);
					failer.push_back(it->roleID);

				}
				else if(it->creatureFlag==4)//����
				{
					PetInfo.Hp=it->hp;
					PetInfo.Mp=it->mp;
					PetInfo.PetID=it->roleID;

					SetPetHPMPFullAfterPk(it->roleID);
				}


		}//end else

	}//end for

		if(failer.size()==0)
		{
			//PVE���н������鿴ս��
 			pklist=_mainSvc->GetCoreData()->GetPkInfo(PkID);
			if(PkID!=pklist.pkID)
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"PKID erro! pkID is %d !!!!",PkID);
				//
			}
			//
			list<SenceMonster> monsters;

			for(itor=pklist.player2.begin();itor!=pklist.player2.end();itor++)
			{
				_mainSvc->GetCoreData()->GetSenceMonster(mapID,*itor,monsters);
				lscinfo.creatureFlag=2;
				lscinfo.creatureType=(*itor%100000000)/10000;
				lscinfo.ID=*itor;
				lscinfo.status=0;//������
				lcs.push_back(lscinfo);
			}

			GetItemFromPkEnd(winers,lei,monsters,lpkei, mapID);//��Ʒ����

			WinPetAddExp(winPets, lpkei);
		}
		//ȥ�����ս���Ĵ洢



}//end

UInt32 PKSvcSS::findtheLev(list<RoleInfopk>& lis,UInt32 roleID)
{
	list<RoleInfopk>::iterator it;
	for(it=lis.begin();it!=lis.end();it++)
	{
		if(it->RoleID==roleID)
		return it->Level;
	}
	return 0;
}



void PKSvcSS::AddExpByExpRune(List<ArchvPKEndInfo>::iterator & itor, PKEndInfo & lrpk)
{
	//ÿһ����ʹ�÷��� 2Сʱ��Ч��6002�Ǿ������ID

	RolePtr pRole = _mainSvc->GetCoreData()->ProcessGetRolePtr(itor->roleID);
    if(time(NULL) - pRole->GetRoleRuneTime(6002) < 2 * 3600){
        lrpk.pkr.exp = UInt32(lrpk.pkr.exp*1.5); //Ŀǰ����50%
    }
}

void PKSvcSS::GetItemFromPkEnd(list<UInt32>& roleIDs,List<ArchvPKEndInfo>& lei,list<SenceMonster>& monsters,list<PKEndInfo>& lpkei,UInt32 mapID)
{
	UInt32 Totallelev=0;
	UInt32 monsterlev=0;
	UInt32 Lev=0;
	list<SenceMonster>::iterator itor1;

	list<UInt32>::iterator itor;
	list<RoleInfopk>::iterator itor2;
	List<ArchvPKEndInfo>::iterator itor4;
	list<RoleInfopk> roleLev;
	list<ItemList>::iterator itor3;
	ItemList taskDrop;
	List<ArchvUnfinishedTask>licUnfinishTask;  //  ��ɫ�ѽ�δ�������
	List<ArchvUnfinishedTask>::iterator taskIter;
	list<UInt32>licTaskID;
	list<UInt32>::iterator result;
	map<UInt32,ItemList>::iterator mapIter;
	list<ItemList> publicdrop;
	List<UInt32> items;
	UInt32 numnum=0;
	PKEndInfo lrpk;
	RoleInfopk roleLev1;
	double expdouble=0.0,extraExp = 0.0;
	UInt32 money=0,extraMoney = 0;
	char Publicdrop=0;
	UInt32 ItemID;
	List<UInt32> a[5];
	int i=0;
	//////test
	List<ArchvUnfinishedTask>::iterator tst;
	list<UInt32>::iterator tstiter;
	list<UInt32>taskid;
	/////


	List<VipRole>licVipRole;   //vip pk �о���ӳɺͽ�Ǯ�ӳ�
	VipRole viprole;
	List<VipRole>::iterator vipIter;
	Byte isVip = 0;
	int iRet = 0;
	UInt32 vipMoney = 0,vipExp = 0;

	srand((unsigned)(time(NULL)));
	for(itor=roleIDs.begin();itor!=roleIDs.end();itor++)
	{

			roleLev1.Level=_mainSvc->GetCoreData()->ProcessGetRolePtr(*itor)->Level();
			Totallelev=roleLev1.Level+Totallelev;
			roleLev1.RoleID=*itor;
			isVip = _mainSvc->GetCoreData()->ProcessGetRolePtr(*itor)->VIP();
			if(isVip >=1 && isVip <= 6)
			{
				viprole.isVip = isVip;
				viprole.roleID = *itor;
				licVipRole.push_back(viprole);
			}
			roleLev.push_back(roleLev1);
	}

	_mainSvc->GetTaskSvc()->GetRoleUnFinishedTask(roleIDs,licUnfinishTask);//��ȡpk ʤ������������Ϣ
		for(tst = licUnfinishTask.begin();tst != licUnfinishTask.end();tst++)
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"roleID[%d]--",tst->roleID);
			taskid = tst->licTaskID;
			for(tstiter = taskid.begin(); tstiter != taskid.end(); tstiter++)
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"taskID[%d]--",*tstiter);
			}
		}

			Monster monster;
			int randnum;
			for(itor1=monsters.begin();itor1!=monsters.end();itor1++)
			{
						DealKillMonsterTask(roleIDs,itor1->MonsterType,itor1->num);//ɱ������
						monster=_mainSvc->GetCoreData()->ProcessGetMonster(mapID,itor1->MonsterType);
						monsterlev=monster.Level();
						expdouble=expdouble+itor1->num*monster.Exp/(pow(abs((int)(Totallelev/roleIDs.size()-monsterlev)+1),1.0/3.0));
						money=money+itor1->num*monster.Money;


						//�������
					    for(mapIter = monster.ItemDropTask.begin();mapIter != monster.ItemDropTask.end();mapIter++)
					    {
							//LOG(LOG_ERROR,__FILE__,__LINE__,"task item loss--taskid[%d]",mapIter->first);
							if(licUnfinishTask.size() == 0)
							     break;                        //û��������Ʒ����

							taskDrop = mapIter->second;
							for(taskIter = licUnfinishTask.begin();taskIter != licUnfinishTask.end();taskIter++)
							{
								licTaskID = taskIter->licTaskID;
								result = find(licTaskID.begin(),licTaskID.end(),mapIter->first);
								if(result != licTaskID.end())   //�Ƿ����ɱ������
								{
									randnum = rand() % 10000;
								    if(randnum < (taskDrop.num)*100)
									{
										items.push_back(taskDrop.ItemID);
										LOG(LOG_ERROR,__FILE__,__LINE__,"task item loss--taskID[%d]-itemid[%d]",mapIter->first,taskDrop.ItemID);
									}
								}


							}

						}
						//ר������
						/*for(itor3=monster.ItemDropSpec.begin();itor3!=monster.ItemDropSpec.end();itor3++)
						{
							randnum=rand()%10000;
							if(randnum<(itor3->num)*100)
							{
								items.push_back(itor3->ItemID);
								LOG(LOG_ERROR,__FILE__,__LINE__,"Special item loss--itemid[%d]",itor3->ItemID);

							}
						}


						publicdrop=_mainSvc->GetCoreData()->GetPublicDrop(mapID,monster.ItemDropPublic);
						//��������
						for(itor3=publicdrop.begin();itor3!=publicdrop.end();itor3++)
						{
							randnum=rand()%10000;
							if(randnum<(itor3->num)*100)
							{
								items.push_back(itor3->ItemID);
								LOG(LOG_ERROR,__FILE__,__LINE__,"public Item loss----- itemid[%d]",itor3->ItemID);
							}
						}*/

			}
			numnum=roleLev.size();

			if(numnum>=5)
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"Lev is erro Rolenum is %d  !!!!",numnum);
			}
			if(numnum!=1)
			{
				for(itor=items.begin();itor!=items.end();itor++)
				{
					//	lrpk.pkr.items
						randnum=rand()%numnum;
						a[randnum].push_back(*itor);

//							LOG(LOG_ERROR,__FILE__,__LINE__,"Lev is erro RoleID is  !!!!",itor4->roleID);
				}
			}
			//����һ���ˣ�װ�������������
			i=0;
			for( itor4=lei.begin();itor4!=lei.end();itor4++)
			{
				if(itor4->opposition==1&&itor4->creatureFlag==1) //1Ϊ����1�� 1Ϊ��ɫ��
				{
						if(roleLev.size()==1)
						{
							lrpk.roleID=itor4->roleID;
							lrpk.pkei=*itor4;
							lrpk.pkei.live=1;//��Ȼ����
							lrpk.pkr.exp=(UInt32)(expdouble);
						    lrpk.pkr.money = money;
						    lrpk.pkr.pkType = 1; //PVP
						    lrpk.pkr.items = items;

							vipMoney = lrpk.pkr.money;
							vipExp = lrpk.pkr.exp;
							LOG(LOG_ERROR,__FILE__,__LINE__,"-----money[%d]-----exp[%d]",money,lrpk.pkr.exp);
							iRet = VipExtraExpAndMoney(itor4->roleID,vipMoney,vipExp,licVipRole);
							if(iRet)
							{
								_mainSvc->GetBagSvc()->GetBindMoney(itor4->roleID, vipMoney);
								_mainSvc->GetCoreData()->RoleExpAdd(itor4->roleID,vipExp);
								lrpk.pkr.money = vipMoney;
								lrpk.pkr.exp = vipExp;
								LOG(LOG_ERROR,__FILE__,__LINE__,"role[%d]-vipMoney[%d]-vipExp[%d]",lrpk.roleID,vipMoney,vipExp);
							}
							else
							{
								_mainSvc->GetBagSvc()->GetBindMoney(itor4->roleID, money);
								_mainSvc->GetCoreData()->RoleExpAdd(itor4->roleID,lrpk.pkr.exp);
								LOG(LOG_ERROR,__FILE__,__LINE__,"role[%d]-money[%d]-exp[%d]",lrpk.roleID,money,lrpk.pkr.exp);

							}
							//_mainSvc->GetBagSvc()->GetBindMoney(itor4->roleID, money);
						    AddExpByExpRune(itor4, lrpk);
						    //_mainSvc->GetCoreData()->RoleExpAdd(itor4->roleID,lrpk.pkr.exp);
							//������Ʒȫ������
							_mainSvc->GetBagSvc()->RoleGetItem(itor4->roleID,lrpk.pkr.items);

							lpkei.push_back(lrpk);
							break;
						}
						else
						{
							lrpk.roleID=itor4->roleID;
							lrpk.pkei=*itor4;
							lrpk.pkei.live=1;//��Ȼ����
							Lev=findtheLev(roleLev,itor4->roleID);
							if(Lev==0)
							{
									LOG(LOG_ERROR,__FILE__,__LINE__,"Lev is erro RoleID is  %d!!!!",itor4->roleID);
							}
							lrpk.pkr.exp=(UInt32)(expdouble*4*Totallelev/(Lev*5*roleLev.size()));
							lrpk.pkr.money=money/roleLev.size();
							lrpk.pkr.pkType=1;//PVP
							lrpk.pkr.items=a[i];
							i++;
							//����
							vipMoney = lrpk.pkr.money;
							vipExp = lrpk.pkr.exp;
							iRet = VipExtraExpAndMoney(itor4->roleID,vipMoney,vipExp,licVipRole);
							if(iRet)
							{
								_mainSvc->GetBagSvc()->GetBindMoney(itor4->roleID, vipMoney);
								_mainSvc->GetCoreData()->RoleExpAdd(itor4->roleID,vipExp);
								lrpk.pkr.money = vipMoney;
								lrpk.pkr.exp = vipExp;
							}
							else
							{
								_mainSvc->GetBagSvc()->GetBindMoney(itor4->roleID, money);
								_mainSvc->GetCoreData()->RoleExpAdd(itor4->roleID,lrpk.pkr.exp);

							}

							//_mainSvc->GetBagSvc()->GetBindMoney(itor4->roleID,lrpk.pkr.money);
							//_mainSvc->GetCoreData()->RoleExpAdd(itor4->roleID,lrpk.pkr.exp);
							_mainSvc->GetBagSvc()->RoleGetItem(itor4->roleID,lrpk.pkr.items);
							//LOG(LOG_ERROR,__FILE__,__LINE__,"")
							lpkei.push_back(lrpk);
						}
				}
			}


}

void PKSvcSS::ProcessPKEnd(Session& session,Packet& packet)
{
	UInt32	RetCode = 0;
	DataBuffer	serbuffer(2048);
	UInt32 pkID=0;
	UInt32 mapID = 0;
	UInt16 tmpMapID = 0;
	Byte winOpposition = 0;
	List<ArchvPKEndInfo> lei;
	List<ArchvPKEndInfo>::iterator it;
	List<ArchvCreatureStatus> lcs;
	list<PKEndInfo> lpkei;

	int iRet = 0;
	Connection con;
	DBOperate dbo;

 	//���л���
	Serializer s(packet.GetBuffer());
	s>>pkID>>tmpMapID>>winOpposition>>lei;

	if( s.GetErrorCode()!= 0 )
	{
		RetCode = ERR_SYSTEM_SERERROR;
		LOG(LOG_ERROR,__FILE__,__LINE__,"serial error" );
		goto EndOf_Process;
	}

	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	mapID = tmpMapID;

DEBUG_PRINTF1( " ProcessPKEnd--winOpposition[%d] \n", winOpposition );

	//��¼��־,���� debug
	LOG(LOG_DEBUG,__FILE__,__LINE__,"ProcessPKEnd ok!!pkID[%d],mapID[%d],winOpposition[%d]", pkID, tmpMapID, winOpposition  );
	LOG(LOG_DEBUG,__FILE__,__LINE__,"PKEndInfo :______"  );
	for( it = lei.begin(); it != lei.end(); ++it )
	{
		LOG(LOG_DEBUG,__FILE__,__LINE__,"CreatureFlag[%d],CreatureType[%d],RoleID[%d],Opposition[%d],Live[%d],HP[%d],MP[%d]",
			it->creatureFlag, it->creatureType, it->roleID, it->opposition,
			it->live, it->hp, it->mp);
	}

	//��pk��������ȡ��Ϣ
	if(winOpposition!=1) //����1
	{
		GetInfoFromPkEnd2(lei,lcs,lpkei,mapID,pkID);
		//pk1����
		//���ùֹ��ʲô��û�б䣬�е��ǳͷ�,�鿴�����������PVP OR PVE
	}
	else
	{
		GetInfoFromPkEnd1( lei, lcs, lpkei ,mapID,pkID);
	}


	// ս��������ɫװ���;ö���ģ��μ�ս���Ľ�ɫ����װ���;ö����
	for(it = lei.begin(); it != lei.end(); it++)
	{
		if(it->creatureFlag == 1 && it->live == 1)	 //���pve��pvp:ս����������ɫ���ţ�װ���;ö���� 1
		{
			iRet = _mainSvc->GetAvatarSvc()->OnEquipDurabilityLoss(it->roleID, 1);
			if(iRet)
			{
			   RetCode = ERR_SYSTEM_SERERROR;
			   LOG(LOG_ERROR,__FILE__,__LINE__,"OnEquipDurabilityLoss Failed ! role[%d]",it->roleID);
			   goto EndOf_Process;
			}
		}
		if(it->creatureFlag == 1 && it->live == 2 ) //���pve:ս����������ɫ������װ���;ö���� 10
		{
			iRet = _mainSvc->GetAvatarSvc()->OnEquipDurabilityLoss(it->roleID, 10);
			if(iRet)
			{
			  RetCode = ERR_APP_OP;
			  LOG(LOG_ERROR,__FILE__,__LINE__,"OnEquipDurabilityLoss Failed ! role[%d]",it->roleID);
			  goto EndOf_Process;
			}
		}
		/*if ()//���pvp(���ڻ�û��):ս����������ɫ������װ���;ö���� 5
		{}*/

	}



	_mainSvc->GetCoreData()->DeletePkBrif(pkID);
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
		//����pk�����㲥
		_mainSvc->GetPkSvc()->NotifyPKEnd(lpkei);

		for( it = lei.begin(); it != lei.end(); ++it )
		{
			_mainSvc->GetBagSvc()->NotifyHPandMP(it->roleID);
			_mainSvc->GetRoleInfoSvc()->NotifyExp(it->roleID);

//			LOG(LOG_DEBUG,__FILE__,__LINE__,"2222222  HP[%d] MP[%d] Exp[%d] ", hp,mp,exp);
		}



		//�޸�����״̬
		_mainSvc->GetCoreData()->ChangeCreatureStatus( mapID, lcs);

		//����ɱ��֪ͨ
//		DealKillMonsterTask( lpkei, lcs );

	}

}



void PKSvcSS::UpdateThedurability(list<RoleEquipToPk> roletype)//�仯��Hp��MP
{
		list<RoleEquipToPk>::iterator itor;
		char szSql[1024];
		char szcat2[128];
		char szcat[128];
		char szcat1[128];
		Connection con;
		DBOperate dbo;
		int iRet = 0;
		//��ȡDB����
		con = _cp->GetConnection();
		dbo.SetHandle(con.GetHandle());
		sprintf( szcat1, "0");
		sprintf( szcat2, "0");

		for(itor=roletype.begin();itor!=roletype.end();itor++)
		{

			if(itor->type==1)
			{
				sprintf( szcat, ",%d");
				strcat(szcat1,szcat);
			}
			else if(itor->type==2)
			{
				sprintf( szcat, ",%d");
				strcat(szcat2,szcat);
			}
			else
			{
//				sprintf()
			}
		}
		sprintf( szcat, "));");
		strcat(szcat1,szcat);
		strcat(szcat2,szcat);
		sprintf( szSql, "update Entity set Durability=Durability-1 where Durability>1 and EntityID in(select EntityID from Equip where RoleID in (");
		strcat(szSql,szcat1);
		iRet=dbo.ExceSQL(szSql);
		if(iRet!=0)
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		}

		sprintf( szSql, "update Entity set Durability=Durability-0.1*(select Durability from Item 	where Item.ItemID=Entity.ItemID)	where EntityID in \
								(select EntityID from Equip where RoleID in (");
		strcat(szSql,szcat2);
		iRet=dbo.ExceSQL(szSql);
		if(iRet!=0)
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		}
		sprintf( szSql, "update Entity set Durability=0 where Durability>1000 \
							and EntityID in(select EntityID from Equip where RoleID in (");
		strcat(szSql,szcat2);
		iRet=dbo.ExceSQL(szSql);
		if(iRet!=0)
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		}


}

//@param: roleid  ��ɫid
//@param: extraExp   vip��õĶ��⾭��
//@param: extraMoney   vip ��õĶ����Ǯ
//@param:licRoleVip     pkʤ��������vip �Ľ�ɫ
//@brief: vip ��pkʤ�����ö��⾭��ͽ�Ǯ
int PKSvcSS::VipExtraExpAndMoney(UInt32 roleID,UInt32 &Exp,UInt32 &Money,List<VipRole>licRoleVip)
{
	List<VipRole>::iterator vipIter;
	int exp = 0,money = 0;
	int flag = 0;

	for(vipIter = licRoleVip.begin();vipIter != licRoleVip.end(); vipIter++)
	{
		if(roleID == vipIter->roleID)
		{
			if(vipIter->isVip == 1)
			{
				Exp = (int)(1.2 * Exp);
				Money = (int)(1.2 * Money);
			}
			else if(vipIter->isVip == 2)
			{
				Exp = (int)(1.3 * Exp);
				Money = (int)(1.3 * Money);
			}
			else if(vipIter->isVip == 3)
			{
				Exp = (int)(1.4 * Exp);
				Money = (int)(1.4 * Money);
			}
			else
			{
				Exp = (int)(1.5 * Exp);
				Money = (int)(1.5 * Money);
			}

			flag = 1;
			break;
		}


	}

	return flag;
}


void PKSvcSS::UpdatePet(list<PetInfoToPk> petinfo)
{
	list<PetInfoToPk>::iterator itor;
		char szSql[1024];
		Connection con;
		DBOperate dbo;
		int iRet = 0;
		//��ȡDB����
		con = _cp->GetConnection();
		dbo.SetHandle(con.GetHandle());
	for(itor=petinfo.begin();itor!=petinfo.end();itor++)
	{
		//���³�������
	}
}


void PKSvcSS::SetPetHPMPFullAfterPk(int petID)
{
	Connection con;
	DBOperate dbo;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	//
	char szSql[1024];
	sprintf(szSql, "update Pet set HP = MaxHP, MP = MaxMP where PetID = %d;", petID);
	int iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return ;
 	}

	_mainSvc->GetPetSvc()->NotifyPetAttrChange(petID);
}

void PKSvcSS::WinPetAddExp(const list<UInt32> &winPets, const list<PKEndInfo> &lpkei)
{
	Connection con;
	DBOperate dbo;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	char szSql[1024];

	for (list<UInt32>::const_iterator petIter = winPets.begin(); petIter != winPets.end(); ++petIter)
	{
		sprintf(szSql, "select RoleID from Pet where PetID = %d;", *petIter);
		int iRet = dbo.QuerySQL(szSql);
		if (iRet == 1)
		{
			LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL pet not have role[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
			continue;
		}

		int roleId = 0;
		while (dbo.HasRowData())
		{
			roleId = dbo.GetIntField(0);
			dbo.NextRow();
		}

		for (list<PKEndInfo>::const_iterator endInfoIter = lpkei.begin(); endInfoIter != lpkei.end(); ++endInfoIter)
		{
			if (endInfoIter->roleID == roleId)
			{
				int roleLevel = _mainSvc->GetCoreData()->ProcessGetRolePtr(endInfoIter->roleID)->Level();
				PetAddExp(*petIter, endInfoIter->pkr.exp, roleLevel);
				break;
			}
		}
	}
}

void PKSvcSS::PetAddExp(int petID, int expAdd, int maxLevel)
{
	Connection con;
	DBOperate dbo;
	con = _cp->GetConnection();
	dbo.SetHandle(con.GetHandle());

	char szSql[1024];
	sprintf(szSql, "select Level, Exp, MaxExp, Strength,Intelligence,\
		Agility, AttackPowerHigh, AttackPowerLow from Pet where PetID =%d;",petID);
	int	iRet=dbo.QuerySQL(szSql);
	if( 1 == iRet )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
		return ;
	}
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return ;
 	}

	int level = 0;
	int exp = 0;
	int MaxExp = 0;
	int strength = 0;
	int intelligence = 0;
	int agility = 0;
	int attackPowerHigh = 0;
	int attackPowerLow = 0;
	while (dbo.HasRowData())
	{
		level = dbo.GetIntField(0);
		exp = dbo.GetIntField(1);
		MaxExp = dbo.GetIntField(2);
		strength = dbo.GetIntField(3);
		intelligence = dbo.GetIntField(4);
		agility = dbo.GetIntField(5);
		attackPowerHigh = dbo.GetIntField(6);
		attackPowerLow = dbo.GetIntField(7);
		dbo.NextRow();
	}

	if (exp + expAdd >= MaxExp)
	{
		bool bLevelUp = (level + 1) <= maxLevel ? true : false;
		if (bLevelUp)
		{
			exp = exp + expAdd - MaxExp;
			level = level +1;
			sprintf(szSql, "select MaxExp from LevelDesc where Level=%d;",level+1);
			int	iRet=dbo.QuerySQL(szSql);
			if( 1 == iRet )
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL data not found ,szSql[%s] " , szSql);
				return ;
			}
			if( iRet < 0 )
			{
				LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
				return ;
 			}
			while(dbo.HasRowData())
			{

				MaxExp = dbo.GetIntField(0) /3;
				dbo.NextRow();
			}
			if (exp >= MaxExp)
			{
				exp = MaxExp - 2;
			}
			strength += 1;
			intelligence += 1;
			agility +=1;
			attackPowerHigh += 2;
			attackPowerLow += 2;
		}
		else
		{
			// ����ȼ����ܸ�������ȼ�����ʱ�Ȱѳ��ﾭ��������������2
			exp = MaxExp - 2;
		}
	}
	else
	{
		exp = exp + expAdd;
	}

	sprintf(szSql, "update Pet set Level = %d, Exp = %d, MaxExp = %d, \
		Strength = %d,Intelligence = %d,\
		Agility = %d, AttackPowerHigh = %d, AttackPowerLow = %d where PetID = %d;",
		level, exp, MaxExp,strength, intelligence,agility,attackPowerHigh,attackPowerLow,petID);
	iRet = dbo.ExceSQL(szSql);
	if( iRet < 0 )
	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"QuerySQL error[%s],szSql[%s] ", mysql_error(con.GetHandle()), szSql);
		return ;
 	}

	_mainSvc->GetPetSvc()->NotifyPetAttrChange(petID);

}



