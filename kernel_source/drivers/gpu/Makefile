ccflags-y += -Werror
obj-y			+= drm/ vga/
obj-$(CONFIG_TEGRA_HOST1X)	+= host1x/
#obj-y  += img/mt6595/m1.3_ED2673322/
#obj-y  += img/
#obj-y  += mali/
ifeq ($(CONFIG_MTK_GPU_SUPPORT),y)
obj-$(CONFIG_MALI_MT8127) += mali/
obj-$(CONFIG_ARCH_MT8590) += mali/
endif
