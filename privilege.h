#ifndef PRIVILEGE_H
#define PRIVILEGE_H

// Return 0 on success, -1 on failure
int push_privilege();
int get_root_privilege();
int drop_root_privilege();
int pop_privilege();

#endif   // PRIVILEGE_H
