#ifndef PRIVILEGE_H
#define PRIVILEGE_H

int get_root_privilege();
int push_uid();
int push_gid();
int pop_uid();
int pop_gid();

#endif   // PRIVILEGE_H
