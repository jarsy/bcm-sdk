/*
 * Generated from /projects/sbx-sdk/work/ren/views/SDK_5_9_0_EXA_PATCH_BRANCH/caladan_ucode/g2p3/src/epucode/sbG2EplibUcode.eg on Thu Feb 24 13:05:01 2011
 * GENERATED FILE DO NOT EDIT
 */

/*
 * -*-  Mode:text; c-basic-offset:4 -*-
 *
 * $Id: sbG2EplibUcode.c,v 1.7 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Guadalupe2k QE2000 Egress Processor Class Instructions
 *
 *======================================================================================
 *
 *  Processing Class Breakdown
 *
 *  Class | HW Type      |  Description
 *  ------+--------------+-------------------------------------------
 *  0     | VLAN         |  Traditional Bridging Unicast
 *  ------+--------------+-------------------------------------------
 *  1     | VLAN         |  Logical Interface Unicast
 *  ------+--------------+-------------------------------------------
 *  2     | VLAN         |  Traditional Bridging Multicast
 *  ------+--------------+-------------------------------------------
 *  3     | VLAN         |  Logical Interface Multicast
 *  ------+--------------+-------------------------------------------
 *  4     | VLAN         |  Logical Interface Multicast (McGroup > 32K)
 *  ------+--------------+-------------------------------------------
 *  5     | VLAN         |  IPMC (4k <= OI < 8k)
 *  ------+--------------+-------------------------------------------
 *
 *======================================================================================
 */

unsigned long aulKaEpClassInstr[16][8] =
{
	/*
	 * Class 0 Instruction Array
	 */
	{
		0x3406EC0B,
		0x340B4C23,
		0x1176EC29,
		0x1176EC18,
		0x90030419,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 1 Instruction Array
	 */
	{
		0x1A3F4C0A,
		0x11A74C0B,
		0x3402EC29,
		0x1176EC18,
		0x90030419,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 2 Instruction Array
	 */
	{
		0x10025C27,
		0x180A540A,
		0x1A0A540A,
		0x3406EC04,
		0x340B4C23,
		0x1176EC29,
		0x1176EC18,
		0x10030419
	},

	/*
	 * Class 3 Instruction Array
	 */
	{
		0x10025C27,
		0x112A5403,
		0x1A3F4C0A,
		0x11A74C0B,
		0x3402EC29,
		0x1176EC18,
		0x90030419,
		0x00000000
	},

	/*
	 * Class 4 Instruction Array
	 */
	{
		0x10025C27,
		0x132A5001,
		0x112A5403,
		0x1A3F4C0A,
		0x11A74C0B,
		0x3402EC29,
		0x1176EC18,
		0x10030419
	},

	/*
	 * Class 5 Instruction Array
	 */
	{
		0x1A53540B,
		0x18534C2A,
		0x3406EC29,
		0x340B4C23,
		0x1176EC29,
		0x1176EC18,
		0x90030419,
		0x00000000
	},

	/*
	 * Class 6 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 7 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 8 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 9 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 10 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 11 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 12 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 13 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 14 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	},

	/*
	 * Class 15 Instruction Array
	 */
	{
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000,
		0x00000000
	}
}; /* end aulKaEpClassInstr */

/* end array declarations */


