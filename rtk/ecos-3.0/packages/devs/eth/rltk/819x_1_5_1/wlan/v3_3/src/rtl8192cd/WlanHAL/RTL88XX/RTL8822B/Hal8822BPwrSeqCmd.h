#ifndef REALTEK_POWER_SEQUENCE_8822B
#define REALTEK_POWER_SEQUENCE_8822B


/* 
	Check document WB-110628-DZ-RTL8195 (Jaguar) Power Architecture-R04.pdf
	There are 6 HW Power States:
	0: POFF--Power Off
	1: PDN--Power Down
	2: CARDEMU--Card Emulation
	3: ACT--Active Mode
	4: LPS--Low Power State
	5: SUS--Suspend

	The transision from different states are defined below
	TRANS_CARDEMU_TO_ACT
	TRANS_ACT_TO_CARDEMU
	TRANS_CARDEMU_TO_SUS
	TRANS_SUS_TO_CARDEMU
	TRANS_CARDEMU_TO_PDN
	TRANS_ACT_TO_LPS
	TRANS_LPS_TO_ACT	

	TRANS_END
*/
#define	RTL8822B_TRANS_CARDEMU_TO_ACT_STEPS	16
#define	RTL8822B_TRANS_ACT_TO_CARDEMU_STEPS	20
#define	RTL8822B_TRANS_CARDEMU_TO_SUS_STEPS	17
#define	RTL8822B_TRANS_SUS_TO_CARDEMU_STEPS	15
#define	RTL8822B_TRANS_CARDEMU_TO_PDN_STEPS	17
#define	RTL8822B_TRANS_PDN_TO_CARDEMU_STEPS	15
#define	RTL8822B_TRANS_ACT_TO_LPS_STEPS	20
#define	RTL8822B_TRANS_LPS_TO_ACT_STEPS	15	
#define	RTL8822B_TRANS_END_STEPS	1


#define RTL8822B_TRANS_CARDEMU_TO_ACT 														\
	/* format */																\
	/* { offset, cut_msk, fab_msk|interface_msk, base|cmd, msk, value }, // comments here*/								\
	{0x0005, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT2, 0},/* disable SW LPS 0x04[10]=0*/	\
	{0x0006, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_POLLING, BIT1, BIT1},/* wait till 0x04[17] = 1    power ready*/	\
	{0x002B, PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT0, BIT0}, /* ??0x28[24]=1, enable pll phase select*/ \
	{0x0015, PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, (BIT3|BIT2|BIT1), (BIT3|BIT2|BIT1)},/* 0x14[11:9]=3'b111,OCP current threshold=1.5A */ \
	{0x002D, PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0x0E, 0x08},/* 0x2C[11:9]=3'b100, select lpf R3 */ \
	{0x002D, PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0x70, 0x50},/* 0x2C[14:12]=3'b101, select lpf Rs*/ \
	{0x007B, PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT6, BIT6},/* 0x78[30]=1'b1, SDM order select*/ \
	/*{0x0005, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT7, 0}, */ /* disable HWPDN 0x04[15]=0*/ \
	{0x0005, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT3, 0},/* disable WL suspend*/	\
	{0x00F0, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT7, 0},/* */	\
	{0x0081, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0x30, 0x20},/* */	\
	{0x0005, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT0, BIT0},/* polling until return 0*/	\
	{0x0005, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_POLLING, BIT0, 0},/**/

#define RTL8822B_TRANS_ACT_TO_CARDEMU													\
	/* format */																\
	/* { offset, cut_msk, fab_msk|interface_msk, base|cmd, msk, value }, // comments here*/								\
	{0x0002, PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT0, 0},  /* 0x2[0] = 0	 RESET BB, CLOSE RF */	\
	{0x0002, PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_DELAY, 0, PWRSEQ_DELAY_US}, /*Delay 1us*/	\
	{0x0002, PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1, 0},  /* Whole BB is reset*/			\
	{0x1002, ~PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT0, 0},  /* 0x2[0] = 0	 RESET BB, CLOSE RF */	\
	{0x0002, ~PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_DELAY, 0, PWRSEQ_DELAY_US}, /*Delay 1us*/	\
	{0x1002, ~PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1, 0},  /* Whole BB is reset*/			\
	{0x001F, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0},  /*0x1F[7:0] = 0 turn off RF*/	\
	/*{0x004E, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT7, 0},*/  /*0x4C[23] = 0x4E[7] = 0, switch DPDT_SEL_P output from register 0x65[2] */	\
	{0x0007, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x28},   /* 0x07[7:0] = 0x28 sps pwm mode 0x2a for BT coex*/	\
	{0x0008, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0x02, 0},   /*0x8[1] = 0 ANA clk =500k */	\
	/*{0x0002, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT0|BIT1, 0},*/   /*  0x02[1:0] = 0	reset BB */	\
	{0x0066, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT7, 0},   /*0x66[7]=0, disable ckreq for gpio7 output SUS */	\
	{0x0041, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT4, 0},   /*0x41[4]=0, disable sic for gpio7 output SUS */	\
	{0x0042, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1, 0},   /*0x42[1]=0, disable ckout for gpio7 output SUS */	\
	{0x004e, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT5, BIT5},   /*0x4E[5]=1, disable LED2 for gpio7 output SUS */	\
	{0x0041, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT0, 0},   /*0x41[0]=0, disable uart for gpio7 output SUS */	\
	{0x0005, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1, BIT1}, /*0x04[9] = 1 turn off MAC by HW state machine*/	\
	{0x0005, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_POLLING, BIT1, 0}, /*wait till 0x04[9] = 0 polling until return 0 to disable*/	

#define RTL8822B_TRANS_CARDEMU_TO_SUS													\
	/* format */								\
	/* { offset, cut_msk, fab_msk|interface_msk, base|cmd, msk, value }, // comments here*/								\
	{0x0061, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0x0F, 0x0c},\
	{0x0061, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0x0F, 0x0E},\
	{0x0062, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0x0F, 0x07},/* gpio11 input mode, gpio10~8 output mode */	\
	{0x0045, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x00},/* gpio 0~7 output same value as input ?? */	\
	{0x0046, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0xff},/* gpio0~7 output mode */	\
	{0x0047, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0},/* 0x47[7:0] = 00 gpio mode */	\
	{0x0007, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0},/* suspend option all off */	\
	{0x0015, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT5, BIT5},/*0x14[13] = 1 turn on ZCD */	\
	{0x0015, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT6, BIT6},/* 0x14[14] =1 trun on ZCD */	\
	{0x0023, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT4, BIT4},/*0x23[4] = 1 hpon LDO sleep mode */	\
	{0x0008, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1, 0},/*0x8[1] = 0 ANA clk =500k */	\
	{0x0091, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xA0, 0xA0}, /* 0x91[7]=1 0x91[5]=1 , disable sps,ldo sleep mode */	\
	{0x0070, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT3, BIT3}, /* 0x70[3]=1 enable mainbias polling */	\
	{0x0005, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT3, BIT3}, /*0x04[11] = 1 enable WL suspend */

#define RTL8822B_TRANS_SUS_TO_CARDEMU													\
	/* format */								\
	/* { offset, cut_msk, fab_msk|interface_msk, base|cmd, msk, value }, // comments here*/								\
	{0x0005, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT3, 0}, /*0x04[11] = 0 enable WL suspend*/   \
	{0x0023, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0x10, 0},/*0x23[4] = 0 hpon LDO sleep mode leave */	\
	{0x0015, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT6, 0},/* 0x14[14] =0 trun off ZCD */	\
	{0x0015, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT5, 0},/*0x14[13] = 0 turn off ZCD */	\
	{0x0046, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x00},/* gpio0~7 input mode */	\
	{0x0062, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x00},/* gpio11 input mode, gpio10~8 input mode */

#define RTL8822B_TRANS_CARDEMU_TO_CARDDIS													\
	/* format */																\
	/* { offset, cut_msk, fab_msk|interface_msk, base|cmd, msk, value }, // comments here*/								\
	/**{0x0194, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT0, 0}, //0x194[0]=0 , disable 32K clock*/	\
	/**{0x0093, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x94}, //0x93=0x94 , 90[30] =0 enable 500k ANA clock .switch clock from 12M to 500K , 90 [26] =0 disable EEprom loader clock*/	\
	{0x0003, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT2, 0}, /*0x03[2] = 0, reset 8051*/	\
	{0x0080, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x05}, /*0x80=05h if reload fw, fill the default value of host_CPU handshake field*/	\
	/*{0x0042, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xF0, 0xcc},*/   \
	/*{0x0042, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xF0, 0xEC},*/   \
	/*{0x0043, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x07},*/  /* gpio11 input mode, gpio10~8 output mode */	\
	/*{0x0045, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x00}, gpio 0~7 output same value as input ?? */	\
	/*{0x0046, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0xff}, gpio0~7 output mode */	\
	/*{0x0047, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0}, 0x47[7:0] = 00 gpio mode */	\
	{0x0015, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT6, BIT6},/* 0x15[6] =1 trun on ZCD output */	\
	{0x0015, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT5, BIT5},/*0x15[5] = 1 turn on ZCD */	\
	{0x0012, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT6, 0},/*0x12[6] = 0 force PFM mode */	\
	{0x0023, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT4, BIT4},/*0x23[4] = 1 hpon LDO sleep mode */	\
	{0x0008, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1, 0},/*0x8[1] = 0 ANA clk =500k */	\
	{0x0007, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x20}, /*0x07=0x20 , SOP option to disable BG/MB*/	\
	{0x001f, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1, 0}, /* 0x01f[1]=0 , disable RFC_0  control  REG_RF_CTRL_8822B */	\
	{0x0020, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1, 0}, /* 0x020[1]=0 , disable RFC_1  control  REG_RF_CTRL_8822B */	\
	{0x0021, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1, 0}, /* 0x021[1]=0 , disable RFC_2  control  REG_RF_CTRL_8822B */	\
	{0x0076, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1, 0}, /* 0x076[1]=0 , disable RFC_3  control REG_OPT_CTRL_8822B +2 */	\
	{0x0091, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xA0, 0xA0}, /* 0x91[7]=1 0x91[5]=1 , disable sps,ldo sleep mode */	\
	{0x0070, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT3, BIT3}, /* 0x70[3]=1 enable mainbias polling */	\
	{0x0005, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT3, BIT3}, /*0x04[11] = 1 enable WL suspend*/	

#define RTL8822B_TRANS_CARDDIS_TO_CARDEMU													\
	/* format */																\
	/* { offset, cut_msk, fab_msk|interface_msk, base|cmd, msk, value }, // comments here*/                       \
	{0x0012, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT6, BIT6},/*0x12[6] = 1 force PWM mode */	\
	{0x0015, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT5, 0},/*0x15[5] = 0 turn off ZCD */	\
	{0x0015, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT6, 0},/* 0x15[6] =0 trun off ZCD output */	\
	{0x0023, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT4, 0},/*0x23[4] = 0 hpon LDO leave sleep mode */	\
	{0x0046, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x00},/* gpio0~7 input mode */	\
	{0x0062, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x00}, /* gpio11 input mode, gpio10~8 input mode */ \
	{0x0005, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT2, 0}, /*0x04[10] = 0, enable SW LPS PCIE only*/	\
	{0x0005, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT3, 0}, /*0x04[11] = 0, enable WL suspend*/	\
	/*{0x0003, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT2, BIT2},*/ /*0x03[2] = 1, enable 3081*/	\
	/*{0x0301, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0},*/ /*PCIe DMA start*/		\
	{0x0071, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT2, 0},/*0x70[10] = 0, CPHY_MBIAS_EN disable*/


#define RTL8822B_TRANS_CARDEMU_TO_PDN												\
	/* format */																\
	/* { offset, cut_msk, fab_msk|interface_msk, base|cmd, msk, value }, // comments here*/	\
	{0x0005, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT7, BIT7},/* 0x04[15] = 1*/

#define RTL8822B_TRANS_PDN_TO_CARDEMU												\
	/* format */																\
	/* { offset, cut_msk, fab_msk|interface_msk, base|cmd, msk, value }, // comments here*/	\
	{0x0005, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT7, 0},/* 0x04[15] = 0*/

#define RTL8822B_TRANS_ACT_TO_LPS														\
	/* format */																\
	/* { offset, cut_msk, fab_msk|interface_msk, base|cmd, msk, value }, // comments here*/								\
	{0x0301, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0xFF},/*PCIe DMA stop*/	\
	{0x0522, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x7F},/*Tx Pause*/		\
	{0x05F8, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_POLLING, 0xFF, 0},/*Should be zero if no packet is transmitting*/	\
	{0x05F9, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_POLLING, 0xFF, 0},/*Should be zero if no packet is transmitting*/	\
	{0x05FA, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_POLLING, 0xFF, 0},/*Should be zero if no packet is transmitting*/	\
	{0x05FB, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_POLLING, 0xFF, 0},/*Should be zero if no packet is transmitting*/	\
	{0x0c00, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x04}, /* 0xc00[7:0] = 4	turn off 3-wire */	\
	{0x0e00, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x04}, /* 0xe00[7:0] = 4	turn off 3-wire */	\
	{0x0002, PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT0, 0},/*CCK and OFDM are disabled,and clock are gated,and RF closed*/	\
	{0x0002, PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_DELAY, 0, PWRSEQ_DELAY_US},/*Delay 1us*/	\
	{0x0002, PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1, 0},  /* Whole BB is reset*/			\
	{0x1002, ~PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT0, 0},/*CCK and OFDM are disabled,and clock are gated,and RF closed*/	\
	{0x0002, ~PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_DELAY, 0, PWRSEQ_DELAY_US},/*Delay 1us*/	\
	{0x1002, ~PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1, 0},  /* Whole BB is reset*/			\
	{0x0100, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x03},/*Reset MAC TRX*/			\
	{0x0101, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1, 0},/*check if removed later*/		\
	{0x05F1, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT0, BIT0},/*Respond TxOK to scheduler*/	


#define RTL8822B_TRANS_LPS_TO_ACT															\
	/* format */																\
	/* { offset, cut_msk, fab_msk|interface_msk, base|cmd, msk, value }, // comments here*/	\
	{0x0080, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_SDIO_MSK,PWR_BASEADDR_SDIO,PWR_CMD_WRITE, 0xFF, 0x84}, /*SDIO RPWM*/	\
	{0xFE58, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_USB_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x84}, /*USB RPWM*/	\
	{0x0361, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_PCI_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0x84}, /*PCIe RPWM*/	\
	{0x0002, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_DELAY, 0, PWRSEQ_DELAY_MS}, /* Delay*/	\
	{0x0008, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT4, 0}, /*.	0x08[4] = 0		 switch TSF to 40M*/	\
	{0x0109, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_POLLING, BIT7, 0}, /* Polling 0x109[7]=0  TSF in 40M*/			\
	/*{0x0029, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT6|BIT7, 0}, */ /*.	??0x29[7:6] = 2b'00	 enable BB clock*/	\
	{0x0101, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1, BIT1}, /*.	0x101[1] = 1*/					\
	{0x0100, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0xFF}, /*.	0x100[7:0] = 0xFF	 enable WMAC TRX*/	\
	{0x0002, PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1|BIT0, BIT1|BIT0}, /*.	0x02[1:0] = 2b'11	 enable BB macro*/	\
	{0x1002, ~PWR_CUT_TESTCHIP_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, BIT1|BIT0, BIT1|BIT0}, /*.	0x1002[1:0] = 2b'11	 enable BB macro*/	\
	{0x0522, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,PWR_BASEADDR_MAC,PWR_CMD_WRITE, 0xFF, 0}, /*.	0x522 = 0*/
 
#define RTL8822B_TRANS_END																\
	/* format */																\
	/* { offset, cut_msk, fab_msk|interface_msk, base|cmd, msk, value }, // comments here*/		\
	{0xFFFF, PWR_CUT_ALL_MSK, PWR_FAB_ALL_MSK, PWR_INTF_ALL_MSK,0,PWR_CMD_END, 0, 0}, //


extern WLAN_PWR_CFG rtl8822B_power_on_flow[RTL8822B_TRANS_CARDEMU_TO_ACT_STEPS+RTL8822B_TRANS_END_STEPS];
extern WLAN_PWR_CFG rtl8822B_radio_off_flow[RTL8822B_TRANS_ACT_TO_CARDEMU_STEPS+RTL8822B_TRANS_END_STEPS];
extern WLAN_PWR_CFG rtl8822B_card_disable_flow[RTL8822B_TRANS_ACT_TO_CARDEMU_STEPS+RTL8822B_TRANS_CARDEMU_TO_PDN_STEPS+RTL8822B_TRANS_END_STEPS];
extern WLAN_PWR_CFG rtl8822B_card_enable_flow[RTL8822B_TRANS_ACT_TO_CARDEMU_STEPS+RTL8822B_TRANS_CARDEMU_TO_PDN_STEPS+RTL8822B_TRANS_END_STEPS];
extern WLAN_PWR_CFG rtl8822B_suspend_flow[RTL8822B_TRANS_ACT_TO_CARDEMU_STEPS+RTL8822B_TRANS_CARDEMU_TO_SUS_STEPS+RTL8822B_TRANS_END_STEPS];
extern WLAN_PWR_CFG rtl8822B_resume_flow[RTL8822B_TRANS_ACT_TO_CARDEMU_STEPS+RTL8822B_TRANS_CARDEMU_TO_SUS_STEPS+RTL8822B_TRANS_END_STEPS];
extern WLAN_PWR_CFG rtl8822B_hwpdn_flow[RTL8822B_TRANS_ACT_TO_CARDEMU_STEPS+RTL8822B_TRANS_CARDEMU_TO_PDN_STEPS+RTL8822B_TRANS_END_STEPS];
extern WLAN_PWR_CFG rtl8822B_enter_lps_flow[RTL8822B_TRANS_ACT_TO_LPS_STEPS+RTL8822B_TRANS_END_STEPS];
extern WLAN_PWR_CFG rtl8822B_leave_lps_flow[RTL8822B_TRANS_LPS_TO_ACT_STEPS+RTL8822B_TRANS_END_STEPS];

#endif

