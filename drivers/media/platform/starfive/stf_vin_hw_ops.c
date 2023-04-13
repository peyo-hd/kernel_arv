// SPDX-License-Identifier: GPL-2.0
/*
 * stf_vin_hw_ops.c
 *
 * Register interface file for StarFive VIN module driver
 *
 * Copyright (C) 2021-2023 StarFive Technology Co., Ltd.
 */
#include "stf_camss.h"

static void vin_intr_clear(struct stfcamss *stfcamss)
{
	stf_syscon_reg_set_bit(stfcamss, SYSCONSAIF_SYSCFG(28),
			       U0_VIN_CNFG_AXIWR0_INTR_CLEAN, 0x1);
	stf_syscon_reg_set_bit(stfcamss, SYSCONSAIF_SYSCFG(28),
			       U0_VIN_CNFG_AXIWR0_INTR_CLEAN, 0x0);
}

irqreturn_t stf_vin_wr_irq_handler(int irq, void *priv)
{
	struct stf_vin_dev *vin_dev = priv;
	struct stfcamss *stfcamss = vin_dev->stfcamss;
	struct dummy_buffer *dummy_buffer =
			&vin_dev->dummy_buffer[STF_DUMMY_VIN];

	if (atomic_dec_if_positive(&dummy_buffer->frame_skip) < 0) {
		vin_dev->isr_ops->isr_change_buffer(&vin_dev->line[VIN_LINE_WR]);
		vin_dev->isr_ops->isr_buffer_done(&vin_dev->line[VIN_LINE_WR]);
	}

	vin_intr_clear(stfcamss);

	return IRQ_HANDLED;
}

irqreturn_t stf_vin_isp_irq_handler(int irq, void *priv)
{
	struct stf_vin_dev *vin_dev = priv;
	u32 int_status;

	int_status = stf_isp_reg_read(vin_dev->stfcamss, ISP_REG_ISP_CTRL_0);

	if (int_status & BIT(24)) {
		if ((int_status & BIT(20)))
			vin_dev->isr_ops->isr_buffer_done(
				&vin_dev->line[VIN_LINE_ISP]);

		/* clear interrupt */
		stf_isp_reg_write(vin_dev->stfcamss,
				  ISP_REG_ISP_CTRL_0,
				  (int_status & ~EN_INT_ALL) |
				  EN_INT_ISP_DONE |
				  EN_INT_CSI_DONE |
				  EN_INT_SC_DONE);
	}

	return IRQ_HANDLED;
}

irqreturn_t stf_vin_isp_irq_csiline_handler(int irq, void *priv)
{
	struct stf_vin_dev *vin_dev = priv;
	struct stf_isp_dev *isp_dev;
	u32 int_status;

	isp_dev = &vin_dev->stfcamss->isp_dev;

	int_status = stf_isp_reg_read(vin_dev->stfcamss, ISP_REG_ISP_CTRL_0);
	if (int_status & BIT(27)) {
		struct dummy_buffer *dummy_buffer =
			&vin_dev->dummy_buffer[STF_DUMMY_ISP];

		if (!atomic_read(&isp_dev->shadow_count)) {
			if (atomic_dec_if_positive(&dummy_buffer->frame_skip) < 0) {
				if ((int_status & BIT(20)))
					vin_dev->isr_ops->isr_change_buffer(
						&vin_dev->line[VIN_LINE_ISP]);
			}

			/* shadow update */
			stf_isp_reg_set_bit(isp_dev->stfcamss,
					    ISP_REG_CSIINTS_ADDR,
					    CSI_INTS_MASK, CSI_INTS(0x3));
			stf_isp_reg_set_bit(isp_dev->stfcamss,
					    ISP_REG_IESHD_ADDR,
					    SHAD_UP_M | SHAD_UP_EN, 0x3);
		}

		/* clear interrupt */
		stf_isp_reg_write(vin_dev->stfcamss, ISP_REG_ISP_CTRL_0,
				  (int_status & ~EN_INT_ALL) | EN_INT_LINE_INT);
	}

	return IRQ_HANDLED;
}

int stf_vin_clk_enable(struct stf_vin_dev *vin_dev, enum link link)
{
	struct stfcamss *stfcamss = vin_dev->stfcamss;

	clk_set_rate(stfcamss->sys_clk[STF_CLK_APB_FUNC].clk, 49500000);

	switch (link) {
	case LINK_DVP_TO_WR:
		break;
	case LINK_DVP_TO_ISP:
		break;
	case LINK_CSI_TO_WR:
		break;
	case LINK_CSI_TO_ISP:
		clk_set_rate(stfcamss->sys_clk[STF_CLK_MIPI_RX0_PXL].clk,
			     198000000);
		clk_set_parent(stfcamss->sys_clk[STF_CLK_WRAPPER_CLK_C].clk,
			       stfcamss->sys_clk[STF_CLK_MIPI_RX0_PXL].clk);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int stf_vin_wr_stream_set(struct stf_vin_dev *vin_dev, int on)
{
	struct stfcamss *stfcamss = vin_dev->stfcamss;

	/* make the axiwr alway on */
	if (on)
		stf_syscon_reg_set(stfcamss, SYSCONSAIF_SYSCFG(20),
				   U0_VIN_CNFG_AXIWR0_EN);

	return 0;
}

int stf_vin_stream_set(struct stf_vin_dev *vin_dev, int on, enum link link)
{
	struct stfcamss *stfcamss = vin_dev->stfcamss;

	switch (link) {
	case LINK_DVP_TO_WR:
		break;
	case LINK_DVP_TO_ISP:
		break;
	case LINK_CSI_TO_WR:
		break;
	case LINK_CSI_TO_ISP:
		stf_syscon_reg_set_bit(stfcamss, SYSCONSAIF_SYSCFG(36),
				       U0_VIN_CNFG_MIPI_BYTE_EN_ISP0_MASK,
				       U0_VIN_CNFG_MIPI_BYTE_EN_ISP0(0));
		stf_syscon_reg_set_bit(stfcamss, SYSCONSAIF_SYSCFG(36),
				       U0_VIN_CNFG_MIPI_CHANNEL_SEL0_MASK,
				       U0_VIN_CNFG_MIPI_CHANNEL_SEL0(0));
		stf_syscon_reg_set_bit(stfcamss, SYSCONSAIF_SYSCFG(36),
				       U0_VIN_CNFG_PIX_NUM_MASK,
				       U0_VIN_CNFG_PIX_NUM(0));
		stf_syscon_reg_set_bit(stfcamss, SYSCONSAIF_SYSCFG(36),
				       U0_VIN_CNFG_P_I_MIPI_HAEDER_EN0_MASK,
				       U0_VIN_CNFG_P_I_MIPI_HAEDER_EN0(1));
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

void stf_vin_wr_irq_enable(struct stf_vin_dev *vin_dev, int enable)
{
	struct stfcamss *stfcamss = vin_dev->stfcamss;
	unsigned int value = 0;

	if (enable) {
		value = ~(0x1 << 1);
		stf_syscon_reg_set_bit(stfcamss, SYSCONSAIF_SYSCFG(28),
				       U0_VIN_CNFG_AXIWR0_INTR_MASK, value);
	} else {
		/* clear vin interrupt */
		value = 0x1 << 1;
		stf_syscon_reg_set_bit(stfcamss, SYSCONSAIF_SYSCFG(28),
				       U0_VIN_CNFG_AXIWR0_INTR_CLEAN, 0x1);
		stf_syscon_reg_set_bit(stfcamss, SYSCONSAIF_SYSCFG(28),
				       U0_VIN_CNFG_AXIWR0_INTR_CLEAN, 0x0);
		stf_syscon_reg_set_bit(stfcamss, SYSCONSAIF_SYSCFG(28),
				       U0_VIN_CNFG_AXIWR0_INTR_MASK, value);
	}
}

void stf_vin_wr_set_ping_addr(struct stf_vin_dev *vin_dev, dma_addr_t addr)
{
	struct stfcamss *stfcamss = vin_dev->stfcamss;

	/* set the start address */
	stf_syscon_reg_write(stfcamss, SYSCONSAIF_SYSCFG(32), (long)addr);
}

void stf_vin_wr_set_pong_addr(struct stf_vin_dev *vin_dev, dma_addr_t addr)
{
	struct stfcamss *stfcamss = vin_dev->stfcamss;

	/* set the start address */
	stf_syscon_reg_write(stfcamss, SYSCONSAIF_SYSCFG(24), (long)addr);
}

void stf_vin_isp_set_yuv_addr(struct stf_vin_dev *vin_dev,
			      dma_addr_t y_addr, dma_addr_t uv_addr)
{
	stf_isp_reg_write(vin_dev->stfcamss,
			  ISP_REG_Y_PLANE_START_ADDR, y_addr);
	stf_isp_reg_write(vin_dev->stfcamss,
			  ISP_REG_UV_PLANE_START_ADDR, uv_addr);
}
