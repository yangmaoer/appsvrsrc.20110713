/**
 *	Npc ���л��õ���
 *	
 */

#ifndef ARCHVNPC_H
#define ARCHVNPC_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "DebugData.h"
#include "List.h"






// Npc ��Ϣ
class ArchvNpcInfo
	:public BaseArchive
{
	
public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvNpcInfo():npcID(0),mapID(0),posX(0),posY(0)
	{}
	
public:
	//��Ա����
	UInt32	npcID;
	string	npcName;
	Byte		mapID;
	UInt16	posX;
	UInt16	posY;
	string	greeting;
	
//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(npcID)
		SERIAL_ENTRY(npcName)
		SERIAL_ENTRY(mapID)
		SERIAL_ENTRY(posX)
		SERIAL_ENTRY(posY)
	END_SERIAL_MAP()

};

#endif
