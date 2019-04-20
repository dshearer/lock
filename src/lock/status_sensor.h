#ifndef __STATUS_SENSOR_H__
#define __STATUS_SENSOR_H__

#include "status.h"

namespace status_sensor {

typedef void (*callback_t)(status_t oldStatus, status_t newStatus);

void setup();
void addCallback(callback_t cb);
status_t getStatus();

} // namespace

#endif /* __STATUS_SENSOR_H__ */