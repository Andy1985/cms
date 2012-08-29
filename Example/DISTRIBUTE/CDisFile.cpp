#include "CDisFile.h"

#include <sstream>
using std::stringstream;

CDisFile::CDisFile(const int type,const int version,const int profile_id)
{
    this->type = type;
    this->version = version;
    this->profile_id = profile_id;
}

CDisFile::CDisFile()
{
    this->type = -1;
    this->version = -1;
    this->profile_id = -1;
}

CDisFile::~CDisFile()
{
}

int CDisFile::getType() const
{
    return this->type;
}

int CDisFile::getVersion() const
{
    return this->version;
}

int CDisFile::getProfileID() const
{
    return this->profile_id;
}

string CDisFile::getFileName() const
{
    stringstream ss;
    ss << this->profile_id;
    ss << "/";
    ss << this->type;
    ss << "/";
    ss << this->version;

    return ss.str();
}

void CDisFile::setType(const int type)
{
    this->type = type;
}

void CDisFile::setVersion(const int version)
{
    this->version = version;
}

void CDisFile::setProfileID(const int profile_id)
{
    this->profile_id = profile_id;
}
