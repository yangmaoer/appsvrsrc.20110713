/**
 *	�˺� ���л��õ���
 *	
 */

#ifndef ARCHVACCOUNT_H
#define ARCHVACCOUNT_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "List.h"

//��ɫ�ƶ�����
class ArchvAccountRoleInfo
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvAccountRoleInfo():roleID(0),proID(0),level(0)
	{
	}
	
	virtual ~ArchvAccountRoleInfo(){}

public:
	//��Ա����
	UInt32				roleID;
	string				roleName;
	Byte					proID;
	UInt32				level;
	
	
//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(roleID)
		SERIAL_ENTRY(roleName)
		SERIAL_ENTRY(proID)
		SERIAL_ENTRY(level)
	END_SERIAL_MAP()

};



#endif


