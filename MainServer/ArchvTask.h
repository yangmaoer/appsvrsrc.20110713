/**
 *	���� ���л��õ���
 *
 */

#ifndef ARCHVTASK_H
#define ARCHVTASK_H

#include "Serializer.h"
#include "BaseArchive.h"
#include "DebugData.h"
#include "List.h"


//��Ҫ������Ϣ
class ArchvTaskInfoBrief
	:public BaseArchive
{

public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvTaskInfoBrief():taskID(0),taskType(0),taskStatus(0),adviceLevel(0)
	{}
	virtual ~ArchvTaskInfoBrief(){}

public:
	//��Ա����
	UInt32	taskID;
	string	taskName;
	Byte		taskType;
	Byte		taskStatus;
	Byte		adviceLevel;

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(taskID)
		SERIAL_ENTRY(taskName)
		SERIAL_ENTRY(taskType)
		SERIAL_ENTRY(taskStatus)
		SERIAL_ENTRY(adviceLevel)

	END_SERIAL_MAP()

};



//������ϸ��
class ArchvTaskDetail
	:public BaseArchive
{

public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvTaskDetail():goalType(0),goalID(0),isFinish(0)
		,goalNum(0),finishNum(0)
	{}
	virtual ~ArchvTaskDetail(){}

public:
	//��Ա����
	Byte		goalType;
	UInt32	goalID;
	Byte		isFinish;
	UInt32	goalNum;
	UInt32	finishNum;

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(goalType)
		SERIAL_ENTRY(goalID)
		SERIAL_ENTRY(isFinish)
		SERIAL_ENTRY(goalNum)
		SERIAL_ENTRY(finishNum)
	END_SERIAL_MAP()

};


//������Ϣ
class ArchvTaskInfo
	:public BaseArchive
{

public:
	//��Ա��������ֵ
	//	�� string���͡���Listģ�����͵ĳ�Ա���������鶼���ϳ�ֵ
	ArchvTaskInfo():taskID(0),taskType(0),taskStatus(0)
	{}
	virtual ~ArchvTaskInfo(){}

public:
	//��Ա����
	UInt32	taskID;
	string	taskName;
	Byte		taskType;
	Byte		taskStatus;
	List<ArchvTaskDetail> ltd;

//��Ա�������л�������Serializer�ཫ���ֶ�˳��������л�
	BEGIN_SERIAL_MAP()
		SERIAL_ENTRY(taskID)
		SERIAL_ENTRY(taskName)
		SERIAL_ENTRY(taskType)
		SERIAL_ENTRY(taskStatus)
		SERIAL_ENTRY(ltd)
	END_SERIAL_MAP()

};

class ArchvUnfinishedTask
	:public BaseArchive
{
 public:
 	ArchvUnfinishedTask():roleID(0)
 		{}
 	virtual ~ArchvUnfinishedTask(){}

 public:

	UInt32 roleID;
	list<UInt32>licTaskID;

	BEGIN_SERIAL_MAP()
	  SERIAL_ENTRY(roleID)
	END_SERIAL_MAP()
};

#endif
