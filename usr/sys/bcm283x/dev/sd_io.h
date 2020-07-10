#ifndef __V7_DEV_SD_IO_H
#define __V7_DEV_SD_IO_H

#include "sd_cmd.h"

#include "../../h/types.h"

struct sd_io_interface_t {
  uptr_t (*init)(uptr_t iobase, u32 extclk);
  void (*fini)(uptr_t handle);
  int (*cmd)(uptr_t handle, u32 cmd, u32 arg, u32 *resp);
  int (*iosetup)(uptr_t handle, u16 s, u16 c);
  int (*iostart)(uptr_t handle, int w, int a, u32 ba, u8 *buf, u32 len);
  int (*iowait)(uptr_t handle, int w, u32 max_us);
  int (*iofinish)(uptr_t handle, int w, u8 *buf, u32 len);
  u32 ocr_supported;
  u32 hs_capable;
  u32 hc_capable;
  u32 switch_1v8_capable;
  u32 four_bit_bus_capable;
};

extern const struct sd_io_interface_t sd_io_interface;

struct sd_io_t {
  uptr_t (*init)(uptr_t iobase, u32 extclk);
  void (*fini)(uptr_t handle);
  int (*iostart)(uptr_t handle, int w, int a, u32 ba, u8 *buf, u32 len);
  int (*iowait)(uptr_t handle, int w, u32 max_us);
  int (*iofinish)(uptr_t handle, int w, u8 *buf, u32 len);
};

extern const struct sd_io_t sd_io;

#endif
