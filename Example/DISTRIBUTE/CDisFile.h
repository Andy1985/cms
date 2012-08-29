#ifndef CDISFILE_H
#define CDISFILE_H

#include <string>
using std::string;

class CDisFile
{
    public:
        CDisFile(const int type,const int version,const int profile_id);
        CDisFile();
        ~CDisFile();
        
        void setType(const int type);
        void setVersion(const int version);
        void setProfileID(const int profile_id);

        int getType() const;
        int getVersion() const;
        int getProfileID() const;
        string getFileName() const;
    private:
        int type;
        int version;
        int profile_id;

};
#endif /*CDISFILE_H*/
