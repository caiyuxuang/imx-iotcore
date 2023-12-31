global-incdirs-y += .
srcs-y += main.c imx-common.c

srcs-$(CFG_MX6)$(CFG_MX7) += imx_src.c mmdc.c

srcs-$(CFG_PL310) += imx_pl310.c
ifeq ($(CFG_PSCI_ARM32),y)
$(call force,CFG_PM_ARM32,y)
asm-defines-y += imx_pm_asm_defines.c
endif

ifneq (,$(filter y, $(CFG_MX6Q) $(CFG_MX6QP) $(CFG_MX6D) $(CFG_MX6DL) \
	$(CFG_MX6S) $(CFG_MX6SL) $(CFG_MX6SLL) $(CFG_MX6SX)))
srcs-y += a9_plat_init.S
endif

ifneq (,$(filter y, $(CFG_MX7) $(CFG_MX7ULP) $(CFG_MX6UL) $(CFG_MX6ULL)))
srcs-y += a7_plat_init.S
endif

srcs-$(CFG_SM_PLATFORM_HANDLER) += sm_platform_handler.c
subdirs-y += pm
subdirs-y += drivers
srcs-$(CFG_IMX_OCRAM) += imx_ocram.c
srcs-$(CFG_DT) += imx_dt.c
ifeq (,$(filter $(PLATFORM_FLAVOR),$(mx8qx-flavorlist)))
srcs-$(CFG_RPMB_FS) += imx_rpmb.c
endif

# Build the busfreq module
subdirs-$(CFG_BUSFREQ) += busfreq

# Build PTA
subdirs-y += pta
