#ifndef MSGQ_H
#define MSGQ_H

#include "ipc.h"

int create_msgq(key_t key);
int send_msg(int msgid, struct message *msg);
int recv_msg(int msgid, struct message *msg, long msg_type);
int remove_msgq(int msgid);

#endif