#include "CProfile.h"

CProfile::CProfile(const int id,const string name)
{
    this->id = id;
    this->name = "";
}

CProfile::~CProfile()
{

}

int CProfile::getID() const
{
    return this->id;
}

string CProfile::getName() const
{
    return this->name;
}

void CProfile::setID(const int id)
{
    this->id = id;
}

void CProfile::setName(const string name)
{
    this->name = name;
}
