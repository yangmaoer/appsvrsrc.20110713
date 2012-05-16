#ifndef PACKET_H
#define PACKET_H

#include "OurDef.h"
#include "DataBuffer.h"
#include <string>

using namespace std;

class Packet
{
public:
	Packet();

	Packet(DataBuffer* buffer);

	Packet(unsigned int bufferSize);
	
	~Packet();

	
	void MakeHeader(UInt16 mid,Byte dir,Byte type, Byte seq, UInt32 RoleID =0,UInt32 tsn=0);

	
	bool PackHeader();

	
	bool UpdatePacketLength();

	
	bool UnpackHeader();

	//���ݻ�����
	inline DataBuffer* GetBuffer()
	{
		return _buffer;
	}

	inline void SetBuffer(DataBuffer* buffer)
	{
		if(_buffer!=NULL&&_ownedBuffer) delete _buffer; 
		
		_buffer = buffer;
		_ownedBuffer = false;
	}

	
	void CopyHeader(const Packet& packet);

public:
	//������(������ͷ)
	UInt16 Length;

	//���ݷ��ͷ���
	Byte   Direction;

	//����������
	Byte   SvrType;

	//���������
	Byte   SvrSeq;

	//��Ϣ���� 
	UInt16 MsgType;

	//��ϢΨһ ID
	UInt32 UniqID;

	//��ɫID
	UInt32 RoleID;

private:
	bool _ownedBuffer;
	DataBuffer *_buffer;
};

#endif
