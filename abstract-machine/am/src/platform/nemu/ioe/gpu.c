#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  // uint32_t w = 400; // Done: get the correct width
  // uint32_t h = 300; // Done: get the correct height
  // outl(VGACTL_ADDR, ((w << 16) | h));
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // for (int i = 0; i < w * h; i++)
  //   fb[i] = i;
  // outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = inw(VGACTL_ADDR+2), .height = inw(VGACTL_ADDR),
    .vmemsz = width * height * sizeof(uint32_t)
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  /* 向屏幕(x, y)坐标处绘制w*h的矩形图像. 
     图像像素按行优先方式存储在pixels中, 
    每个像素用32位整数以00RRGGBB的方式描述颜色. 
  AM_GPU_FBDRAW_T
    x: point x 
    y: point y
    pixel: color_buf
    w : picture w
    h : picture h
    sync

  <----x---->|
             |
             y
             |
             |
             |<---w---->|
             |          |
             h   pic    |
             |          |
             |----------|
  */ 
  uint32_t *fb = (uint32_t *)FB_ADDR;
  uint32_t *pi = (uint32_t *)ctl->pixels;
  int x = ctl->x, y = ctl->y, w = ctl->w,h = ctl->h;
  int width = io_read(AM_GPU_CONFIG).width;
  

  #define fb_index(x,y,w,h,width) (y)*(width)+(x)+(h)*(width)+(w)
  for (int i = 0; i < h; ++i){
    for (int j = 0; j < w; ++j){
      fb[fb_index(x,y,j,i,width)] = pi[i * w + j];
    }
  }

  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
