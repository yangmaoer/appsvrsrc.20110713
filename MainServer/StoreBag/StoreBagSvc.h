//ҵ���� server   �������

#ifndef STOREBAGSVC_H
#define STOREBAGSVC_H
//#define TOPCELL_NUM 25
#include "GWProxy.h"
#include "IniFile.h"
#include "ConnectionPool.h"
#include "ArchvBagItemCell.h"


class MainSvc;
 
class StoreBagSvc
{
public:
	StoreBagSvc(void* service, ConnectionPool * cp);
	~StoreBagSvc();
	void SetService( MainSvc * svc );

 	void OnProcessPacket(Session& session,Packet& packet);

	void ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode);



	//===================��ҵ��====================================
	
	
	void ProcessGetItem(Session& session,Packet& packet);//msgtype 1101 ��ѯ�ֿ���Ʒ�б�
	
	void ProcessMoveoutItem(Session& session,Packet& packet);//ȡ���ֿⶫ��  1102
	
	void ProcessDropItem(Session & session,Packet & packet);//��Ʒ������
	
	void ProcessSortItem(Session & session,Packet & packet);//��Ʒ����
	
	void ProcesschangeItem(Session & session,Packet & packet);//��Ʒλ�ý���

    void ProcessAddTopCell(Session & session,Packet & packet);//�ı�ֿ�����

	UInt32 GetRoleCellNum(UInt32 roleID);//��õ�Ԫ������

	//===================��ҵ��=======================================

	
	//---------------------s-c�Ĺ㲥----------------------------------

	
	void NotifyStoreBag(UInt32 roleID,List<ItemCell>& lic);


private:
	MainSvc * _mainSvc;
	IniFile _file;
	ConnectionPool *_cp;

};


#endif

