/*
* Copyright (C) 2012 Sony Mobile Communications AB.
* All rights, including trade secret rights, reserved.
*
*@ file drivers/platform/msm/qpnp-powerkey-forcecrash.c
*
*
*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/timer.h>
#include <linux/device.h>
#include <linux/spmi.h>
#include <linux/fs.h>

#include "../../../arch/arm/mach-msm/include/mach/restart.h"

#define QPNP_PON_RT_STS(base) (base + 0x10)
#define FORCE_CRASH_TIMEOUT 5
#define QPNP_PON_KPDPWR_N_SET BIT(0)

static struct timer_list forcecrash_timer;
static struct device *dev;
static struct spmi_device *gspmi;
static u16 base;

static void forcecrash_timeout(unsigned long data)
{
	u8 pon_rt_sts = 0, rt_bit = QPNP_PON_KPDPWR_N_SET;
	int rc;

	rc = spmi_ext_register_readl(gspmi->ctrl, gspmi->sid,
			QPNP_PON_RT_STS(base), &pon_rt_sts, 1);
	if (rc)
		dev_err(dev, "Unable to read PON RT status\n");

	dev_info(dev, "status %d\n", pon_rt_sts & rt_bit);
	if (pon_rt_sts & rt_bit) {
		emergency_sync();
		msm_restart('\0', "bootloader");
	} else {
		del_timer(&forcecrash_timer);
	}
}

void qpnp_powerkey_forcecrash_timer_setup(bool key_pressed)
{

	if (key_pressed)
		mod_timer(&forcecrash_timer,
				jiffies + FORCE_CRASH_TIMEOUT * HZ);
	else
		del_timer(&forcecrash_timer);
}
EXPORT_SYMBOL(qpnp_powerkey_forcecrash_timer_setup);

int qpnp_powerkey_forcecrash_init(struct spmi_device *spmi, u16 pon_base)
{
	gspmi = spmi;
	base = pon_base;
	dev = &spmi->dev;
	init_timer(&forcecrash_timer);
	forcecrash_timer.function = forcecrash_timeout;
	return 0;
}
EXPORT_SYMBOL(qpnp_powerkey_forcecrash_init);

void qpnp_powerkey_forcecrash_exit(struct spmi_device *spmi)
{
	del_timer(&forcecrash_timer);
}
EXPORT_SYMBOL(qpnp_powerkey_forcecrash_exit);
