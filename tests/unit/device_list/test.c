#include "device/device_list.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Track owned allocations without requiring a camera or a leak sanitizer. */
static void *allocations[256];
static size_t allocation_count;
static size_t free_count;

void __real_free(void *ptr);
void __wrap_free(void *ptr)
{
  if (ptr) {
    for (size_t i = 0; i < allocation_count; i++) {
      if (allocations[i] == ptr) {
        allocations[i] = NULL;
        free_count++;
        break;
      }
    }
  }
  __real_free(ptr);
}

static void *allocate(size_t size)
{
  assert(allocation_count < sizeof(allocations) / sizeof(allocations[0]));
  void *ptr = calloc(1, size);
  assert(ptr);
  allocations[allocation_count++] = ptr;
  return ptr;
}

static int check_cleanup(unsigned ndevices, bool empty_formats)
{
  allocation_count = 0;
  free_count = 0;
  device_list_t *list = allocate(sizeof(*list));
  list->ndevices = ndevices;
  if (ndevices) {
    list->devices = allocate(ndevices * sizeof(*list->devices));
  }
  for (unsigned i = 0; i < ndevices; i++) {
    device_info_t *info = &list->devices[i];
    info->name = allocate(16);
    info->path = allocate(32);
    if (!empty_formats) {
      info->capture_formats.formats = allocate(3 * sizeof(unsigned));
      info->capture_formats.n = 3;
      info->output_formats.formats = allocate(2 * sizeof(unsigned));
      info->output_formats.n = 2;
    }
  }

  device_list_free(list);
  int failed = free_count != allocation_count;
  if (failed) {
    fprintf(stderr, "cleanup of %u devices left %zu owned allocations\n",
      ndevices, allocation_count - free_count);
    /* Clean up leftovers so a failed test itself does not leak. */
    for (size_t i = 0; i < allocation_count; i++) {
      if (allocations[i]) {
        __real_free(allocations[i]);
        allocations[i] = NULL;
      }
    }
  }
  return failed;
}

int main(void)
{
  device_list_free(NULL);
  int failures = check_cleanup(0, false);
  failures += check_cleanup(1, false);
  failures += check_cleanup(24, false);
  failures += check_cleanup(3, true);
  for (unsigned i = 0; i < 100; i++) {
    failures += check_cleanup(8, false);
  }
  printf("104 device-list cleanup cases: %d failed\n", failures);
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
