#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <sys/stat.h>

#include "CBuildBlackList.h"

#include <sstream>
using std::stringstream;

CBuildBlackList::CBuildBlackList(const char * file)
{
    this->fp = NULL;
    strcpy(this->File,file);
}

CBuildBlackList::~CBuildBlackList()
{
    if (NULL != fp) fclose(fp);
}

bool CBuildBlackList::OpenFile()
{
    if ((fp = fopen(this->File,"w")) == NULL)
    {
        fprintf(stderr,"Cannot Open File %s,exit!\n",this->File);
        return false;
    }

    return true;
}

bool CBuildBlackList::WriteContent(const char *content)
{
    if (fwrite(content,strlen(content),sizeof(char),fp) <= 0)
    {
        fprintf(stderr,"Write Into File %s Failure,exit!\n",this->File);
        return false;
    }

    return true;
}

bool CBuildBlackList::BuildBlackList(const vector<string>& bllist)
{
    if (NULL == fp) 
    {
        if (false == OpenFile()) return false;
    }
    
    for(vector<string>::const_iterator i = bllist.begin(); i != bllist.end(); i++)
    {
        stringstream ss;
		ss << *i;
        ss << '\n';
                
        if (false == WriteContent(ss.str().data())) 
        {
            fclose(fp);
            fp = NULL;
            unlink(File);
            return false;
        }
    }

    return true;
}
