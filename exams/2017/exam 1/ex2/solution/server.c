
#include "server.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

extern const char *program_name;

// declarations for demo solution
bool task_3_demo(device_t *, uint8_t, uint8_t);

bool update_device_status(device_t *list, uint8_t id, uint8_t value) {
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
  printf("value: %d\n", value);

  while (list != NULL) {
    if (list->id == id) {
      switch (list->kind) {
        case D_LIGHT:
          if (value < 0 || value > 100) {
            return false;
          }
          break;

        case D_POWER:
          if (value < 0 || value > 1) {
            return false;
          }
          break;

        case D_SUNBLIND:
          if (value < 0 || value > 100) {
            return false;
          }
          break;

        case D_LOCK:
          if (value < 0 || value > 1) {
            return false;
          }
          break;

        case D_ALARM:
          if (value < 0 || value > 1) {
            return false;
          }
          break;

        case NUM_DEVICEKIND:
          if (value < 0 || value > 1) {
            return false;
          }
          break;

        default:
          assert(false);
          break;
      }

      *(list->statep) = value;
      printf("update successful!\n");
      return true;
    }
    list = list->next;
  }

  return false;
}
