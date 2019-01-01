#ifndef __LOCK_H__
#define __LOCK_H__

namespace lock {

typedef enum {
    LOCK_ENGAGED,
    LOCK_DISENGAGED,
} lock_state_t;

void setup();

void set_lock_state(lock_state_t new_state);

};

#endif /* __LOCK_H__ */