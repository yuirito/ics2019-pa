#include "cpu/exec.h"
#include "cc.h"

make_EHelper(test) {
  /* PA2.2 */
  rtl_and(&s0,&id_dest->val,&id_src->val);
  
  rtl_update_ZFSF(&s0,id_dest->width);
 
  // CF = 0, OF = 0
  s1 = 0;
  rtl_set_CF(&s1);
  rtl_set_OF(&s1);


  print_asm_template2(test);
}

make_EHelper(and) {
  /* PA2.1 */
  rtl_and(&s0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &s0);
  rtl_update_ZFSF(&s0, id_dest -> width);
  
  // CF = 0, OF = 0
  s1 = 0;
  rtl_set_CF(&s1);
  rtl_set_OF(&s1);

  print_asm_template2(and);
}

make_EHelper(xor) {
  /* PA2.1 */
  rtl_xor(&s0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &s0);
  rtl_update_ZFSF(&s0, id_dest -> width);
  
  // CF = 0, OF = 0
  s1 = 0;
  rtl_set_CF(&s1);
  rtl_set_OF(&s1);
  
  print_asm_template2(xor);
}

make_EHelper(or) {
  /* PA2.1 */
  rtl_or(&s0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &s0);
  rtl_update_ZFSF(&s0, id_dest -> width);
  
  // CF = 0, OF = 0
  s1 = 0;
  rtl_set_CF(&s1);
  rtl_set_OF(&s1);

  print_asm_template2(or);
}

make_EHelper(sar) {
  
  // unnecessary to update CF and OF in NEMU
  /* PA2.2 */
  rtl_sar(&s0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &s0);
  rtl_update_ZFSF(&s0, id_dest->width);

  print_asm_template2(sar);
}

make_EHelper(shl) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  /* PA2.2 */
  rtl_shl(&s0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &s0);
  rtl_update_ZFSF(&s0, id_dest->width);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  TODO();
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint32_t cc = decinfo.opcode & 0xf;

  rtl_setcc(&s0, cc);
  operand_write(id_dest, &s0);

  print_asm("set%s %s", get_cc_name(cc), id_dest->str);
}

make_EHelper(not) {
  /* PA2.2 */
  rtl_not(&s0,&id_dest->val);
  operand_write(id_dest,&s0);

  print_asm_template1(not);
}
