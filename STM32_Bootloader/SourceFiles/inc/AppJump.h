#ifndef APP_JUMP_H
#define APP_JUMP_H

/* Jumps to the application, but only if App_IsValid() passes. Never returns on
 * success; returns normally if there is no valid application. */
void JumpToApplication(void);

#endif
