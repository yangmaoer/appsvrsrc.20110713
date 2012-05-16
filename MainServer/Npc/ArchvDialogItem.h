/**
 *	npc �Ի�����
 *	��һ��� BaseArchive ���ඨ�塢����������ͬ
 */

#ifndef ARCHVDIALOGITEM_H
#define ARCHVDIALOGITEM_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "DebugData.h"
#include "List.h"

#include "ArchvTask.h"
#include "OurDef.h"


class ArchvDialogItem
	:public BaseArchive
{
public:
	ArchvDialogItem();
	
//	ArchvDialogItem(Byte type,void* data);

	ArchvDialogItem(Byte type, UInt32 input);

	ArchvDialogItem(Byte type, const ArchvTaskInfoBrief &input);

	~ArchvDialogItem();

	void FreeData();

	Serializer& Serialize(Serializer &serializer,bool isLoading);

private:
	unsigned char _type;
//	void* _data;

	UInt32	_valueID;
	ArchvTaskInfoBrief	_taskInfoBrief;
	
	
};


#endif
