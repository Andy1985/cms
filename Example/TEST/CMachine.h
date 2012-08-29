#ifndef CMACHINE_H
#define CMACHINE_H

#include <string>
using std::string;

class CMachine
{
    public:
        CMachine(string name,int id);
        CMachine();
        ~CMachine();

        string getName() const;
        int getID() const;

        void setName(const string name);
        void setID(const int id); 
    
    private:
        string name;
        int id;
};
#endif /*CMACHINE_H*/
