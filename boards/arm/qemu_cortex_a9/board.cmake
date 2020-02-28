#
# Copyright (c) 2020, Immo Birnbaum
# SPDX-License-Identifier: Apache-2.0
#

set(EMU_PLATFORM qemu)

set(QEMU_CPU_TYPE_${ARCH} cortex-a9)
set(QEMU_FLAGS_${ARCH}
  -cpu ${QEMU_CPU_TYPE_${ARCH}}
  -machine xilinx-zynq-a9
  -m 512M
  -display none
  -vga none
  -device loader,addr=0xf8000008,data=0xDF0D,data-len=4
  -device loader,addr=0xf8000140,data=0x00500801,data-len=4
  -device loader,addr=0xf800012c,data=0x1ed044d,data-len=4
  -device loader,addr=0xf8000108,data=0x0001e008,data-len=4
  )

board_set_debugger_ifnset(qemu)
