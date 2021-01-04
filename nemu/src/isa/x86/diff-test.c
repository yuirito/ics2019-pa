#include "nemu.h"
#include "monitor/diff-test.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  /* PA2.2 */
  if (ref_r->eax != reg_l(R_EAX) || ref_r->ebx != reg_l(R_EBX) || ref_r->ecx != reg_l(R_ECX) || 
		ref_r->edx != reg_l(R_EDX) || ref_r->ebp != reg_l(R_EBP) || ref_r->esi != reg_l(R_ESI) ||
		ref_r->edi != reg_l(R_EDI) || ref_r->pc != cpu.pc ) 
	{
		return false;
	}
	return true;
}

void isa_difftest_attach(void) {
	/* PA3.3 */
	ref_difftest_memcpy_from_dut(0, &pmem[0], 0x7c00);
	ref_difftest_memcpy_from_dut(0x100000, &pmem[0x100000], PMEM_SIZE - 0x100000);
	ref_difftest_setregs(&cpu);

	ref_difftest_memcpy_from_dut(0x7e00, (void *)&cpu.idtr.limit, sizeof(cpu.idtr.limit));
    ref_difftest_memcpy_from_dut(0x7e00 + sizeof(cpu.idtr.limit), (void *)&cpu.idtr.base, sizeof(cpu.idtr.base));

    uint8_t lidt_code[] = { 0x0f, 0x01, 0x1d, 0x00, 0x7e, 0x00, 0x00 };
    ref_difftest_memcpy_from_dut(0x7e40, (void *)&lidt_code, sizeof(lidt_code));
    CPU_state cpu_t = cpu;
    cpu_t.pc = 0x7e40;
	ref_difftest_setregs(&cpu_t);
    ref_difftest_exec(1);
	ref_difftest_setregs(&cpu);
}
