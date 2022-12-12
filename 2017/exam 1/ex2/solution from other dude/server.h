#ifndef _SERVER_H_
#define _SERVER_H_

#include <stdint.h>

/* kinds of devices */
enum devicekind {
  D_LIGHT = 0,
  D_POWER,
  D_SUNBLIND,
  D_LOCK,
  D_ALARM,
  NUM_DEVICEKIND
};

/* device list element */
struct device {
  struct device *next;
  uint8_t id;
  enum devicekind kind;
  uint8_t *statep;
};

typedef struct device device_t;


#endif /* _SERVER_H_ */
