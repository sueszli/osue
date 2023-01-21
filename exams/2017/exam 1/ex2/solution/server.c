
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "server.h"
#include "common.h"

extern const char *program_name;

// declarations for demo solution
bool task_3_demo(device_t *, uint8_t, uint8_t);


/*
// server.h:

enum devicekind {
  D_LIGHT = 0,
  D_POWER,
  D_SUNBLIND,
  D_LOCK,
  D_ALARM,
  NUM_DEVICEKIND
};

// device list element
struct device {
  struct device *next;
  uint8_t id;
  enum devicekind kind;
  uint8_t *statep;
};
typedef struct device device_t;

*/

bool update_device_status(device_t * list, uint8_t id, uint8_t value)
{
    /*******************************************************************
     * Task 3
     * ------
     * Update device status.
     *
     * - Go through device list until device with given id is found.
     * - Update the status of the device according to the given value.
     * - Return true if the update was sucessful, otherwise false
     *   (device with id not existing, value out of range).
     *******************************************************************/

    printf("ID: %d\n", id);
    printf("VALUE: %d\n", id);

    /* REPLACE FOLLOWING LINE WITH YOUR SOLUTION */
    return task_3_demo(list, id, value);
}
