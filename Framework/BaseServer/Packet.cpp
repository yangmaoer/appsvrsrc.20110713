#include "Packet.h"

#include "DebugData.h"

Packet::Packet()
	:_ownedBuffer(false)
	,_buffer(NULL)
{
}

Packet::Packet(unsigned int bufferSize)
{
	_buffer = new DataBuffer(bufferSize);
	_ownedBuffer = true;
}

Packet::Packet(DataBuffer* buffer)
	:_buffer(buffer)
	,_ownedBuffer(false)
{
}

Packet::~Packet()
{
	if(_buffer!=NULL&&_ownedBuffer) delete _buffer;
}

//�����ͷ����ֶεĸ�ֵ
void Packet::MakeHeader(UInt16 mid,Byte dir,Byte type, Byte seq, UInt32 roleID,UInt32 tsn)
{
	Direction = dir;
	SvrType = type;
	SvrSeq = seq;
	MsgType = mid;
	UniqID = tsn;
	
	if(roleID)
	{
		RoleID = roleID;
	}
}

//��ͷ���, ����� databuffer
bool Packet::PackHeader()
{
	//���ݻ������
	_buffer->Reset();

	_buffer->Write(&Length,sizeof(Length));
	_buffer->Write(&Direction,sizeof(Direction));
	_buffer->Write(&SvrType,sizeof(SvrType));
	_buffer->Write(&SvrSeq,sizeof(SvrSeq));
	
	_buffer->Write(&MsgType,sizeof(MsgType));
	_buffer->Write(&UniqID,sizeof(UniqID));
	_buffer->Write(&RoleID, sizeof(RoleID));

	return true;
}

//��ͷ '������'�ֶθ���
bool Packet::UpdatePacketLength()
{
	Length = _buffer->GetWritePtr()- _buffer->GetDataPtr();

	//дָ���Ƶ���ǰ
	if(_buffer->MoveWritePtr(-Length)) return false;

	//��д��ͷ '������'�ֶ�
	if(_buffer->Write(&Length,sizeof(Length))) return false;

	//дָ�����
	_buffer->MoveWritePtr(Length-sizeof(Length));

	return true;
}

//��ͷ��� ,�� databuffer���
bool Packet::UnpackHeader()
{
	_buffer->Read(&Length,sizeof(Length));
	_buffer->Read(&Direction,sizeof(Direction));
	_buffer->Read(&SvrType,sizeof(SvrType));
	_buffer->Read(&SvrSeq,sizeof(SvrSeq));

	
	_buffer->Read(&MsgType,sizeof(MsgType));
	_buffer->Read(&UniqID,sizeof(UniqID));
	_buffer->Read(&RoleID,sizeof(RoleID));
	
	
	return true;
}

//���ư�ͷ����
void Packet::CopyHeader(const Packet &packet)
{
	Length = packet.Length;
	Direction = packet.Direction;
	SvrType = packet.SvrType;
	SvrSeq = packet.SvrSeq;
	MsgType = packet.MsgType;
	UniqID = packet.UniqID;
	RoleID = packet.RoleID;
}

