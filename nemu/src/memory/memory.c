#include "nemu.h"
#include "device/mmio.h"

#define PMEM_SIZE (128 * 1024 * 1024)

#define pmem_rw(addr, type) *(type *)({\
    Assert(addr < PMEM_SIZE, "physical address(0x%08x) is out of bound", addr); \
    guest_to_host(addr); \
    })

uint8_t pmem[PMEM_SIZE];

/* Memory accessing interfaces */

uint32_t paddr_read(paddr_t addr, int len) {
  	// 通过is_mmio () 函数判断一个物理地址是否被映射到I/O 空间
	int id = is_mmio(addr);
	
	if (id == -1) // 没有映射到I/O 空间
		return pmem_rw(addr, uint32_t) & (~0u >> ((4 - len) << 3));
	// 映射到I/O 空间
	return mmio_read(addr, len, id);
}

void paddr_write(paddr_t addr, int len, uint32_t data) {
  int id = is_mmio(addr);
	
	if (id == -1) // 没有映射到I/O 空间
		memcpy(guest_to_host(addr), &data, len);
	else // 映射到I/O 空间
		mmio_write(addr, len, data, id);

}

uint32_t vaddr_read(vaddr_t addr, int len) {
  return paddr_read(addr, len);
}

void vaddr_write(vaddr_t addr, int len, uint32_t data) {
  paddr_write(addr, len, data);
}
