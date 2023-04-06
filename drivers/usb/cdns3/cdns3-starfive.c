// SPDX-License-Identifier: GPL-2.0
/**
 * cdns3-starfive.c - StarFive specific Glue layer for Cadence USB Controller
 *
 * Copyright (C) 2022 Starfive, Inc.
 * Author:	Yanhong Wang <yanhong.wang@starfivetech.com>
 * Author:	Mason Huo <mason.huo@starfivetech.com>
 * Author:	Minda Chen <minda.chen@starfivetech.com>
 */

#include <linux/bits.h>
#include <linux/clk.h>
#include <linux/module.h>
#include <linux/mfd/syscon.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/of_platform.h>
#include <linux/regmap.h>
#include <linux/reset.h>
#include <linux/usb/otg.h>
#include "core.h"

#define USB_STRAP_HOST			BIT(17)
#define USB_STRAP_DEVICE		BIT(18)
#define USB_STRAP_MASK			GENMASK(18, 16)

#define USB_SUSPENDM_HOST		BIT(19)
#define USB_SUSPENDM_MASK		BIT(19)
#define CDNS_IRQ_WAKEUP_INDEX		3

struct cdns_starfive {
	struct device *dev;
	struct phy *usb2_phy;
	struct phy *usb3_phy;
	struct regmap *stg_syscon;
	struct reset_control *resets;
	struct clk_bulk_data *clks;
	int num_clks;
	enum phy_mode phy_mode;
	u32 stg_usb_mode;
};

static int set_phy_power_on(struct cdns_starfive *data)
{
	int ret;

	ret = phy_power_on(data->usb2_phy);
	if (ret)
		return ret;

	ret = phy_power_on(data->usb3_phy);
	if (ret)
		phy_power_off(data->usb2_phy);

	return ret;
}

static void set_phy_power_off(struct cdns_starfive *data)
{
	phy_power_off(data->usb3_phy);
	phy_power_off(data->usb2_phy);
}

static void cdns_mode_init(struct platform_device *pdev,
				struct cdns_starfive *data)
{
	enum usb_dr_mode mode;

	mode = usb_get_dr_mode(&pdev->dev);

	switch (mode) {
	case USB_DR_MODE_HOST:
		regmap_update_bits(data->stg_syscon,
			data->stg_usb_mode,
			USB_STRAP_MASK,
			USB_STRAP_HOST);
		regmap_update_bits(data->stg_syscon,
			data->stg_usb_mode,
			USB_SUSPENDM_MASK,
			USB_SUSPENDM_HOST);
		data->phy_mode = PHY_MODE_USB_HOST;
		break;

	case USB_DR_MODE_PERIPHERAL:
		regmap_update_bits(data->stg_syscon, data->stg_usb_mode,
			USB_STRAP_MASK, USB_STRAP_DEVICE);
		regmap_update_bits(data->stg_syscon, data->stg_usb_mode,
			USB_SUSPENDM_MASK, 0);
		data->phy_mode = PHY_MODE_USB_DEVICE;
		break;

	case USB_DR_MODE_OTG:
		data->phy_mode = PHY_MODE_USB_OTG;
	default:
		break;
	}
}

static int cdns_clk_rst_init(struct cdns_starfive *data)
{
	int ret;

	data->num_clks = devm_clk_bulk_get_all(data->dev, &data->clks);
	if (data->num_clks < 0)
		return dev_err_probe(data->dev, -ENODEV,
			"Failed to get clocks\n");

	data->resets = devm_reset_control_array_get_exclusive(data->dev);
	if (IS_ERR(data->resets)) {
		return dev_err_probe(data->dev, PTR_ERR(data->resets),
			"Failed to get resets");
	}

	ret = clk_bulk_prepare_enable(data->num_clks, data->clks);
	if (ret)
		return dev_err_probe(data->dev, ret,
			"failed to enable clocks\n");

	ret = reset_control_deassert(data->resets);
	if (ret) {
		ret = dev_err_probe(data->dev, ret,
			"failed to reset clocks\n");
		goto err_clk_init;
	}

	return ret;

err_clk_init:
	clk_bulk_disable_unprepare(data->num_clks, data->clks);
	return ret;
}

static int cdns3_starfive_phy_init(struct device *dev, struct cdns_starfive *data)
{
	int ret;

	ret = phy_init(data->usb2_phy);
	if (ret)
		return ret;

	ret = phy_init(data->usb3_phy);
	if (ret)
		goto err_phy3_init;

	ret = set_phy_power_on(data);
	if (ret)
		goto err_phy_power_on;

	phy_set_mode(data->usb2_phy, data->phy_mode);
	phy_set_mode(data->usb3_phy, data->phy_mode);

	return 0;

err_phy_power_on:
	phy_exit(data->usb3_phy);
err_phy3_init:
	phy_exit(data->usb2_phy);
	return ret;
}

static int cdns3_starfive_platform_device_add(struct platform_device *pdev,
		struct cdns_starfive *data)
{
	struct platform_device *cdns3;
	struct resource	cdns_res[CDNS_RESOURCES_NUM], *res;
	struct device *dev = &pdev->dev;
	const char *reg_name[CDNS_IOMEM_RESOURCES_NUM] = {"otg", "xhci", "dev"};
	const char *irq_name[CDNS_IRQ_RESOURCES_NUM] = {"host", "peripheral", "otg", "wakeup"};
	int i, ret, res_idx = 0;

	cdns3 = platform_device_alloc("cdns-usb3", PLATFORM_DEVID_AUTO);
	if (!cdns3)
		return dev_err_probe(dev, -ENOMEM,
			"couldn't alloc cdns3 usb device\n");

	cdns3->dev.parent = dev;
	memset(cdns_res, 0, sizeof(cdns_res));

	for (i = 0; i < CDNS_IOMEM_RESOURCES_NUM; i++) {
		res = platform_get_resource_byname(pdev, IORESOURCE_MEM, reg_name[i]);
		if (!res) {
			ret = dev_err_probe(dev,
				-ENXIO, "couldn't get %s reg resource\n", reg_name[i]);
			goto free_memory;
		}
		cdns_res[res_idx] = *res;
		res_idx++;
	}

	for (i = 0; i < CDNS_IRQ_RESOURCES_NUM; i++) {
		if (i == CDNS_IRQ_WAKEUP_INDEX) {
			ret = platform_get_irq_byname_optional(pdev, irq_name[i]);
			if (ret < 0)
				continue;
		} else {
			ret = platform_get_irq_byname(pdev, irq_name[i]);
			if (ret < 0) {
				dev_err(dev, "couldn't get %s irq\n", irq_name[i]);
				goto free_memory;
			}
		}
		cdns_res[res_idx].start = ret;
		cdns_res[res_idx].end = ret;
		cdns_res[res_idx].flags = IORESOURCE_IRQ;
		cdns_res[res_idx].name = irq_name[i];
		res_idx++;
	}

	ret = platform_device_add_resources(cdns3, cdns_res, res_idx);
	if (ret) {
		dev_err(dev, "couldn't add res to cdns3 device\n");
		goto free_memory;
	}

	ret = platform_device_add(cdns3);
	if (ret) {
		dev_err(dev, "failed to register cdns3 device\n");
		goto free_memory;
	}

	return ret;
free_memory:
	platform_device_put(cdns3);
	return ret;
}

static int cdns_starfive_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct cdns_starfive *data;
	unsigned int args;
	int ret;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	platform_set_drvdata(pdev, data);

	data->dev = dev;

	data->stg_syscon = syscon_regmap_lookup_by_phandle_args(pdev->dev.of_node,
		"starfive,stg-syscon", 1, &args);

	if (IS_ERR(data->stg_syscon))
		return dev_err_probe(dev, PTR_ERR(data->stg_syscon),
			"Failed to parse starfive,stg-syscon\n");

	data->stg_usb_mode = args;

	cdns_mode_init(pdev, data);

	ret = cdns_clk_rst_init(data);
	if (ret)
		return ret;

	data->usb2_phy = devm_phy_optional_get(dev, "usb2-phy");
	if (IS_ERR(data->usb2_phy))
		return dev_err_probe(dev, PTR_ERR(data->usb2_phy),
			"Failed to parse usb2 phy\n");

	data->usb3_phy = devm_phy_optional_get(dev, "usb3-phy");
	if (IS_ERR(data->usb3_phy))
		return dev_err_probe(dev, PTR_ERR(data->usb3_phy),
			"Failed to parse usb3 phy\n");

	cdns3_starfive_phy_init(dev, data);

	ret = cdns3_starfive_platform_device_add(pdev, data);
	if (ret) {
		set_phy_power_off(data);
		phy_exit(data->usb3_phy);
		phy_exit(data->usb2_phy);
		reset_control_assert(data->resets);
		clk_bulk_disable_unprepare(data->num_clks, data->clks);
		return dev_err_probe(dev, ret, "Failed to create children\n");
	}

	device_set_wakeup_capable(dev, true);
	pm_runtime_set_active(dev);
	pm_runtime_enable(dev);

	dev_info(dev, "usb mode %d probe success\n", data->phy_mode);

	return 0;
}

static int cdns_starfive_remove_core(struct device *dev, void *c)
{
	struct platform_device *pdev = to_platform_device(dev);

	platform_device_unregister(pdev);

	return 0;
}

static int cdns_starfive_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct cdns_starfive *data = dev_get_drvdata(dev);

	pm_runtime_get_sync(dev);
	device_for_each_child(dev, NULL, cdns_starfive_remove_core);

	set_phy_power_off(data);
	phy_exit(data->usb2_phy);
	phy_exit(data->usb3_phy);

	reset_control_assert(data->resets);
	clk_bulk_disable_unprepare(data->num_clks, data->clks);
	pm_runtime_disable(dev);
	pm_runtime_put_noidle(dev);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

#ifdef CONFIG_PM
static int cdns_starfive_resume(struct device *dev)
{
	struct cdns_starfive *data = dev_get_drvdata(dev);
	int ret;

	ret = clk_bulk_prepare_enable(data->num_clks, data->clks);
	if (ret)
		return ret;

	ret = reset_control_deassert(data->resets);
	if (ret)
		return ret;

	ret = cdns3_starfive_phy_init(dev, data);

	return ret;
}

static int cdns_starfive_suspend(struct device *dev)
{
	struct cdns_starfive *data = dev_get_drvdata(dev);

	set_phy_power_off(data);
	phy_exit(data->usb2_phy);
	phy_exit(data->usb3_phy);
	reset_control_assert(data->resets);
	clk_bulk_disable_unprepare(data->num_clks, data->clks);

	return 0;
}
#endif

static const struct dev_pm_ops cdns_starfive_pm_ops = {
	SET_RUNTIME_PM_OPS(cdns_starfive_suspend, cdns_starfive_resume, NULL)
	SET_SYSTEM_SLEEP_PM_OPS(cdns_starfive_suspend, cdns_starfive_resume)
};

static const struct of_device_id cdns_starfive_of_match[] = {
	{ .compatible = "starfive,jh7110-usb", },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, cdns_starfive_of_match);

static struct platform_driver cdns_starfive_driver = {
	.probe		= cdns_starfive_probe,
	.remove		= cdns_starfive_remove,
	.driver		= {
		.name	= "cdns3-starfive",
		.of_match_table	= cdns_starfive_of_match,
		.pm	= &cdns_starfive_pm_ops,
	},
};
module_platform_driver(cdns_starfive_driver);

MODULE_ALIAS("platform:cdns3-starfive");
MODULE_AUTHOR("YanHong Wang <yanhong.wang@starfivetech.com>");
MODULE_AUTHOR("Mason Huo <mason.huo@starfivetech.com>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Cadence USB3 StarFive Glue Layer");
