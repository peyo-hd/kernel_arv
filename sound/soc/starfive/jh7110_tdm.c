// SPDX-License-Identifier: GPL-2.0
/*
 * TDM driver for the StarFive JH7110 SoC
 *
 * Copyright (C) 2023 StarFive Technology Co., Ltd.
 */

#include <linux/clk.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/pm_runtime.h>
#include <linux/regmap.h>
#include <linux/reset.h>
#include <sound/initval.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dai.h>
#include "jh7110_tdm.h"

static inline u32 jh7110_tdm_readl(struct jh7110_tdm_dev *tdm, u16 reg)
{
	return readl_relaxed(tdm->tdm_base + reg);
}

static inline void jh7110_tdm_writel(struct jh7110_tdm_dev *tdm, u16 reg, u32 val)
{
	writel_relaxed(val, tdm->tdm_base + reg);
}

static void jh7110_tdm_save_context(struct jh7110_tdm_dev *tdm,
				    struct snd_pcm_substream *substream)
{
	tdm->saved_pcmgbcr = jh7110_tdm_readl(tdm, TDM_PCMGBCR);
	tdm->saved_pcmdiv = jh7110_tdm_readl(tdm, TDM_PCMDIV);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		tdm->saved_pcmtxcr = jh7110_tdm_readl(tdm, TDM_PCMTXCR);
	else
		tdm->saved_pcmrxcr = jh7110_tdm_readl(tdm, TDM_PCMRXCR);
}

static void jh7110_tdm_start(struct jh7110_tdm_dev *tdm, struct snd_pcm_substream *substream)
{
	u32 data;
	unsigned int val;

	data = jh7110_tdm_readl(tdm, TDM_PCMGBCR);
	jh7110_tdm_writel(tdm, TDM_PCMGBCR, data | PCMGBCR_ENABLE);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		val = jh7110_tdm_readl(tdm, TDM_PCMTXCR);
		jh7110_tdm_writel(tdm, TDM_PCMTXCR, val | PCMTXCR_TXEN);
	} else {
		val = jh7110_tdm_readl(tdm, TDM_PCMRXCR);
		jh7110_tdm_writel(tdm, TDM_PCMRXCR, val | PCMRXCR_RXEN);
	}
}

static void jh7110_tdm_stop(struct jh7110_tdm_dev *tdm, struct snd_pcm_substream *substream)
{
	unsigned int val;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		val = jh7110_tdm_readl(tdm, TDM_PCMTXCR);
		val &= ~PCMTXCR_TXEN;
		jh7110_tdm_writel(tdm, TDM_PCMTXCR, val);
	} else {
		val = jh7110_tdm_readl(tdm, TDM_PCMRXCR);
		val &= ~PCMRXCR_RXEN;
		jh7110_tdm_writel(tdm, TDM_PCMRXCR, val);
	}
}

static int jh7110_tdm_syncdiv(struct jh7110_tdm_dev *tdm)
{
	u32 sl, sscale, syncdiv;

	sl = (tdm->rx.sl >= tdm->tx.sl) ? tdm->rx.sl : tdm->tx.sl;
	sscale = (tdm->rx.sscale >= tdm->tx.sscale) ? tdm->rx.sscale : tdm->tx.sscale;
	syncdiv = tdm->pcmclk / tdm->samplerate - 1;

	if ((syncdiv + 1) < (sl * sscale)) {
		dev_err(tdm->dev, "Failed to set syncdiv!\n");
		return -EINVAL;
	}

	if (tdm->syncm == TDM_SYNCM_LONG &&
	    (tdm->rx.sscale <= 1 || tdm->tx.sscale <= 1)) {
		if ((syncdiv + 1) <= sl) {
			dev_err(tdm->dev, "Wrong syncdiv! It must be (syncdiv+1) > max[tx.sl, rx.sl]\n");
			return -EINVAL;
		}
	}

	jh7110_tdm_writel(tdm, TDM_PCMDIV, syncdiv);
	return 0;
}

static void jh7110_tdm_control(struct jh7110_tdm_dev *tdm)
{
	u32 data;

	data = (tdm->clkpolity << CLKPOL_BIT) |
		(tdm->elm << ELM_BIT) |
		(tdm->syncm << SYNCM_BIT) |
		(tdm->ms_mode << MS_BIT);
	jh7110_tdm_writel(tdm, TDM_PCMGBCR, data);
}

static void jh7110_tdm_config(struct jh7110_tdm_dev *tdm,
			      struct snd_pcm_substream *substream)
{
	u32 datarx, datatx;

	jh7110_tdm_control(tdm);
	jh7110_tdm_syncdiv(tdm);

	datarx = (tdm->rx.ifl << IFL_BIT) |
		  (tdm->rx.wl << WL_BIT) |
		  (tdm->rx.sscale << SSCALE_BIT) |
		  (tdm->rx.sl << SL_BIT) |
		  (tdm->rx.lrj << LRJ_BIT);

	datatx = (tdm->tx.ifl << IFL_BIT) |
		  (tdm->tx.wl << WL_BIT) |
		  (tdm->tx.sscale << SSCALE_BIT) |
		  (tdm->tx.sl << SL_BIT) |
		  (tdm->tx.lrj << LRJ_BIT);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		jh7110_tdm_writel(tdm, TDM_PCMTXCR, datatx);
	else
		jh7110_tdm_writel(tdm, TDM_PCMRXCR, datarx);
}

static void jh7110_tdm_clk_disable(struct jh7110_tdm_dev *tdm)
{
	clk_disable_unprepare(tdm->clk_tdm);
	clk_disable_unprepare(tdm->clk_tdm_ext);
	clk_disable_unprepare(tdm->clk_tdm_internal);
	clk_disable_unprepare(tdm->clk_tdm_apb);
	clk_disable_unprepare(tdm->clk_tdm_ahb);
	clk_disable_unprepare(tdm->clk_mclk_inner);
}

static int jh7110_tdm_clk_enable(struct jh7110_tdm_dev *tdm)
{
	int ret;

	ret = clk_prepare_enable(tdm->clk_mclk_inner);
	if (ret) {
		dev_err(tdm->dev, "failed to prepare enable clk_mclk_inner\n");
		return ret;
	}

	ret = clk_prepare_enable(tdm->clk_tdm_ahb);
	if (ret) {
		dev_err(tdm->dev, "Failed to prepare enable clk_tdm_ahb\n");
		goto dis_mclk_inner_clk;
	}

	ret = clk_prepare_enable(tdm->clk_tdm_apb);
	if (ret) {
		dev_err(tdm->dev, "Failed to prepare enable clk_tdm_apb\n");
		goto dis_tdm_ahb_clk;
	}

	ret = clk_prepare_enable(tdm->clk_tdm_internal);
	if (ret) {
		dev_err(tdm->dev, "Failed to prepare enable clk_tdm_intl\n");
		goto dis_tdm_apb_clk;
	}

	ret = clk_prepare_enable(tdm->clk_tdm_ext);
	if (ret) {
		dev_err(tdm->dev, "Failed to prepare enable clk_tdm_ext\n");
		goto dis_tdm_internal_clk;
	}

	ret = clk_prepare_enable(tdm->clk_tdm);
	if (ret) {
		dev_err(tdm->dev, "Failed to prepare enable clk_tdm\n");
		goto dis_tdm_ext_clk;
	}

	ret = reset_control_deassert(tdm->resets);
	if (ret) {
		dev_err(tdm->dev, "%s: failed to deassert tdm resets\n", __func__);
		goto dis_tdm_clk;
	}

	ret = clk_set_parent(tdm->clk_tdm, tdm->clk_tdm_ext);
	if (ret) {
		dev_err(tdm->dev, "Can't set clock source for clk_tdm: %d\n", ret);
		goto dis_tdm_clk;
	}
	return 0;

dis_tdm_clk:
	clk_disable_unprepare(tdm->clk_tdm);
dis_tdm_ext_clk:
	clk_disable_unprepare(tdm->clk_tdm_ext);
dis_tdm_internal_clk:
	clk_disable_unprepare(tdm->clk_tdm_internal);
dis_tdm_apb_clk:
	clk_disable_unprepare(tdm->clk_tdm_apb);
dis_tdm_ahb_clk:
	clk_disable_unprepare(tdm->clk_tdm_ahb);
dis_mclk_inner_clk:
	clk_disable_unprepare(tdm->clk_mclk_inner);

	return ret;
}

#ifdef CONFIG_PM
static int jh7110_tdm_runtime_suspend(struct device *dev)
{
	struct jh7110_tdm_dev *tdm = dev_get_drvdata(dev);

	jh7110_tdm_clk_disable(tdm);

	return 0;
}

static int jh7110_tdm_runtime_resume(struct device *dev)
{
	struct jh7110_tdm_dev *tdm = dev_get_drvdata(dev);

	return jh7110_tdm_clk_enable(tdm);
}
#endif

#ifdef CONFIG_PM_SLEEP
static int jh7110_tdm_suspend(struct snd_soc_component *component)
{
	return pm_runtime_force_suspend(component->dev);
}

static int jh7110_tdm_resume(struct snd_soc_component *component)
{
	struct jh7110_tdm_dev *tdm = snd_soc_component_get_drvdata(component);

	/* restore context */
	jh7110_tdm_writel(tdm, TDM_PCMGBCR, tdm->saved_pcmgbcr);
	jh7110_tdm_writel(tdm, TDM_PCMDIV, tdm->saved_pcmdiv);

	return pm_runtime_force_resume(component->dev);
}

#else
#define jh7110_tdm_suspend	NULL
#define jh7110_tdm_resume	NULL
#endif

static const struct snd_soc_component_driver jh7110_tdm_component = {
	.name = "jh7110-tdm",
	.suspend = jh7110_tdm_suspend,
	.resume	= jh7110_tdm_resume,
};

static int jh7110_tdm_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	struct jh7110_tdm_dev *tdm = snd_soc_dai_get_drvdata(dai);
	int chan_wl, chan_sl, chan_nr;
	unsigned int data_width;
	unsigned int dma_bus_width;
	struct snd_dmaengine_dai_dma_data *dma_data = NULL;
	struct snd_soc_pcm_runtime *rtd = asoc_substream_to_rtd(substream);
	struct snd_soc_dai_link *dai_link = rtd->dai_link;

	dai_link->stop_dma_first = 1;

	data_width = params_width(params);

	tdm->samplerate = params_rate(params);
	tdm->pcmclk = params_channels(params) * tdm->samplerate * data_width;

	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		chan_wl = TDM_16BIT_WORD_LEN;
		chan_sl = TDM_16BIT_SLOT_LEN;
		dma_bus_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
		break;

	case SNDRV_PCM_FORMAT_S32_LE:
		chan_wl = TDM_32BIT_WORD_LEN;
		chan_sl = TDM_32BIT_SLOT_LEN;
		dma_bus_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
		break;

	default:
		dev_err(tdm->dev, "tdm: unsupported PCM fmt");
		return -EINVAL;
	}

	chan_nr = params_channels(params);
	switch (chan_nr) {
	case ONE_CHANNEL_SUPPORT:
	case TWO_CHANNEL_SUPPORT:
	case FOUR_CHANNEL_SUPPORT:
	case SIX_CHANNEL_SUPPORT:
	case EIGHT_CHANNEL_SUPPORT:
		break;
	default:
		dev_err(tdm->dev, "channel not supported\n");
		return -EINVAL;
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		tdm->tx.wl = chan_wl;
		tdm->tx.sl = chan_sl;
		tdm->tx.sscale = chan_nr;
		tdm->play_dma_data.addr_width = dma_bus_width;
		dma_data = &tdm->play_dma_data;
	} else {
		tdm->rx.wl = chan_wl;
		tdm->rx.sl = chan_sl;
		tdm->rx.sscale = chan_nr;
		tdm->capture_dma_data.addr_width = dma_bus_width;
		dma_data = &tdm->capture_dma_data;
	}

	snd_soc_dai_set_dma_data(dai, substream, dma_data);

	jh7110_tdm_config(tdm, substream);
	jh7110_tdm_save_context(tdm, substream);

	return 0;
}

static int jh7110_tdm_trigger(struct snd_pcm_substream *substream,
			      int cmd, struct snd_soc_dai *dai)
{
	struct jh7110_tdm_dev *tdm = snd_soc_dai_get_drvdata(dai);
	int ret = 0;

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		/* restore context */
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
			jh7110_tdm_writel(tdm, TDM_PCMTXCR, tdm->saved_pcmtxcr);
		else
			jh7110_tdm_writel(tdm, TDM_PCMRXCR, tdm->saved_pcmrxcr);

		jh7110_tdm_start(tdm, substream);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		jh7110_tdm_stop(tdm, substream);
		break;
	default:
		ret = -EINVAL;
		break;
	}
	return ret;
}

static const struct snd_soc_dai_ops jh7110_tdm_dai_ops = {
	.hw_params = jh7110_tdm_hw_params,
	.trigger = jh7110_tdm_trigger,
};

static int jh7110_tdm_dai_probe(struct snd_soc_dai *dai)
{
	struct jh7110_tdm_dev *tdm = snd_soc_dai_get_drvdata(dai);

	snd_soc_dai_init_dma_data(dai, &tdm->play_dma_data, &tdm->capture_dma_data);
	snd_soc_dai_set_drvdata(dai, tdm);
	return 0;
}

#define JH7110_TDM_RATES	SNDRV_PCM_RATE_8000_48000

#define JH7110_TDM_FORMATS	(SNDRV_PCM_FMTBIT_S16_LE | \
				 SNDRV_PCM_FMTBIT_S32_LE)

static struct snd_soc_dai_driver jh7110_tdm_dai = {
	.name = "sf_tdm",
	.id = 0,
	.playback = {
		.stream_name    = "Playback",
		.channels_min   = 1,
		.channels_max   = 8,
		.rates          = JH7110_TDM_RATES,
		.formats        = JH7110_TDM_FORMATS,
	},
	.capture = {
		.stream_name    = "Capture",
		.channels_min   = 1,
		.channels_max   = 8,
		.rates          = JH7110_TDM_RATES,
		.formats        = JH7110_TDM_FORMATS,
	},
	.ops = &jh7110_tdm_dai_ops,
	.probe = jh7110_tdm_dai_probe,
	.symmetric_rate = 1,
};

static const struct snd_pcm_hardware jh7110_pcm_hardware = {
	.info			= (SNDRV_PCM_INFO_MMAP		|
				   SNDRV_PCM_INFO_MMAP_VALID	|
				   SNDRV_PCM_INFO_PAUSE		|
				   SNDRV_PCM_INFO_RESUME	|
				   SNDRV_PCM_INFO_INTERLEAVED	|
				   SNDRV_PCM_INFO_BLOCK_TRANSFER),
	.buffer_bytes_max	= 192512,
	.period_bytes_min	= 4096,
	.period_bytes_max	= 32768,
	.periods_min		= 1,
	.periods_max		= 48,
	.fifo_size		= 16,
};

static const struct snd_dmaengine_pcm_config jh7110_dmaengine_pcm_config = {
	.pcm_hardware = &jh7110_pcm_hardware,
	.prepare_slave_config = snd_dmaengine_pcm_prepare_slave_config,
	.prealloc_buffer_size = 192512,
};

static void jh7110_tdm_init_params(struct jh7110_tdm_dev *tdm)
{
	tdm->clkpolity = TDM_TX_RASING_RX_FALLING;
	if (tdm->frame_mode == SHORT_LATER) {
		tdm->elm = TDM_ELM_LATE;
		tdm->syncm = TDM_SYNCM_SHORT;
	} else if (tdm->frame_mode == SHORT_EARLY) {
		tdm->elm = TDM_ELM_EARLY;
		tdm->syncm = TDM_SYNCM_SHORT;
	} else {
		tdm->elm = TDM_ELM_EARLY;
		tdm->syncm = TDM_SYNCM_LONG;
	}

	tdm->ms_mode = TDM_AS_SLAVE;
	tdm->rx.ifl = TDM_FIFO_HALF;
	tdm->tx.ifl = TDM_FIFO_HALF;
	tdm->rx.wl = TDM_16BIT_WORD_LEN;
	tdm->tx.wl = TDM_16BIT_WORD_LEN;
	tdm->rx.sscale = 2;
	tdm->tx.sscale = 2;
	tdm->rx.lrj = TDM_LEFT_JUSTIFT;
	tdm->tx.lrj = TDM_LEFT_JUSTIFT;

	tdm->play_dma_data.addr = TDM_FIFO;
	tdm->play_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	tdm->play_dma_data.fifo_size = TDM_FIFO_DEPTH / 2;
	tdm->play_dma_data.maxburst = 16;

	tdm->capture_dma_data.addr = TDM_FIFO;
	tdm->capture_dma_data.addr_width = DMA_SLAVE_BUSWIDTH_2_BYTES;
	tdm->capture_dma_data.fifo_size = TDM_FIFO_DEPTH / 2;
	tdm->capture_dma_data.maxburst = 8;
}

static int jh7110_tdm_clk_reset_init(struct platform_device *pdev,
				     struct jh7110_tdm_dev *tdm)
{
	int ret;

	static struct clk_bulk_data clks[] = {
		{ .id = "tdm_ahb" },
		{ .id = "tdm_apb" },
		{ .id = "tdm_internal" },
		{ .id = "tdm" },
		{ .id = "mclk_inner" },
		{ .id = "tdm_ext" },
	};

	ret = devm_clk_bulk_get(&pdev->dev, ARRAY_SIZE(clks), clks);
	if (ret) {
		dev_err(&pdev->dev, "Failed to get tdm clocks\n");
		return ret;
	}

	tdm->clk_tdm_ahb = clks[0].clk;
	tdm->clk_tdm_apb = clks[1].clk;
	tdm->clk_tdm_internal = clks[2].clk;
	tdm->clk_tdm = clks[3].clk;
	tdm->clk_mclk_inner = clks[4].clk;
	tdm->clk_tdm_ext = clks[5].clk;

	tdm->resets = devm_reset_control_array_get_exclusive(&pdev->dev);
	if (IS_ERR(tdm->resets)) {
		ret = PTR_ERR(tdm->resets);
		dev_err(&pdev->dev, "Failed to get tdm resets");
		return ret;
	}

	return jh7110_tdm_clk_enable(tdm);
}

static int jh7110_tdm_probe(struct platform_device *pdev)
{
	struct jh7110_tdm_dev *tdm;
	int ret;

	tdm = devm_kzalloc(&pdev->dev, sizeof(*tdm), GFP_KERNEL);
	if (!tdm)
		return -ENOMEM;

	tdm->tdm_base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(tdm->tdm_base))
		return PTR_ERR(tdm->tdm_base);

	tdm->dev = &pdev->dev;

	ret = jh7110_tdm_clk_reset_init(pdev, tdm);
	if (ret) {
		dev_err(&pdev->dev, "Failed to enable audio-tdm clock\n");
		return ret;
	}

	tdm->frame_mode = SHORT_LATER;
	jh7110_tdm_init_params(tdm);

	dev_set_drvdata(&pdev->dev, tdm);
	ret = devm_snd_soc_register_component(&pdev->dev, &jh7110_tdm_component,
					      &jh7110_tdm_dai, 1);
	if (ret != 0) {
		dev_err(&pdev->dev, "Failed to register dai\n");
		return ret;
	}

	ret = devm_snd_dmaengine_pcm_register(&pdev->dev,
					      &jh7110_dmaengine_pcm_config,
					      SND_DMAENGINE_PCM_FLAG_COMPAT);
	if (ret) {
		dev_err(&pdev->dev, "Could not register pcm: %d\n", ret);
		return ret;
	}

	pm_runtime_enable(&pdev->dev);
#ifdef CONFIG_PM
	jh7110_tdm_clk_disable(tdm);
#endif

	return 0;
}

static int jh7110_tdm_dev_remove(struct platform_device *pdev)
{
	pm_runtime_disable(&pdev->dev);
	return 0;
}

static const struct of_device_id jh7110_tdm_of_match[] = {
	{.compatible = "starfive,jh7110-tdm",},
	{}
};

MODULE_DEVICE_TABLE(of, jh7110_tdm_of_match);

static const struct dev_pm_ops jh7110_tdm_pm_ops = {
	SET_RUNTIME_PM_OPS(jh7110_tdm_runtime_suspend,
			   jh7110_tdm_runtime_resume, NULL)
};

static struct platform_driver jh7110_tdm_driver = {
	.driver = {
		.name = "jh7110-tdm",
		.of_match_table = jh7110_tdm_of_match,
		.pm = &jh7110_tdm_pm_ops,
	},
	.probe = jh7110_tdm_probe,
	.remove = jh7110_tdm_dev_remove,
};
module_platform_driver(jh7110_tdm_driver);

MODULE_AUTHOR("Walker Chen <walker.chen@starfivetech.com>");
MODULE_DESCRIPTION("StarFive JH7110 TDM ASoC Driver");
MODULE_LICENSE("GPL");
