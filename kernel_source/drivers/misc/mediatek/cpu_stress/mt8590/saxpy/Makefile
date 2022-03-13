ifeq ($(CONFIG_ARM64), y)
obj-y   := slt_saxpy.o slt_fp5_saxpy_asm_64.o
KBUILD_AFLAGS   :=$(KBUILD_AFLAGS:-msoft-float=-Wa,-mfpu=neon-vfpv4)
KBUILD_CFLAGS   := $(filter-out -mgeneral-regs-only,$(KBUILD_CFLAGS))
else
obj-y   := slt_saxpy.o slt_fp5_saxpy_asm.o
KBUILD_AFLAGS   :=$(KBUILD_AFLAGS:-msoft-float=-Wa,-mfpu=neon-vfpv4)
endif
