#include "NpcLua.h"
#include "NpcSvc.h"
#include "Log.h"
#include "../Task/TaskSvc.h"

namespace NpcLua
{

//@brief ע��C API for lua
void RegisterNpcCAPI(lua_State* L)
{
	lua_register(L,"GetRoleTaskStatus",GetRoleTaskStatus);
	
 	lua_settop(L,0);
}

//@brief ��ȡlua �ű�ִ�еķ���ֵ 
//		��������һ����������ֵ�Ľű�
//@return lua �ķ���ֵ
int GetLuaRetCode(lua_State* L)
{
	return lua_tointeger(L,-1);
}


//@brief ��ѯָ����ɫָ�������״̬��Ϣ
//@lua ���ø�ʽ: GetRoleTaskStatus( taskID ) 
//@param-Lua	taskID	����ID
//@return-Lua	status 1 δ�� 2δ��� 3 ����� 4 �ѽ���
int GetRoleTaskStatus(lua_State* L)
{
	TaskSvc * taskSvc;
	UInt32 roleID;
	UInt32 taskID;
	Byte status = 0;
 	int iRet = 0;

	//���
	GET_TASKSVC(L,taskSvc);
	GET_ROLEID(L, roleID);	
	taskID = lua_tointeger(L,1);

 	iRet = taskSvc->GetRoleTaskStatus( roleID, taskID, status );
 	if(iRet)
 	{
		LOG(LOG_ERROR,__FILE__,__LINE__,"GetRoleTaskStatus error!!!!");
		return 0;
	}
	
	lua_pushinteger(L,status);
 
	return 1;
}








}


