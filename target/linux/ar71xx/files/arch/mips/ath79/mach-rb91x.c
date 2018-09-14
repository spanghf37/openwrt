#include "pci.h"
#include "routerboot.h"
#define RB91X_FLAG_USB BIT(0)
#define RB91X_FLAG_PCIE BIT(1)
@@ -94,26 +87,37 @@ struct rb_board_info {
static struct mtd_partition rb711gr100_spi_partitions[] = {
{
.name = "routerboot",
.offset = 0,
.size = 0x0e000,
.mask_flags = MTD_WRITEABLE,
}, {
.name = "hard_config",
.offset = 0x0e000,
.size = 0x01000,
.mask_flags = MTD_WRITEABLE,
}, {
.name = "soft_config",
.offset = 0x0f000,
.size = 0x01000,
}, {
.name = "kernel",
.offset = 0x020000,
.size = 0x180000,
}, {
.name = "rootfs",
.offset = 0x1a0000,
.size = 0x180000,
}, {
.name = "ubifs",
.offset = 0x320000,
.size = MTDPART_SIZ_FULL
},
};
static struct flash_platform_data
rb711gr100_spi_flash_data = {
.type = "m25p05",
.parts = rb711gr100_spi_partitions,
.nr_parts = 3,
};
static int
rb711gr100_gpio_latch_gpios[AR934X_GPIO_COUNT]
__initdata = {
@@ -223,12 +227,25 @@ static struct gpio_led
rb711gr100_leds[] __initdata = {
},
};
static struct at803x_platform_data rb91x_at803x_data = {
 .disable_smarteee = 1,
 .enable_rgmii_rx_delay = 1,
 .enable_rgmii_tx_delay = 1,
};

static struct mdio_board_info rb91x_mdio0_info[] = {
 {
 .bus_id = "ag71xx-mdio.0",
 .phy_addr = 0,
 .platform_data = &rb91x_at803x_data,
 },
};

static void __init rb711gr100_init_partitions(const struct rb_info *info)
{
rb711gr100_spi_partitions[0].size = info->hard_cfg_offs;
rb711gr100_spi_partitions[1].offset = info->hard_cfg_offs;
rb711gr100_spi_partitions[2].offset = info->soft_cfg_offs;
}
void __init rb711gr100_wlan_init(void)
@@ -293,10 +310,14 @@ static void __init
rb711gr100_setup(void)
ARRAY_SIZE(rb711gr100_spi_info));
ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 | AR934X_ETH_CFG_RXD_DELAY | AR934X_ETH_CFG_SW_ONLY_MODE);
ath79_register_mdio(0, 0x0);
mdiobus_register_board_info(rb91x_mdio0_info, ARRAY_SIZE(rb91x_mdio0_info));
ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
ath79_eth0_data.phy_mask = BIT(0);
@@ -328,3 +349,69 @@ static void __init
rb711gr100_setup(void)
}
MIPS_MACHINE_NONAME(ATH79_MACH_RB_711GR100, "711Gr100", rb711gr100_setup);

static struct gpio_led rbgroove_leds_gpio[] __initdata = {
 {
 .name = "rbgroove:wlan:1",
 .gpio = 0,
 .active_low = 0,
 }, {
 .name = "rbgroove:wlan:2",
 .gpio = 1,
 .active_low = 0,
 }, {
 .name = "rbgroove:wlan:3",
 .gpio = 2,
 .active_low = 0,
 }
};

static int nand_disabled;

static int __init no_nand(char *str)
{
 nand_disabled = 1;
 return 0;
}

early_param("no-nand", no_nand);

static void __init rbgroove_setup(void)
{
 const struct rb_info *info;

 info = rb_init_info((void *) KSEG1ADDR(0x1f000000),0x20000);
 if (!info)
 return;

 rb711gr100_init_partitions(info);
 if (nand_disabled) {
 printk(KERN_NOTICE "using SPI flash for root\n");
 rb711gr100_spi_flash_data.nr_parts = ARRAY_SIZE(rb711gr100_spi_partitions);
 }
 ath79_register_spi(&rb711gr100_spi_data, rb711gr100_spi_info, 1);

ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_MII_GMAC0);

 ath79_register_mdio(0, 0x0);

 ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
 ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
 ath79_eth0_data.phy_mask = BIT(0);

 ath79_register_eth(0);
 rb711gr100_wlan_init();

 if (!nand_disabled) {
 printk(KERN_NOTICE "using NAND flash for root\n");
 platform_device_register_data(NULL, "rb91xnand", -1, &rb711gr100_nand_data, sizeof(rb711gr100_nand_data));
 }

 ath79_register_leds_gpio(-1, ARRAY_SIZE(rbgroove_leds_gpio), + rbgroove_leds_gpio);
}

MIPS_MACHINE(ATH79_MACH_RB_GROOVE, "groove-52", "MikroTik 52HPn", rbgroove_setup);
