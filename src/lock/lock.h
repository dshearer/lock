#ifndef __LOCK_H__
#define __LOCK_H__

#include "status.h"

namespace lock {

void setup();

/*
Engage or disengage lock.

@return 0 on success; non-0 on error.
*/
int setLockStatus(status_t status);

};

#endif /* __LOCK_H__ */