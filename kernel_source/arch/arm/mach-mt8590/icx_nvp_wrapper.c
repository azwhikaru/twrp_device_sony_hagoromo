/*
 * icx_nvp_wrapper.c -- ICX NVP driver wrapper
 *
 * Copyright 2016 Sony Corporation.
 *
 */

#ifdef CONFIG_ICX_NVP_WRAPPER

#include <linux/types.h>
#include <linux/kernel.h>

struct icx_nvp_func_table {
    int (*icx_nvp_write)(int zn, unsigned char *buf, int size);
    int (*icx_nvp_read)(int zn, unsigned char *buf, int size);
};

static struct icx_nvp_func_table icx_nvp_func = {
	.icx_nvp_write = 0,
	.icx_nvp_read = 0,
};

int register_icx_nvp_func(struct icx_nvp_func_table *regi_func)
{
    if (!regi_func) {
        return -1;
    }
	icx_nvp_func.icx_nvp_write = regi_func->icx_nvp_write;
	icx_nvp_func.icx_nvp_read  = regi_func->icx_nvp_read;
	return 0;
}
EXPORT_SYMBOL(register_icx_nvp_func);

int icx_nvp_write_data_wrapper(int zn, unsigned char *buf, int size)
{
	if (icx_nvp_func.icx_nvp_write) {
		return icx_nvp_func.icx_nvp_write(zn, buf, size);
	}
	return -1;
}

int icx_nvp_read_data_wrapper(int zn, unsigned char *buf, int size)
{
	if (icx_nvp_func.icx_nvp_read) {
		return icx_nvp_func.icx_nvp_read(zn, buf, size);
	}
	return -1;
}
#endif /* CONFIG_ICX_NVP_WRAPPER */

