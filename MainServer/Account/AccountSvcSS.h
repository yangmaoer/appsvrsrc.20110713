// S_S ҵ����  pk
//

#ifndef ACCOUNTSVCSS_H
#define ACCOUNTSVCSS_H

#include "SSServer.h"
#include "ConnectionPool.h"
#include <list>
#include "List.h"
#include "Role.h"

class MainSvc;
class Role;

class AccountSvcSS
{
public:

	AccountSvcSS(MainSvc * mainSvc, ConnectionPool *cp );
	~AccountSvcSS();

	void OnProcessPacket(Session& session,Packet& packet);

protected:

	void ClientErrorAck(Session& session, Packet& packet, UInt32	RetCode);

	//------------S_S ����-----------------------------------------

	//[MsgType:0301] Accountע��
	void	ProcessAccountReg(Session& session,Packet& packet);

	//[MsgType:0302] Account��½
	void	ProcessAccountLogin(Session& session,Packet& packet);

	//[MsgType:0303] ��ѯAccount��Rolesӳ��
	void	ProcessGetAccountRoles(Session& session,Packet& packet);

	//[MsgType:0304] ����Role
	void	ProcessAddRole(Session& session,Packet& packet);

	//[MsgType:0305] ɾ��Role
	void	ProcessDelRole(Session& session,Packet& packet);

	//[MsgType:0306] �����֤֪ͨ����ÿ��C_S��½ǰ���ͣ�
	void	ProcessIdNotify(Session& session,Packet& packet);

	//[MsgType:0307] �����Լ�� 
	void	ProcessPlayTimeCheck(Session& session,Packet& packet);

	//[MsgType:0308] ���֤ע��
	void	ProcessIDCardReg(Session& session,Packet& packet);

	//[MsgType:0399] Session��֤
	void	ProcessCheckSession(Session& session,Packet& packet);

	//------------��ҵ���� -----------------------------------------
	
private:
	int SetProAttr( Byte proID, RolePtr& role );
	int AddRole( UInt32 accountID, const string& passwd, const string& roleName, Byte proID, UInt32& roleID );

private:
	MainSvc * _mainSvc;
	ConnectionPool *_cp;
};


#endif

