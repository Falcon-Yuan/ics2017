#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  memcpy(&t1,&cpu.eflags,sizeof(cpu.eflags));
  rtl_li(&t0,t1);
  rtl_push(&t0);
  cpu.eflags.IF = 0;
  rtl_push(&cpu.cs);
  rtl_li(&t0,ret_addr);
  rtl_push(&t0);

  vaddr_t gate_addr=cpu.idtr.base + NO * sizeof(GateDesc);
  assert(gate_addr <= cpu.idtr.base + cpu.idtr.limit);

	uint32_t eip_low, eip_high, offset;
	eip_low = vaddr_read(gate_addr,2);
	eip_high = vaddr_read(gate_addr + sizeof(GateDesc) - 2,2);
	offset = (eip_high << 16) + eip_low;
	decoding.jmp_eip = offset;
	decoding.is_jmp = true;
}

void dev_raise_intr() {
	cpu.INTR = true;
}
