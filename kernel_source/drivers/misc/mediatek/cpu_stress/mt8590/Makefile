#
# for CA7 cpu stress test
#
ifeq ($(CONFIG_ARM64), y)
obj-y	:= slt_dhry.o slt_dhry_1.o slt_dhry_2.o slt_maxpower.o slt_max_power.o slt_cache_miss.o cache_miss_asm.o
obj-y   += saxpy/
KBUILD_AFLAGS	:=$(KBUILD_AFLAGS:-msoft-float=-Wa,-mfpu=neon-vfpv4) 
#KBUILD_CFLAGS   := $(filter-out -mgeneral-regs-only,$(KBUILD_CFLAGS))
else
obj-y	:= slt_maxpower.o slt_maxtrans.o slt_memcpyL2.o slt_fp3_memcpyL2_asm.o slt_vfp_func.o slt_fp4_vfp_func_asm.o slt_fdct.o slt_fp7_fdct_asm.o slt_imdct.o slt_fp8_imdct_asm.o slt_dhry.o slt_dhry_1.o slt_dhry_2.o slt_fp0_maxpower_asm_CA7L.o slt_fp0_maxtrans_asm_CA7L.o slt_fp13_adv_simd_asm.o slt_adv_simd.o
obj-y   += saxpy/
KBUILD_AFLAGS	:=$(KBUILD_AFLAGS:-msoft-float=-Wa,-mfpu=neon-vfpv4) 
endif
