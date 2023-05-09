// SPDX-License-Identifier: GPL-2.0+
/*
 * DPHY driver for the StarFive JH7110 SoC
 *
 * Copyright (C) 2023 StarFive Technology Co., Ltd.
 */

#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/reset.h>

#define STF_DPHY_APBCFGSAIF_SYSCFG(x)		(x)

#define STF_DPHY_DA_CDPHY_R100_CTRL0_2D1C_EFUSE_EN BIT(6)
#define STF_DPHY_DA_CDPHY_R100_CTRL0_2D1C_EFUSE_IN GENMASK(12, 7)
#define STF_DPHY_DA_CDPHY_R100_CTRL1_2D1C_EFUSE_EN BIT(19)
#define STF_DPHY_DA_CDPHY_R100_CTRL1_2D1C_EFUSE_IN GENMASK(25, 20)

#define STF_DPHY_DATA_BUS16_8			BIT(8)
#define STF_DPHY_DEBUG_MODE_SEL			GENMASK(15, 9)

#define STF_DPHY_ENABLE_CLK			BIT(6)
#define STF_DPHY_ENABLE_CLK1			BIT(7)
#define STF_DPHY_ENABLE_LAN0			BIT(8)
#define STF_DPHY_ENABLE_LAN1			BIT(9)
#define STF_DPHY_ENABLE_LAN2			BIT(10)
#define STF_DPHY_ENABLE_LAN3			BIT(11)
#define STF_DPHY_GPI_EN				GENMASK(17, 12)
#define STF_DPHY_HS_FREQ_CHANGE_CLK		BIT(18)
#define STF_DPHY_HS_FREQ_CHANGE_CLK1		BIT(19)
#define STF_DPHY_LANE_SWAP_CLK			GENMASK(22, 20)
#define STF_DPHY_LANE_SWAP_CLK1			GENMASK(25, 23)
#define STF_DPHY_LANE_SWAP_LAN0			GENMASK(28, 26)
#define STF_DPHY_LANE_SWAP_LAN1			GENMASK(31, 29)

#define STF_DPHY_LANE_SWAP_LAN2			GENMASK(2, 0)
#define STF_DPHY_LANE_SWAP_LAN3			GENMASK(5, 3)
#define STF_DPHY_MP_TEST_EN			BIT(6)
#define STF_DPHY_MP_TEST_MODE_SEL		GENMASK(11, 7)
#define STF_DPHY_PLL_CLK_SEL			GENMASK(21, 12)
#define STF_DPHY_PRECOUNTER_IN_CLK		GENMASK(29, 22)

#define STF_DPHY_PRECOUNTER_IN_CLK1		GENMASK(7, 0)
#define STF_DPHY_PRECOUNTER_IN_LAN0		GENMASK(15, 8)
#define STF_DPHY_PRECOUNTER_IN_LAN1		GENMASK(23, 16)
#define STF_DPHY_PRECOUNTER_IN_LAN2		GENMASK(31, 24)

#define STF_DPHY_PRECOUNTER_IN_LAN3		GENMASK(7, 0)
#define STF_DPHY_RX_1C2C_SEL			BIT(8)

#define STF_MAP_LANES_NUM			6

struct regval {
	u32 addr;
	u32 val;
};

struct stf_dphy {
	struct device *dev;
	void __iomem *regs;
	struct clk *cfg_clk;
	struct clk *ref_clk;
	struct clk *tx_clk;
	struct reset_control *rstc;
	struct regulator *mipi_0p9;
	struct phy *phy;
	u8 maps[STF_MAP_LANES_NUM];
};

static const struct regval stf_dphy_init_list[] = {
	{ STF_DPHY_APBCFGSAIF_SYSCFG(4), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(8), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(12), 0x0000fff0 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(16), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(20), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(24), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(28), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(32), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(36), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(40), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(40), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(48), 0x24000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(52), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(56), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(60), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(64), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(68), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(72), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(76), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(80), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(84), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(88), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(92), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(96), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(100), 0x02000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(104), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(108), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(112), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(116), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(120), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(124), 0x0000000c },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(128), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(132), 0xcc500000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(136), 0x000000cc },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(140), 0x00000000 },
	{ STF_DPHY_APBCFGSAIF_SYSCFG(144), 0x00000000 },
};

static int stf_dphy_configure(struct phy *phy, union phy_configure_opts *opts)
{
	struct stf_dphy *dphy = phy_get_drvdata(phy);
	int i;

	for (i = 0; i < ARRAY_SIZE(stf_dphy_init_list); i++)
		writel(stf_dphy_init_list[i].val,
		       dphy->regs + stf_dphy_init_list[i].addr);

	writel(FIELD_PREP(STF_DPHY_DA_CDPHY_R100_CTRL0_2D1C_EFUSE_EN, 1) |
	       FIELD_PREP(STF_DPHY_DA_CDPHY_R100_CTRL0_2D1C_EFUSE_IN, 0x1b) |
	       FIELD_PREP(STF_DPHY_DA_CDPHY_R100_CTRL1_2D1C_EFUSE_EN, 1) |
	       FIELD_PREP(STF_DPHY_DA_CDPHY_R100_CTRL1_2D1C_EFUSE_IN, 0x1b),
	       dphy->regs + STF_DPHY_APBCFGSAIF_SYSCFG(0));

	writel(FIELD_PREP(STF_DPHY_DATA_BUS16_8, 0) |
	       FIELD_PREP(STF_DPHY_DEBUG_MODE_SEL, 0x5a),
	       dphy->regs + STF_DPHY_APBCFGSAIF_SYSCFG(184));

	writel(FIELD_PREP(STF_DPHY_ENABLE_CLK, 1) |
	       FIELD_PREP(STF_DPHY_ENABLE_CLK1, 1) |
	       FIELD_PREP(STF_DPHY_ENABLE_LAN0, 1) |
	       FIELD_PREP(STF_DPHY_ENABLE_LAN1, 1) |
	       FIELD_PREP(STF_DPHY_ENABLE_LAN2, 1) |
	       FIELD_PREP(STF_DPHY_ENABLE_LAN3, 1) |
	       FIELD_PREP(STF_DPHY_GPI_EN, 0) |
	       FIELD_PREP(STF_DPHY_HS_FREQ_CHANGE_CLK, 0) |
	       FIELD_PREP(STF_DPHY_HS_FREQ_CHANGE_CLK1, 0) |
	       FIELD_PREP(STF_DPHY_LANE_SWAP_CLK, dphy->maps[0]) |
	       FIELD_PREP(STF_DPHY_LANE_SWAP_CLK1, dphy->maps[5]) |
	       FIELD_PREP(STF_DPHY_LANE_SWAP_LAN0, dphy->maps[1]) |
	       FIELD_PREP(STF_DPHY_LANE_SWAP_LAN1, dphy->maps[2]),
	       dphy->regs + STF_DPHY_APBCFGSAIF_SYSCFG(188));

	writel(FIELD_PREP(STF_DPHY_LANE_SWAP_LAN2, dphy->maps[3]) |
	       FIELD_PREP(STF_DPHY_LANE_SWAP_LAN3, dphy->maps[4]) |
	       FIELD_PREP(STF_DPHY_MP_TEST_EN, 0) |
	       FIELD_PREP(STF_DPHY_MP_TEST_MODE_SEL, 0) |
	       FIELD_PREP(STF_DPHY_PLL_CLK_SEL, 0x37c) |
	       FIELD_PREP(STF_DPHY_PRECOUNTER_IN_CLK, 8),
	       dphy->regs + STF_DPHY_APBCFGSAIF_SYSCFG(192));

	writel(FIELD_PREP(STF_DPHY_PRECOUNTER_IN_CLK1, 8) |
	       FIELD_PREP(STF_DPHY_PRECOUNTER_IN_LAN0, 7) |
	       FIELD_PREP(STF_DPHY_PRECOUNTER_IN_LAN1, 7) |
	       FIELD_PREP(STF_DPHY_PRECOUNTER_IN_LAN2, 7),
	       dphy->regs + STF_DPHY_APBCFGSAIF_SYSCFG(196));

	writel(FIELD_PREP(STF_DPHY_PRECOUNTER_IN_LAN3, 7) |
	       FIELD_PREP(STF_DPHY_RX_1C2C_SEL, 0),
	       dphy->regs + STF_DPHY_APBCFGSAIF_SYSCFG(200));

	return 0;
}

static int stf_dphy_power_on(struct phy *phy)
{
	struct stf_dphy *dphy = phy_get_drvdata(phy);
	int ret;

	pm_runtime_get_sync(dphy->dev);

	ret = regulator_enable(dphy->mipi_0p9);
	if (ret)
		return ret;

	clk_set_rate(dphy->cfg_clk, 99000000);
	clk_set_rate(dphy->ref_clk, 49500000);
	clk_set_rate(dphy->tx_clk, 19800000);
	reset_control_deassert(dphy->rstc);

	return 0;
}

static int stf_dphy_power_off(struct phy *phy)
{
	struct stf_dphy *dphy = phy_get_drvdata(phy);

	reset_control_assert(dphy->rstc);

	regulator_disable(dphy->mipi_0p9);

	pm_runtime_put_sync(dphy->dev);

	return 0;
}

static const struct phy_ops stf_dphy_ops = {
	.configure = stf_dphy_configure,
	.power_on  = stf_dphy_power_on,
	.power_off = stf_dphy_power_off,
};

static int stf_dphy_probe(struct platform_device *pdev)
{
	struct phy_provider *phy_provider;
	struct stf_dphy *dphy;
	int ret;

	dphy = devm_kzalloc(&pdev->dev, sizeof(*dphy), GFP_KERNEL);
	if (!dphy)
		return -ENOMEM;

	dev_set_drvdata(&pdev->dev, dphy);
	dphy->dev = &pdev->dev;

	dphy->regs = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(dphy->regs))
		return PTR_ERR(dphy->regs);

	dphy->cfg_clk = devm_clk_get(&pdev->dev, "cfg");
	if (IS_ERR(dphy->cfg_clk))
		return PTR_ERR(dphy->cfg_clk);

	dphy->ref_clk = devm_clk_get(&pdev->dev, "ref");
	if (IS_ERR(dphy->ref_clk))
		return PTR_ERR(dphy->ref_clk);

	dphy->tx_clk = devm_clk_get(&pdev->dev, "tx");
	if (IS_ERR(dphy->tx_clk))
		return PTR_ERR(dphy->tx_clk);

	dphy->rstc = devm_reset_control_array_get_exclusive(&pdev->dev);
	if (IS_ERR(dphy->rstc))
		return PTR_ERR(dphy->rstc);

	dphy->mipi_0p9 = devm_regulator_get(&pdev->dev, "mipi_0p9");
	if (IS_ERR(dphy->mipi_0p9))
		return PTR_ERR(dphy->mipi_0p9);

	ret = of_property_read_u8_array(pdev->dev.of_node, "lane_maps",
					dphy->maps, STF_MAP_LANES_NUM);
	if (ret < 0) {
		dev_err(&pdev->dev, "Error to get lane_maps\n");
		return ret;
	}

	dphy->phy = devm_phy_create(&pdev->dev, NULL, &stf_dphy_ops);
	if (IS_ERR(dphy->phy)) {
		dev_err(&pdev->dev, "Failed to create PHY\n");
		return PTR_ERR(dphy->phy);
	}

	pm_runtime_enable(&pdev->dev);

	phy_set_drvdata(dphy->phy, dphy);
	phy_provider = devm_of_phy_provider_register(&pdev->dev,
						     of_phy_simple_xlate);

	return PTR_ERR_OR_ZERO(phy_provider);
}

static const struct of_device_id stf_dphy_dt_ids[] = {
	{ .compatible = "starfive,jh7110-dphy-rx" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, stf_dphy_dt_ids);

static struct platform_driver stf_dphy_driver = {
	.probe = stf_dphy_probe,
	.driver = {
		.name	= "starfive-dphy-rx",
		.of_match_table = stf_dphy_dt_ids,
	},
};
module_platform_driver(stf_dphy_driver);

MODULE_AUTHOR("Jack Zhu <jack.zhu@starfivetech.com>");
MODULE_AUTHOR("Changhuang Liang <changhuang.liang@starfivetech.com>");
MODULE_DESCRIPTION("StarFive DPHY RX driver");
MODULE_LICENSE("GPL");
