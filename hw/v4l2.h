#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <poll.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/videodev2.h>
#include <linux/v4l2-subdev.h>

#include "opts/log.h"

#ifndef CFG_XIOCTL_RETRIES
#	define CFG_XIOCTL_RETRIES 4
#endif
#define XIOCTL_RETRIES ((unsigned)(CFG_XIOCTL_RETRIES))

typedef struct {
	char buf[10];
} fourcc_string;


fourcc_string fourcc_to_string(unsigned format);
unsigned fourcc_to_stride(unsigned width, unsigned format);
int xioctl(const char *name, int fd, int request, void *arg);

#define E_XIOCTL(dev, _fd, _request, _value, _msg, ...) do { \
		int ret; \
		if ((ret = xioctl(dev_name(dev), _fd, _request, _value)) < 0) { \
			E_LOG_ERROR(dev, "xioctl(ret=%d): " _msg, ret, ##__VA_ARGS__); \
		} \
	} while(0)
