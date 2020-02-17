# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources(arm_core_mmu.c)

zephyr_library_include_directories(
  .
  ../../../../include/cortex_a
)
