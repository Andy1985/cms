#include <sys/time.h>
#include <string.h>
#include "Common.h"

#include   <sys/types.h> 
#include   <sys/stat.h> 
#include   <fcntl.h> 
#include   <unistd.h>
#include   <errno.h>

int my_itoa (char *to, unsigned int from)
{
    return sprintf (to, "%u", from);
}

unsigned int my_atoi (const char *to)
{
    return (unsigned int) strtoul (to, NULL, 10);
}

int same_domain_check (char *from, char const *to)
{
    if (NULL == from || NULL == to)
    {
        return -1;
    }

    char *from_domain = strstr (from, "@");
    char *to_domain = strstr (to, "@");

    if (NULL == from_domain)
    {
        return -1;
    }

    if (NULL == to_domain)
    {
        return -1;
    }

    if (0 == strcmp (from_domain, to_domain))
    {
        return 0;
    }

    return 1;
}

char *replace_str (char *strData)
{
    static char szRet[2048] = { 0 };
    const char *p = strData;
    char *pFind = NULL;

    memset (szRet, 0, 2048);
    while ((pFind = strchr (p, '\'')))
    {
        strncpy (szRet + strlen (szRet), p, pFind - p);
        strcat (szRet, "\'\'");
        p = pFind + 1;
    }

    if (p)
    {
        strcat (szRet, p);
    }

    return szRet;
}

bool recv_check (char *recv)
{
    char *p = strstr (recv, "@");

    if (NULL == p)
    {
        return false;
    }

    p = strstr (p, ".");

    if (NULL == p)
    {
        return false;
    }

    return true;

}

void recvs2vector (char *snd, char *recvs, vector < string > &vec)
{
    vec.clear ();

    char buffer[TO_MAIL_ADDR_SIZE] = { 0 };
    int j = 0;
    char *p = recvs;

    for (unsigned int i = 0; i <= strlen (recvs); i++)
    {
        if ((char) p[i] == RCPT_TOK || i == strlen (recvs))
        {
            buffer[j] = '\0';

            if (buffer != NULL)
            {
                if (true == recv_check (buffer))
                {
                    if (0 != same_domain_check (snd, buffer))
                    {
                        if (buffer[0] != '\0')
                            vec.push_back (string (buffer));
                    }
                }
            }

            memset (&buffer, 0, TO_MAIL_ADDR_SIZE);
            j = 0;
        }
        else if (i < strlen (recvs))
        {
            buffer[j] = p[i];
            j++;
        }

    }
}

string GetCurrentTime ()
{
    char current_time[32] = { 0 };
    time_t now = time (NULL);
    struct tm *ptr = localtime (&now);

    strftime (current_time, 32, "%Y-%m-%d %H:%M:%S", ptr);

    return string (current_time);
}

bool my_copy(const char* from, const char* to)
{
    int f1;
    int f2;

    if (!(f1 = open(from,O_RDONLY)))
    {
        return false;
    }

    if (!(f2 = open(to,O_WRONLY|O_CREAT,0644)))
    {
        return false;
    }

    char buf[1024];
    memset(buf,0,1024);

    int size;
    size = read(f1,buf,1024);
    while (size != 0)
    {
        write(f2,buf,size);
        size = read(f1,buf,1024);
    }

    close(f1);
    close(f2);

    return true;
}

int my_copy2(const char *source, const char *target)
{
        struct stat src_stat;

        if (lstat(source, &src_stat) == -1)
                 return -1;

         if (S_ISLNK(src_stat.st_mode)) {
                 char lnk[1024];
                 int lnk_size;
                 if ((lnk_size = readlink(source, lnk, 1023)) == -1)
                         return -2;
                 lnk[lnk_size] = '\0';
                 if (symlink(lnk, target) == -1)
                         return -3;
         } else if (S_ISREG(src_stat.st_mode)) {
                 int src, dst;
                 int rsize;
                 char buf[1024];
                 if ((src = open(source, O_RDONLY)) == -1) {
                         close(dst);
                         return -4;
                 }
                 if ((dst = creat(target, src_stat.st_mode)) == -1)
                         return -5;
                 while ((rsize = read(src, buf, 1024))) {
                         if (rsize == -1 && errno == EINTR)
                                 continue ;
                         if (rsize == -1) {
                                 close(src);
                                 close(dst);
                                 return -6;
                         }
                         while (write(dst, buf, rsize) == -1)
                                 if (errno != EINTR) {
                                 close(src);
                                 close(dst);
                                 return -7;
                         }
                 }
                 close(src);
                 close(dst);
         } else {
                 return -8;
         }
         return 0;
 }

string& replace_all(string& str,const string& old_value,const string& new_value)      
{      
    while (true)
    {      
        string::size_type pos(0);      

        if ((pos = str.find(old_value)) != string::npos)      
            str.replace(pos,old_value.length(),new_value);      
        else
           break;      
    }      

    return str;      
}
