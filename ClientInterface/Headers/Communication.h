#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include"myWidget.h"

namespace Core{
class myWidget;

class Communication
{
public:
    Communication(myWidget *wid);
    void *recvmg();
    int conection(char *user_name);
    int send_msg(char *msg, int sock);
    int send_f(char *arguments, int sock, char* user_name);
    int download_f(char *arguments, int sock, char* user_name);
    int my_socket;
    pthread_t recvt;
private:
    myWidget *widget;
};
} //end namespace
#endif // COMMUNICATION_H
