#include "CMachine.h"

CMachine::CMachine(string name,int id)
{
    this->name = name;
    this->id = id;
}

CMachine::CMachine()
{
    this->name = "";
    this->id = -1;
}

CMachine::~CMachine()
{
}

string CMachine::getName() const
{
    return this->name;
}

int CMachine::getID() const
{
    return this->id;
}

void CMachine::setName(const string name)
{
    this->name = name;
}

void CMachine::setID(const int id)
{
    this->id = id;
}
