//������
#ifndef MONSTER_H
#define MONSTER_H

#include "OurDef.h"
#include "Creature.h"
#include  "ArchvBagItemCell.h"
#include "ArchvBagItemCell.h"
#include "Map.h"


class Monster
	:public Creature
{

public:
	Monster();
	~Monster();
public:
	UInt32 Exp;
	UInt32 Money;

	map<UInt32,ItemList> ItemDropTask;
	list<ItemList> ItemDropSpec;//ר����Ʒ�ĵ���
	char ItemDropPublic;//��������

private:
	
};


#endif

