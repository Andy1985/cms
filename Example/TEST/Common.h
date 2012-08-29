#ifndef _COMMON_H_
#define _COMMON_H_

#define RCPT_TOK '\n'
#define TO_MAIL_ADDR_SIZE 128

#include <vector>
#include <string>
using std::vector;
using std::string;

int my_itoa (char *to, unsigned int from);
unsigned int my_atoi (const char *to);
int same_domain_check (char *from, const char *to);
void recvs2vector (char *snd, char *recvs, vector < string > &vec);
string GetCurrentTime ();
char *replace_str (char *strData);
bool recv_check (char *recv);
bool my_copy(const char* from,const char* to);
int my_copy2(const char *source, const char *target);

#endif /*_COMMON_H_*/
