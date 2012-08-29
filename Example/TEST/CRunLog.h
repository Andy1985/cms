#ifndef CRUNLOG_H
#define CRUNLOG_H

#include <cstdio>

#define MAX_LINE_SIZE 1024
#define MAX_FILE_SIZE (1024 * 1024 * 200) //200M

class CRunLog
{
    private:
        FILE *fp;
        char LogFile[1024];
        int FileSize;
        void AddHead(char[]);
        long GetFileSize(char*);
        void ResetFile();
        void OpenFile();
        void WriteContent(const char*);

    public:
        CRunLog();
        CRunLog(char[]);
        ~CRunLog();

        void SetLogFileName(char[]); 
        void WriteRunLog(const char*,...);
        void SetFileSize(const int FileSize);

};

#endif /*CRUNLOG_H*/
