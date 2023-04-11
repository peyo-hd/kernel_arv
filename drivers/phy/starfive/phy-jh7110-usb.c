// SPDX-License-Identifier: GPL-2.0+
/*
 * StarFive JH7110 USB 2.0 PHY driver
 *
 * Copyright (C) 2023 Minda Chen <minda.chen@starfivetech.com>
 */

#include <linux/bits.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/usb/of.h>

#define USB_125M_CLK_RATE		125000000
#define USB_LS_KEEPALIVE_OFF		0x4
#define USB_LS_KEEPALIVE_ENABLE		BIT(4)

struct jh7110_usb2_phy {
	struct phy *phy;
	void __iomem *regs;
	struct clk *usb_125m_clk;
	struct clk *app_125;
	enum phy_mode mode;
};

static void jh7110_usb2_mode_set(struct jh7110_usb2_phy *phy)
{
	unsigned int val;

	if (phy->mode != PHY_MODE_USB_HOST) {
		/* Enable the LS speed keep-alive signal */
		val = readl(phy->regs + USB_LS_KEEPALIVE_OFF);
		val |= USB_LS_KEEPALIVE_ENABLE;
		writel(val, phy->regs + USB_LS_KEEPALIVE_OFF);
	}
}

static int jh7110_usb2_phy_set_mode(struct phy *_phy,
				  enum phy_mode mode, int submode)
{
	struct jh7110_usb2_phy *phy = phy_get_drvdata(_phy);

	switch (mode) {
	case PHY_MODE_USB_HOST:
	case PHY_MODE_USB_DEVICE:
	case PHY_MODE_USB_OTG:
		break;
	default:
		return -EINVAL;
	}

	if (mode != phy->mode) {
		dev_info(&_phy->dev, "Changing phy to %d\n", mode);
		phy->mode = mode;
		jh7110_usb2_mode_set(phy);
	}

	return 0;
}

static int jh7110_usb2_phy_init(struct phy *_phy)
{
	struct jh7110_usb2_phy *phy = phy_get_drvdata(_phy);
	int ret;

	ret = clk_set_rate(phy->usb_125m_clk, USB_125M_CLK_RATE);
	if (ret)
		return ret;

	ret = clk_prepare_enable(phy->app_125);
	if (ret)
		return ret;

	return 0;
}

static int jh7110_usb2_phy_exit(struct phy *_phy)
{
	struct jh7110_usb2_phy *phy = phy_get_drvdata(_phy);

	clk_disable_unprepare(phy->app_125);

	return 0;
}

static const struct phy_ops jh7110_usb2_phy_ops = {
	.init		= jh7110_usb2_phy_init,
	.exit		= jh7110_usb2_phy_exit,
	.set_mode	= jh7110_usb2_phy_set_mode,
	.owner		= THIS_MODULE,
};

static int jh7110_usb_phy_probe(struct platform_device *pdev)
{
	struct jh7110_usb2_phy *phy;
	struct device *dev = &pdev->dev;
	struct phy_provider *phy_provider;

	phy = devm_kzalloc(dev, sizeof(*phy), GFP_KERNEL);
	if (!phy)
		return -ENOMEM;

	phy->usb_125m_clk = devm_clk_get(dev, "125m");
	if (IS_ERR(phy->usb_125m_clk))
		return dev_err_probe(dev, PTR_ERR(phy->usb_125m_clk),
			"Failed to get 125m clock\n");

	phy->app_125 = devm_clk_get(dev, "app_125");
	if (IS_ERR(phy->app_125))
		return dev_err_probe(dev, PTR_ERR(phy->app_125),
			"Failed to get app 125m clock\n");

	phy->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(phy->regs))
		return dev_err_probe(dev, PTR_ERR(phy->regs),
			"Failed to map phy base\n");

	phy->phy = devm_phy_create(dev, NULL, &jh7110_usb2_phy_ops);
	if (IS_ERR(phy->phy))
		return dev_err_probe(dev, PTR_ERR(phy->phy),
			"Failed to create phy\n");

	platform_set_drvdata(pdev, phy);
	phy_set_drvdata(phy->phy, phy);
	phy_provider = devm_of_phy_provider_register(dev, of_phy_simple_xlate);

	return PTR_ERR_OR_ZERO(phy_provider);
}

static int jh7110_usb_phy_remove(struct platform_device *pdev)
{
	struct jh7110_usb2_phy *phy = platform_get_drvdata(pdev);

	clk_disable_unprepare(phy->app_125);
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static const struct of_device_id jh7110_usb_phy_of_match[] = {
	{ .compatible = "starfive,jh7110-usb-phy" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, jh7110_usb_phy_of_match);

static struct platform_driver jh7110_usb_phy_driver = {
	.probe	= jh7110_usb_phy_probe,
	.remove	= jh7110_usb_phy_remove,
	.driver = {
		.of_match_table	= jh7110_usb_phy_of_match,
		.name  = "jh7110-usb-phy",
	}
};
module_platform_driver(jh7110_usb_phy_driver);

MODULE_DESCRIPTION("StarFive JH7110 USB 2.0 PHY driver");
MODULE_AUTHOR("Minda Chen <minda.chen@starfivetech.com>");
MODULE_LICENSE("GPL");
