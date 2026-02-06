# SPL启动流程分析
跟踪一下 u-boot 2011.09 MLO在 nandflash 下运行的流程
首先，直接进入 start.S
```
    // arch/arm/cpu/armv7/start.S
     36 .globl _start
     37 _start: b   reset
    // ... ...

   136 reset:
    137     bl  save_boot_params

   // ... ...
    206 call_board_init_f:
    207     ldr sp, =(CONFIG_SYS_INIT_SP_ADDR)
    208     bic sp, sp, #7 /* 8-byte alignment for ABI compliance */
    209     ldr r0,=0x00000000
    210     bl  board_init_f
```
SPL 的 board_init_f 在 arch/arm/cpu/armv7/omap-common/spl.c 里面
```
   // arch/arm/cpu/armv7/omap-common/spl.c
     57 void board_init_f(ulong dummy)
     58 {
     59     /*
     60      * We call relocate_code() with relocation target same as the
     61      * CONFIG_SYS_SPL_TEXT_BASE. This will result in relocation getting
     62      * skipped. Instead, only .bss initialization will happen. That's
     63      * all we need
     64      */
     65     debug(">>board_init_f()\n");
     66     relocate_code(CONFIG_SPL_STACK, &gdata, CONFIG_SPL_TEXT_BASE);
              // 这里又跳回  start.S
     67 }
```

   回到 start.S
  arch/arm/cpu/armv7/start.S
```
    221     .globl  relocate_code    // 全局函数
    222 relocate_code:
    223     mov r4, r0  /* save addr_sp */
    224     mov r5, r1  /* save addr of gd */
    225     mov r6, r2  /* save addr of destination */

   // ... 上面是将 代码拷贝到 memory （RAM）上的过程
    336 _board_init_r_ofs:
    337     .word board_init_r - _start
    // 下面就跳到 SPL 的 board_init_r
```


SPL 的board_init_r 是在 arch/arm/cpu/armv7/omap-common/spl.c 里面
arch/arm/cpu/armv7/omap-common/spl.c
```
    109 void board_init_r(gd_t *id, ulong dummy)
    110 {
    111     u32 boot_device;
    112     debug(">>spl:board_init_r()\n");
    113 
    114     timer_init();
    115     i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
    116 
    117 #ifdef CONFIG_SPL_BOARD_INIT
    118     spl_board_init();   // 启动了 I2C， 并使用I2C控制 PMIC
    119 #endif
    120 
    121     boot_device = omap_boot_device(); // 选择启动设备，是nand还是mmc
    122 
    123     switch (boot_device) {
    124 #ifdef CONFIG_SPL_MMC_SUPPORT
    125     case BOOT_DEVICE_MMC1:
    126     case BOOT_DEVICE_MMC2:
    127         printk("Booting from MMC...\n");
    128         spl_mmc_load_image();
    129         break;
    130 #endif
    131 #ifdef CONFIG_SPL_NAND_SUPPORT
    132     case BOOT_DEVICE_NAND:
    133         printk("Booting from NAND...\n");
    134         spl_nand_load_image();
    135         break;
```
arch/arm/cpu/armv7/omap-common/spl_nand.c
```
     33 void spl_nand_load_image(void)
     34 {
     35     struct image_header *header;
     36     switch (omap_boot_mode()) {
     37     case NAND_MODE_HW_ECC:
     38         debug("spl: nand - using hw ecc\n");
     39         gpmc_init();
     40         nand_init();
     41         break;
     42     default:
     43         puts("spl: ERROR: This bootmode is not implemented - hanging");
     44         hang();
     45     }
        // …
     65     nand_spl_load_image(CONFIG_SYS_NAND_U_BOOT_OFFS,
     66         CONFIG_SYS_NAND_PAGE_SIZE, (void *)header);
     67     spl_parse_image_header(header);
               // 实现完了 spl 加载到 u-boot.img 
     68     nand_spl_load_image(CONFIG_SYS_NAND_U_BOOT_OFFS,
     69         spl_image.size, (void *)spl_image.load_addr);
     70     nand_deselect();
     71 }
```
nand_init( 的实现在 drivers/mtd/nand/nand_spl_simple.c 里面
drivers/mtd/nand/nand_spl_simple.c
```
    224 /* nand_init() - initialize data to make nand usable by SPL */
    225 void nand_init(void)
    226 {       
    227     /*  
    228      * Init board specific nand support
    229      */
    230     mtd.priv = &nand_chip;
    231     nand_chip.IO_ADDR_R = nand_chip.IO_ADDR_W =
    232         (void  __iomem *)CONFIG_SYS_NAND_BASE;
    233     nand_chip.options = 0;
    234     board_nand_init(&nand_chip);
    235         
    236     if (nand_chip.select_chip)
    237         nand_chip.select_chip(&mtd, 0);
    238 }
```
board_nand_init 的实现在 drivers/mtd/nand/ti81xx_nand.c
drivers/mtd/nand/ti81xx_nand.c
```
     924 int board_nand_init(struct nand_chip *nand)
     925 {
     926     /* int32_t gpmc_config = 0; */
     927     cs = 0;
     928 
     929     /*
     930      * xloader/Uboot's gpmc configuration would have configured GPMC for
     931      * nand type of memory. The following logic scans and latches on to the
     932      * first CS with NAND type memory.
     933      * TBD: need to make this logic generic to handle multiple CS NAND
     934      * devices.
     935      */
     936     while (cs < GPMC_MAX_CS) {
     937         /* Check if NAND type is set */
     938         if ((readl(&gpmc_cfg->cs[cs].config1) & 0xC00) == 0x800) {
     939             /* Found it!! */
     940 #ifdef NAND_DEBUG
     941             printf("Searching for NAND device @ GPMC CS:%1d\n", cs);
     942 #endif
     943             break;
     944         }
     945         cs++;
     946     }
     947     if (cs >= GPMC_MAX_CS) {
     948         printf("NAND: Unable to find NAND settings in "
     949             "GPMC Configuration - quitting\n");
     950         return -ENODEV;
     951     }
     952 
     953     nand->IO_ADDR_R = (void __iomem *)&gpmc_cfg->cs[cs].nand_dat;
     954     nand->IO_ADDR_W = (void __iomem *)&gpmc_cfg->cs[cs].nand_cmd;
     955 
     956     nand->cmd_ctrl = ti81xx_nand_hwcontrol;
     957     nand->options = NAND_NO_PADDING | NAND_CACHEPRG | NAND_NO_AUTOINCR;
     958     /* If we are 16 bit dev, our gpmc config tells us that */
     959     if ((readl(&gpmc_cfg->cs[cs].config1) & 0x3000) == 0x1000) {
     960         nand->options |= NAND_BUSWIDTH_16;
     961     }
     962 
     963     nand->chip_delay = 100;
     964 
     965     /* required in case of BCH */
     966     elm_init();
     967 
     968     /* BCH info that will be correct for SPL or overridden otherwise. */
     969     nand->priv = &bch_priv;
     970
     971 #ifndef CONFIG_SPL_BUILD
     972     /* For undocumented reasons we need to currently keep our environment
     973      * in 1-bit ECC so we configure ourself thusly. */
     974     nand_curr_device = 0;
     975     ti81xx_nand_switch_ecc(NAND_ECC_HW, 0);
     976 #else
     977     /* The NAND chip present requires that we have written data in with
     978      * at least 4-bit ECC so we configure outself for that in SPL.
     979      */
     980     nand->ecc.mode = NAND_ECC_HW_SYNDROME;
     981     /* nand->ecc.layout = &hw_bch8_nand_oob; */
     982     nand->ecc.layout = &nand_ecclayout;
     983     nand->ecc.size = CONFIG_SYS_NAND_ECCSIZE;
     984     nand->ecc.bytes = CONFIG_SYS_NAND_ECCBYTES;
     985     nand->ecc.steps = CONFIG_SYS_NAND_ECCSTEPS;
     986     nand->ecc.total = CONFIG_SYS_NAND_ECCTOTAL;
     987     nand->ecc.hwctl = ti81xx_enable_ecc_bch;
     988     nand->ecc.correct = ti81xx_correct_data_bch;
     989     nand->ecc.calculate = ti81xx_calculate_ecc_bch;
     990 
     991     if (nand->options & NAND_BUSWIDTH_16)
     992         nand->read_buf = nand_read_buf16;
     993     else
     994         nand->read_buf = nand_read_buf;
     995     nand->dev_ready = ti81xx_spl_dev_ready;
     996 
     997     ti81xx_hwecc_init_bch(nand, NAND_ECC_READ);
     998 #endif
     999 
    1000     return 0;
    1001 }
```

