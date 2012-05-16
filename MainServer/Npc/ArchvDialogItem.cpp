#include "ArchvDialogItem.h"
#include "DebugData.h"

ArchvDialogItem::ArchvDialogItem()
:_type(0)
,_valueID(0)
{
}
	
ArchvDialogItem::ArchvDialogItem(Byte type, UInt32 input)
:_type(type)
,_valueID(input)
{
}


ArchvDialogItem::ArchvDialogItem(Byte type, const ArchvTaskInfoBrief &input)
:_type(type)
,_valueID(0)
{
	_taskInfoBrief = input;
}

ArchvDialogItem::~ArchvDialogItem()
{
}

/*
void ArchvDialogItem::FreeData()
{
	if(_data)
	{
		switch(_type)
		{
		case 1: //npc �̵�
			delete static_cast<UInt32*>(_data);
			break;
			
		case 2: //����Ի���
			delete static_cast<ArchvTaskInfoBrief*>(_data);
			break;
 		}
	}
}
*/

Serializer& ArchvDialogItem::Serialize(Serializer &serializer,bool isLoading)
{
	if(isLoading)
	{
	}
	else
	{
		switch(_type)
		{
		case 1: //npc �̵�
			serializer<<_type<<_valueID;
			break;
		case 2: //����Ի���
			serializer<<_type<<_taskInfoBrief;
			break;
		}
	}

	return serializer;
}


