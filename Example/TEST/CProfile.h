#ifndef CPROFILE_H
#define CPROFILE_H

#include <string>
using std::string;

class CProfile
{
    public:
        CProfile(const int id,const string name);
        CProfile();
        ~CProfile();

        void setID(const int id);
        void setName(const string name);
        
        int getID() const;
        string getName() const;

    private:
        int id;
        string name;
        
};
#endif  /*CPROFILE_H*/
