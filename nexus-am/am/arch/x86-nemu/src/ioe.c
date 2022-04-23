#include <am.h>
#include <x86.h>

#define RTC_PORT 0x48   // Note that this is not standard
static unsigned long boot_time;

void _ioe_init() {
  boot_time = inl(RTC_PORT);
}

unsigned long _uptime() {
  // return 0;
  return inl(RTC_PORT)-boot_time;//RTC寄存器中的当前时间 - boot_time
}

uint32_t* const fb = (uint32_t *)0x40000;

_Screen _screen = {
  .width  = 400,
  .height = 300,
};
 
extern void* memcpy(void *, const void *, int);

void _draw_rect(const uint32_t *pixels, int x, int y, int w, int h) {
  // int i;
  // for (i = 0; i < _screen.width * _screen.height; i++) {
  //   fb[i] = i;
  // }
  int cp_bytes=sizeof(uint32_t) * (w<_screen.width-x?w:_screen.width-x);//一行占的空间
  for(int j=0;j<h && y+j<_screen.height;j++) {//按行拷贝
    memcpy(&fb[(y+j)*_screen.width+x],pixels,cp_bytes);
    pixels+=w;
  }
}

void _draw_sync() {
}

int _read_key() {
  // return _KEY_NONE;
  if(inb(0x64)&0x1)//判断是否有键按下
	  return inl(0x60);//获取键盘码
  return _KEY_NONE;
}
