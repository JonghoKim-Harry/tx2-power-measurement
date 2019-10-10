#ifndef PRIVILEGE_H
#define PRIVILEGE_H

/**
 *   You can not gain root privilege.
 *   Start your program as root then drop the root privilge.
 *
 *   See:   https://wiki.sei.cmu.edu/confluence/display/c/POS37-C.+Ensure+that+privilege+relinquishment+is+successful
 *          https://stackoverflow.com/questions/2483755/how-to-programmatically-gain-root-privileges
 */


// Return 0 on success, -1 on failure
int restore_root_privilege();
int drop_root_privilege_temp();
int drop_root_privilege_perm();

#endif   // PRIVILEGE_H
