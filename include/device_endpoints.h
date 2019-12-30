#ifndef DEVICE_ENDPOINTS_H_
#define DEVICE_ENDPOINTS_H_

// Device setup endpoints
struct {
  struct {
    struct  {
      const char* DATA = "/api/device/data";
    } DEVICE;
  } API;
} DEVICE_URLS;

#endif
