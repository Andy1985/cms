#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <sys/stat.h>

#include "CRunLog.h"

CRunLog::CRunLog()
{
    this->fp = NULL;
    this->FileSize = MAX_FILE_SIZE;
    strcpy(this->LogFile,"./NoName.log");
}

CRunLog::CRunLog(char file[])
{
    this->fp = NULL;
    this->FileSize = MAX_FILE_SIZE;
    strcpy(this->LogFile,file);
}

CRunLog::~CRunLog()
{
    if (NULL != fp) fclose(fp);
}

void CRunLog::SetFileSize(const int FileSize)
{
    this->FileSize = FileSize;    
}

void CRunLog::SetLogFileName(char file[])
{
    strcpy(this->LogFile,file);
}

void CRunLog::AddHead(char current_time[])
{
    time_t now = time (NULL);
    struct tm *ptr = localtime (&now);
    strftime (current_time, MAX_LINE_SIZE, "[%Y-%m-%d %H:%M:%S] ", ptr);
}

long CRunLog::GetFileSize(char *FileName)
{
    struct stat buf;
    if (stat(FileName,&buf) != -1)
    {
        return buf.st_size;
    }

    return -1;
}

void CRunLog::ResetFile()
{
    if (NULL != fp) fclose(fp);            
    if ((fp = fopen(this->LogFile,"w")) != NULL) fclose(fp);
    fp = NULL;
}

void CRunLog::OpenFile()
{
    if ((fp = fopen(this->LogFile,"a")) == NULL)
    {
        fprintf(stderr,"Cannot Open LogFile %s,exit!\n",this->LogFile);
        exit(EXIT_FAILURE);
    }
}

void CRunLog::WriteContent(const char *content)
{
    if (fwrite(content,strlen(content),sizeof(char),fp) <= 0)
    {
        fprintf(stderr,"Write Into LogFile %s Failure,exit!\n",this->LogFile);
        exit(EXIT_FAILURE);
    }
}

void CRunLog::WriteRunLog(const char *shm,...)
{
    char buffer[MAX_LINE_SIZE];
    va_list sp;

    AddHead(buffer);

    va_start(sp,shm);
    vsnprintf(buffer+strlen(buffer),MAX_LINE_SIZE - strlen(buffer),shm,sp);
    va_end(sp);

    strcat(buffer,"\n");
    buffer[MAX_LINE_SIZE - 2] = '\n';

    if (GetFileSize(this->LogFile) > this->FileSize) ResetFile();
    if (NULL == fp) OpenFile();
    WriteContent(buffer);
}
