//ҵ���� server   Npc���

#ifndef NPCSVC_H
#define NPCSVC_H

#include "GWProxy.h"
#include "IniFile.h"
#include "ConnectionPool.h"
#include "LuaState.h"

class MainSvc;
 
class NpcSvc
{
public:
	NpcSvc(void* service, ConnectionPool * cp);
	~NpcSvc();

 	void OnProcessPacket(Session& session,Packet& packet);

	void ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode);

	int GetNpcStatus( UInt32 roleID, UInt32 level, UInt32 npcID, Byte& status );

	
	//------------S_C ����-----------------------------------------


	//------------C_S ����-----------------------------------------
	//msgtype 601 Npc�Ի� 
	void ProcessNpcTalk(Session& session,Packet& packet);

	//msgtype 602 Npc�̵���Ʒ��ѯ
	void ProcessGetNpcShopItem(Session& session,Packet& packet);


	//------------��ҵ���� -----------------------------------------
private:
	int ExecCanAcceptScript( UInt32 roleID, const string& script );

private:
	MainSvc * _mainSvc;
	IniFile _file;
	ConnectionPool *_cp;

	LuaState _luaState;
	
};


#endif

