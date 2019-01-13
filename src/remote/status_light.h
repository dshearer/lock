#ifndef __STATUS_LIGHT_H__
#define __STATUS_LIGHT_H__

namespace status_light {

typedef enum {
    STATUS_SUCCESS,
    STATUS_UNAUTHN,
    STATUS_LOCK_ERROR,
    STATUS_RADIO_ERROR,
} status_t;

void setup();
void show_status(status_t status);

};

#endif /* __STATUS_LIGHT_H__ */