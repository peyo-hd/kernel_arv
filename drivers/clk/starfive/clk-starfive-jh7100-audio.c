// SPDX-License-Identifier: GPL-2.0
/*
 * StarFive JH7100 Audio Clock Driver
 *
 * Copyright (C) 2021 Emil Renner Berthing <kernel@esmil.dk>
 * Copyright (C) 2021 Walker Chen <walker.chen@starfivetech.com>
 */

#include <linux/bits.h>
#include <linux/clk-provider.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

#include <dt-bindings/clock/starfive-jh7100-audio.h>

#include "clk-starfive-jh7100.h"

/* external clocks */
#define JH7100_AUDCLK_AUDIO_SRC			(JH7100_AUDCLK_END + 0)
#define JH7100_AUDCLK_AUDIO_12288		(JH7100_AUDCLK_END + 1)
#define JH7100_AUDCLK_DOM7AHB_BUS		(JH7100_AUDCLK_END + 2)
#define JH7100_AUDCLK_VAD_INTMEM                (JH7100_AUDCLK_END + 3)

static const struct jh7100_clk_data jh7100_audclk_data[] = {
	JH7100__GMD(JH7100_AUDCLK_ADC_MCLK, "adc_mclk", 0, 15, 2,
		    JH7100_AUDCLK_AUDIO_SRC,
		    JH7100_AUDCLK_AUDIO_12288),
	JH7100__GMD(JH7100_AUDCLK_I2S1_MCLK, "i2s1_mclk", 0, 15, 2,
		    JH7100_AUDCLK_AUDIO_SRC,
		    JH7100_AUDCLK_AUDIO_12288),
	JH7100_GATE(JH7100_AUDCLK_I2SADC_APB, "i2sadc_apb", 0, JH7100_AUDCLK_APB0_BUS),
	JH7100_MDIV(JH7100_AUDCLK_I2SADC_BCLK, "i2sadc_bclk", 31, 2,
		    JH7100_AUDCLK_ADC_MCLK,
		    JH7100_AUDCLK_I2SADC_BCLK_IOPAD),
	JH7100__INV(JH7100_AUDCLK_I2SADC_BCLK_N, "i2sadc_bclk_n", JH7100_AUDCLK_I2SADC_BCLK),
	JH7100_MDIV(JH7100_AUDCLK_I2SADC_LRCLK, "i2sadc_lrclk", 63, 3,
		    JH7100_AUDCLK_I2SADC_BCLK_N,
		    JH7100_AUDCLK_I2SADC_LRCLK_IOPAD,
		    JH7100_AUDCLK_I2SADC_BCLK),
	JH7100_GATE(JH7100_AUDCLK_PDM_APB, "pdm_apb", 0, JH7100_AUDCLK_APB0_BUS),
	JH7100__GMD(JH7100_AUDCLK_PDM_MCLK, "pdm_mclk", 0, 15, 2,
		    JH7100_AUDCLK_AUDIO_SRC,
		    JH7100_AUDCLK_AUDIO_12288),
	JH7100_GATE(JH7100_AUDCLK_I2SVAD_APB, "i2svad_apb", 0, JH7100_AUDCLK_APB0_BUS),
	JH7100__GMD(JH7100_AUDCLK_SPDIF, "spdif", 0, 15, 2,
		    JH7100_AUDCLK_AUDIO_SRC,
		    JH7100_AUDCLK_AUDIO_12288),
	JH7100_GATE(JH7100_AUDCLK_SPDIF_APB, "spdif_apb", 0, JH7100_AUDCLK_APB0_BUS),
	JH7100_GATE(JH7100_AUDCLK_PWMDAC_APB, "pwmdac_apb", 0, JH7100_AUDCLK_APB0_BUS),
	JH7100__GMD(JH7100_AUDCLK_DAC_MCLK, "dac_mclk", 0, 15, 2,
		    JH7100_AUDCLK_AUDIO_SRC,
		    JH7100_AUDCLK_AUDIO_12288),
	JH7100_GATE(JH7100_AUDCLK_I2SDAC_APB, "i2sdac_apb", 0, JH7100_AUDCLK_APB0_BUS),
	JH7100_MDIV(JH7100_AUDCLK_I2SDAC_BCLK, "i2sdac_bclk", 31, 2,
		    JH7100_AUDCLK_DAC_MCLK,
		    JH7100_AUDCLK_I2SDAC_BCLK_IOPAD),
	JH7100__INV(JH7100_AUDCLK_I2SDAC_BCLK_N, "i2sdac_bclk_n", JH7100_AUDCLK_I2SDAC_BCLK),
	JH7100_MDIV(JH7100_AUDCLK_I2SDAC_LRCLK, "i2sdac_lrclk", 63, 3,
		    JH7100_AUDCLK_I2SDAC_BCLK_N,
		    JH7100_AUDCLK_I2SDAC_LRCLK_IOPAD,
		    JH7100_AUDCLK_I2SDAC_BCLK),
	JH7100_GATE(JH7100_AUDCLK_I2S1_APB, "i2s1_apb", 0, JH7100_AUDCLK_APB0_BUS),
	JH7100_MDIV(JH7100_AUDCLK_I2S1_BCLK, "i2s1_bclk", 31, 2,
		    JH7100_AUDCLK_I2S1_MCLK,
		    JH7100_AUDCLK_I2SDAC_BCLK_IOPAD),
	JH7100__INV(JH7100_AUDCLK_I2S1_BCLK_N, "i2s1_bclk_n", JH7100_AUDCLK_I2S1_BCLK),
	JH7100_MDIV(JH7100_AUDCLK_I2S1_LRCLK, "i2s1_lrclk", 63, 3,
		    JH7100_AUDCLK_I2S1_BCLK_N,
		    JH7100_AUDCLK_I2SDAC_LRCLK_IOPAD,
		    JH7100_AUDCLK_I2S1_BCLK),
	JH7100_GATE(JH7100_AUDCLK_I2SDAC16K_APB, "i2s1dac16k_apb", 0, JH7100_AUDCLK_APB0_BUS),
	JH7100__DIV(JH7100_AUDCLK_APB0_BUS, "apb0_bus", 8, JH7100_AUDCLK_DOM7AHB_BUS),
	JH7100_GATE(JH7100_AUDCLK_DMA1P_AHB, "dma1p_ahb", 0, JH7100_AUDCLK_DOM7AHB_BUS),
	JH7100_GATE(JH7100_AUDCLK_USB_APB, "usb_apb", CLK_IGNORE_UNUSED, JH7100_AUDCLK_APB_EN),
	JH7100_GDIV(JH7100_AUDCLK_USB_LPM, "usb_lpm", CLK_IGNORE_UNUSED, 4, JH7100_AUDCLK_USB_APB),
	JH7100_GDIV(JH7100_AUDCLK_USB_STB, "usb_stb", CLK_IGNORE_UNUSED, 3, JH7100_AUDCLK_USB_APB),
	JH7100__DIV(JH7100_AUDCLK_APB_EN, "apb_en", 8, JH7100_AUDCLK_DOM7AHB_BUS),
	JH7100__MUX(JH7100_AUDCLK_VAD_MEM, "vad_mem", 2,
		    JH7100_AUDCLK_VAD_INTMEM,
		    JH7100_AUDCLK_AUDIO_12288),
};

static void jh7100_clk_set_divider(struct jh7100_clk_priv *priv,
					unsigned int idx,
					unsigned int mask,
					unsigned int div)
{
	unsigned long flags;
	unsigned int value;
	void __iomem *reg = priv->base + 4 * idx;

	spin_lock_irqsave(&priv->rmw_lock, flags);
	value = readl_relaxed(reg) & ~mask;
	value |= div;
	writel_relaxed(value, reg);
	spin_unlock_irqrestore(&priv->rmw_lock, flags);
}

static struct clk_hw *jh7100_audclk_get(struct of_phandle_args *clkspec, void *data)
{
	struct jh7100_clk_priv *priv = data;
	unsigned int idx = clkspec->args[0];

	if (idx < JH7100_AUDCLK_I2SADC_BCLK_IOPAD)
		return &priv->reg[idx].hw;

	if (idx < JH7100_AUDCLK_END)
		return priv->pll[idx - JH7100_AUDCLK_I2SADC_BCLK_IOPAD];

	return ERR_PTR(-EINVAL);
}

static int jh7100_audclk_probe(struct platform_device *pdev)
{
	struct jh7100_clk_priv *priv;
	unsigned int idx;
	int ret;

	priv = devm_kzalloc(&pdev->dev, struct_size(priv, reg, JH7100_AUDCLK_I2SADC_BCLK_IOPAD), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	spin_lock_init(&priv->rmw_lock);
	priv->dev = &pdev->dev;
	priv->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

	priv->pll[0] = clk_hw_register_fixed_rate(priv->dev, "i2sadc_bclk_iopad", NULL, 0, 12288000);
	priv->pll[1] = clk_hw_register_fixed_rate(priv->dev, "i2sadc_lrclk_iopad", NULL, 0, 12288000);
	priv->pll[2] = clk_hw_register_fixed_rate(priv->dev, "i2sdac_bclk_iopad", NULL, 0, 12288000);
	priv->pll[3] = clk_hw_register_fixed_rate(priv->dev, "i2sdac_lrclk_iopad", NULL, 0, 12288000);
	priv->pll[4] = clk_hw_register_fixed_rate(priv->dev, "audio_codec_clk", NULL, 0, 24576000);

	// for special initialization
	jh7100_clk_set_divider(priv, JH7100_AUDCLK_I2SADC_BCLK, 0x1f, 0x1);
	jh7100_clk_set_divider(priv, JH7100_AUDCLK_I2SADC_LRCLK, 0x3f, 0x1);
	jh7100_clk_set_divider(priv, JH7100_AUDCLK_I2SDAC_BCLK, 0x1f, 0x1);
	jh7100_clk_set_divider(priv, JH7100_AUDCLK_I2SDAC_LRCLK, 0x3f, 0x1);

	for (idx = 0; idx < JH7100_AUDCLK_I2SADC_BCLK_IOPAD; idx++) {
		u32 max = jh7100_audclk_data[idx].max;
		struct clk_parent_data parents[4] = {};
		struct clk_init_data init = {
			.name = jh7100_audclk_data[idx].name,
			.ops = starfive_jh7100_clk_ops(max),
			.parent_data = parents,
			.num_parents = ((max & JH7100_CLK_MUX_MASK) >> JH7100_CLK_MUX_SHIFT) + 1,
			.flags = jh7100_audclk_data[idx].flags,
		};
		struct jh7100_clk *clk = &priv->reg[idx];
		unsigned int i;

		for (i = 0; i < init.num_parents; i++) {
			unsigned int pidx = jh7100_audclk_data[idx].parents[i];

			if (pidx < JH7100_AUDCLK_I2SADC_BCLK_IOPAD)
				parents[i].hw = &priv->reg[pidx].hw;
			else if (pidx < JH7100_AUDCLK_END)
				parents[i].hw = priv->pll[pidx - JH7100_AUDCLK_I2SADC_BCLK_IOPAD];
			else if (pidx == JH7100_AUDCLK_AUDIO_SRC)
				parents[i].fw_name = "audio_src";
			else if (pidx == JH7100_AUDCLK_AUDIO_12288)
				parents[i].fw_name = "audio_12288";
			else if (pidx == JH7100_AUDCLK_DOM7AHB_BUS)
				parents[i].fw_name = "dom7ahb_bus";
		}

		clk->hw.init = &init;
		clk->idx = idx;
		clk->max_div = max & JH7100_CLK_DIV_MASK;

		ret = devm_clk_hw_register(priv->dev, &clk->hw);
		if (ret)
			return ret;
	}

	return devm_of_clk_add_hw_provider(priv->dev, jh7100_audclk_get, priv);
}

static const struct of_device_id jh7100_audclk_match[] = {
	{ .compatible = "starfive,jh7100-audclk" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, jh7100_audclk_match);

static struct platform_driver jh7100_audclk_driver = {
	.probe = jh7100_audclk_probe,
	.driver = {
		.name = "clk-starfive-jh7100-audio",
		.of_match_table = jh7100_audclk_match,
	},
};
module_platform_driver(jh7100_audclk_driver);

MODULE_AUTHOR("Emil Renner Berthing");
MODULE_AUTHOR("Walker Chen");
MODULE_DESCRIPTION("StarFive JH7100 audio clock driver");
MODULE_LICENSE("GPL");
