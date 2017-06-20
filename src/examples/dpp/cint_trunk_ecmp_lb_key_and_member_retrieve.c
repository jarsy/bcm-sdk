/*  $Id: lb_key.c,v 1.3 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    cint_trunk_ecmp_lb_key_and_member_retrieve.c.c
 * Purpose: This CINT emulates the trunk(LAG) member and FEC resolution process preformed by the Arad.
 *          The file contains 3 essential functions:
 *              - fer_lb_key - Emulating the Arad's trunk and Ecmp LB keys calculation HW.
 *              - lag resolution - calculating the destination flow id or system physical port.
 *              - ecmp_resolution - ECMP resolution as described in the Arad PP Arch Spec under "FEC Resolution".
 */

/*General*/
static uint8 ECMP_LB_WIDTH = 16;
static uint8 ARAD_ECMP_LB_CRC_WIDTH = 16;
static uint8 ARAD_P_ECMP_LB_CRC_WIDTH = 15;
static uint8 LAG_LB_WIDTH = 16;
static uint8 ARAD_LAG_LB_CRC_WIDTH = 16;
static uint8 ARAD_P_LAG_LB_CRC_WIDTH = 15;

/*ECMP Polynomials:*/
static uint16 ARAD_ECMP_LB_POL[10] = { 0x0491, 0x7111, 0x9715, 0x55f5, 0x013d, 0x0053, 0x40cd, 0x3965, 0x698d, 0x105d };
/* 11 - Use counter incremented every packet*/
/* 12 - Use counter incremented every two clocks*/
static uint16 ARAD_P_ECMP_LB_POL[10] = { 0x0003, 0x0011, 0x0423, 0x0101, 0x04a1, 0x1019 };
/* 6-9 - Invalid*/
/* 11 - Use counter incremented every packet*/
/* 12 - Use counter incremented every two clocks*/

/*LAG Polynomials:*/
static uint16 ARAD_LAG_LB_POL[10] = { 0x0491, 0x7111, 0x9715, 0x55f5, 0x013d, 0x0053, 0x40cd, 0x3965, 0x698d, 0x105d };
/* 11 - Use counter incremented every packet*/
/* 12 - Use counter incremented every two clocks*/
static uint16 ARAD_P_LAG_LB_POL[10] = { 0x0003, 0x0011, 0x0423, 0x0101, 0x04a1, 0x1019 };
/* 6-9 - Invalid*/
/* 11 - Use counter incremented every packet*/
/* 12 - Use counter incremented every two clocks*/

/* Packet format codes:*/
static uint8 IHP_PFC_PART_NONE = 0;
static uint8 IHP_PFC_PART_E = 1;
static uint8 IHP_PFC_PART_IPV4 = 2;
static uint8 IHP_PFC_PART_IPV6 = 3;
static uint8 IHP_PFC_PART_TRILL = 4;
static uint8 IHP_PFC_PART_MPLS1 = 5;
static uint8 IHP_PFC_PART_MPLS2 = 6;
static uint8 IHP_PFC_PART_MPLS3 = 7;


/* Packet format codes (mappings from pkt format to constants):
								  [5:3]                [2:0]*/
static uint8 IHP_PFC_E = (8 * IHP_PFC_PART_NONE + IHP_PFC_PART_NONE);
static uint8 IHP_PFC_EoE = (8 * IHP_PFC_PART_NONE + IHP_PFC_PART_E);
static uint8 IHP_PFC_MPLS1oE = (8 * IHP_PFC_PART_NONE + IHP_PFC_PART_MPLS1);
static uint8 IHP_PFC_MPLS2oE = (8 * IHP_PFC_PART_NONE + IHP_PFC_PART_MPLS2);
static uint8 IHP_PFC_MPLS3oE = (8 * IHP_PFC_PART_NONE + IHP_PFC_PART_MPLS3);
static uint8 IHP_PFC_EoMPLS1oE = (8 * IHP_PFC_PART_E    + IHP_PFC_PART_MPLS1);
static uint8 IHP_PFC_EoMPLS2oE = (8 * IHP_PFC_PART_E    + IHP_PFC_PART_MPLS2);
static uint8 IHP_PFC_EoMPLS3oE = (8 * IHP_PFC_PART_E    + IHP_PFC_PART_MPLS3);
static uint8 IHP_PFC_IPv4oMPLS1oE = (8 * IHP_PFC_PART_IPV4 + IHP_PFC_PART_MPLS1);
static uint8 IHP_PFC_IPv4oMPLS2oE = (8 * IHP_PFC_PART_IPV4 + IHP_PFC_PART_MPLS2);
static uint8 IHP_PFC_IPv4oMPLS3oE = (8 * IHP_PFC_PART_IPV4 + IHP_PFC_PART_MPLS3);
static uint8 IHP_PFC_IPv6oMPLS1oE = (8 * IHP_PFC_PART_IPV6 + IHP_PFC_PART_MPLS1);
static uint8 IHP_PFC_IPv6oMPLS2oE = (8 * IHP_PFC_PART_IPV6 + IHP_PFC_PART_MPLS2);
static uint8 IHP_PFC_IPv6oMPLS3oE = (8 * IHP_PFC_PART_IPV6 + IHP_PFC_PART_MPLS3);
static uint8 IHP_PFC_IPv4oIPv4oE = (8 * IHP_PFC_PART_IPV4 + IHP_PFC_PART_IPV4);
static uint8 IHP_PFC_IPv6oIPv4oE = (8 * IHP_PFC_PART_IPV6 + IHP_PFC_PART_IPV4);
static uint8 IHP_PFC_IPv4oE = (8 * IHP_PFC_PART_NONE + IHP_PFC_PART_IPV4);
static uint8 IHP_PFC_IPv6oE = (8 * IHP_PFC_PART_NONE + IHP_PFC_PART_IPV6);
static uint8 IHP_PFC_EoTRILLoE = (8 * IHP_PFC_PART_E    + IHP_PFC_PART_TRILL);
static uint8 IHP_PFC_UD_TYPE = 32; /* 32 and up - User defined*/

/* forwarding code types*/
static uint8 PP_FWD_CODE_ETHERNET = 0;
static uint8 PP_FWD_CODE_IPV4_UC = 1;
static uint8 PP_FWD_CODE_IPV4_MC = 2;
static uint8 PP_FWD_CODE_IPV6_UC = 3;
static uint8 PP_FWD_CODE_IPV6_MC = 4;
static uint8 PP_FWD_CODE_MPLS = 5;
static uint8 PP_FWD_CODE_TRILL = 6;
static uint8 PP_FWD_CODE_CPU_TRAP_RESERVED = 7;
static uint8 PP_FWD_CODE_ETHERNET_AFTER_TERMINATION = 8;
static uint8 PP_FWD_CODE_TM = 14;
static uint8 PP_FWD_CODE_CPU_TRAP = 15;

static uint8 IHP_MPLS_HEADER_LENGTH = 4; /*length in bytes*/

/*LB_VECTOR_INDEX Values*/
static uint8 LB_VECTOR_INDEX_NONE = 0;
static uint8 LB_VECTOR_INDEX_ETHERNET = 1;
static uint8 LB_VECTOR_INDEX_FC = 2;
static uint8 LB_VECTOR_INDEX_IPV4 = 3;
static uint8 LB_VECTOR_INDEX_IPV6 = 4;
static uint8 LB_VECTOR_INDEX_MPLSX1 = 5;
static uint8 LB_VECTOR_INDEX_MPLSX2_LABEL1 = 6;
static uint8 LB_VECTOR_INDEX_MPLSX2_LABEL2 = 7;
static uint8 LB_VECTOR_INDEX_MPLSX3_LABEL1 = 8;
static uint8 LB_VECTOR_INDEX_MPLSX3_LABEL2 = 9;
static uint8 LB_VECTOR_INDEX_MPLSX3_LABEL3 = 10;
static uint8 LB_VECTOR_INDEX_TRILL = 11;
static uint8 LB_VECTOR_INDEX_L4 = 12;
static uint8 LB_VECTOR_INDEX_UD1 = 13;
static uint8 LB_VECTOR_INDEX_UD2 = 14;
static uint8 LB_VECTOR_INDEX_CALC = 15;

/*
 * Function:
 *    integer_bit_unpack
 * Purpose:
 *    Extracting a sequence of bits from a given integer.
 * Parameters: 
 *    number    -- integer to release bits from.
 *    offset    -- the offset is counted from the MSB.
 *    num_bits  -- length of the bit sequence.
 */
uint32 integer_bit_unpack(uint32 number, uint32 offset, uint32 num_bits) {
   uint32 res;
   res = (number >> (32 - offset - num_bits)) & ((1 << (offset + num_bits)) - 1);
   res = res & ((1 << num_bits) - 1);
   return (res);
}

/*
 * Function:
 *    bit_unpack
 * Purpose:
 *    Extracting a sequence of bits from a given array.
 * Parameters: 
 *    header    -- array to release bits from.
 *    offset    -- the offset is counted from the MSB of header[0] (=SOP-Start Of Packet).
 *    num_bits  -- length of the bit sequence (<=32).
 */
uint32 bit_unpack(uint32 *header, uint32 offset, uint32 num_bits) {
   uint32 header_index;
   uint32 modulo;
   uint32 res;
   header_index = (offset / 32);
   modulo = offset % 32;
   res = (integer_bit_unpack(header[header_index], modulo, 32 - modulo) << modulo) & ((1 << 32) - (1 << modulo));
   if (modulo != 0) res += integer_bit_unpack(header[header_index + 1], 0, modulo);
   res = integer_bit_unpack(res, 0, num_bits);
   return (res);
}

/*
 * Function:
 *    lb_lfsr
 * Purpose:
 *    Hash function used for LB key calaculation.
 * Parameters: 
 *    is_ecmp    -- 1-Ecmp hashing, 0-Lag hashing (the hashing is similar - this value is used only for prints).
 *    polynom    -- 16bit polynomial used for hashing.
 *    seed       -- hash seed.
 *    lb_width   -- the desired length for the output.
 *    data       -- data to hash.
 *    data_width -- length of data to hash.
 */
uint32 lb_lfsr(uint32 unit, uint8 is_ecmp, uint32 polynom, uint32 seed, uint32 lb_width, uint32 *data, uint32 data_width) {
   uint32 msb;
   uint32 lfsr;
   uint32 i;
   uint32 temp;
   int rv;

   printf("%s Hashing: polynom=0x%x, seed=0x%x, lb_width=%d, data=0x%x-%x-%x-%x, data_width=%d", (is_ecmp ? "ECMP" : "LAG "), polynom, seed, lb_width, data[0], data[1], data[2], data[3], data_width);
   lfsr = seed;
   for (i = 0; i < data_width; i++) {
	  msb  = (lfsr >> (lb_width - 1)) & 1;
	  lfsr = (lfsr << 1);
	  lfsr = lfsr & ((1 << lb_width) - 1);
	  temp = (i / 32);
	  lfsr = lfsr + (1 & (data[3 - (temp & ((1 << 5) - 1))] >> (i % 32)));
	  lfsr = lfsr ^ ((msb ? ((1 << lb_width) - 1) : 0) & polynom);
   }

   if (is_device_or_above(unit,ARAD_PLUS)) {
	  lfsr = (lfsr & ((1 << lb_width) - 1)) | (seed & (1 << lb_width));
	  for (i = 0; i < data_width; i++) {
		 temp = (i / 32);
		 msb  = (lfsr >> lb_width) & 1;
		 lfsr = lfsr & ((1 << lb_width) - 1);
		 lfsr = lfsr | (((1 & (data[3 - (temp & ((1 << 5) - 1))] >> (i % 32))) ^ msb) << lb_width);
	  }
   }

   printf(", result=0x%x\n", lfsr);
   return (lfsr);
}


/*
 * Function:
 *    fer_lb_key
 * Purpose:
 *    Emulating the Arad's Lag and Ecmp LB keys calculation HW.
 * Parameters: 
 *    unit                     -- unit number.
 *    packet_header            -- the complete headers stack (inputting the whole packet is possible), where the MSB of packet_header[0] is the SOP.
 *    parser_leaf_context      -- 4b leaf context calculated by the parser.
 *    packet_format_code       -- 6b code calculated by the parser which describe the header stack (values are according to the IHP_PFC 'define's).
 *    packet_qualifier         -- 5x5b generated by the parser to describe each header according to the following:
 *                                    - For Ethernet [4:0] - specify the next protocol using the following values:
 *                                          0 - No match;    1-7 - User Defined;    8 - Trill;    9 - Mac-In-Mac;    10 - Arp;
 *                                         11 - CFM;    12 - FCoE;    13 - IPv4;    14 - IPv6;    15 - MPLS;    16 - FC
 *                                    - For MPLS [0:0] - specify wheather the outermost MPLS label is the bottom of stack (values: 1/0);.
 *                                    - For IP [0:0] - specify wheather the IP packet is fragmented (values: 1/0);.
 *    vsi                      -- 16b resolved vsi for the packet.
 *    header_offset            -- 6x7b array to describe offsets of each header (0 to 5).
 *    forwarding_offset_index  -- 3b the index of the forwarding header (1 to 5).
 *    forwarding_offset_ext    -- 2b in a complex header (i.e. mpls stack) forwarding_offset_extension identifies the explicit header, otherwise it is '0'.
 *    forwarding_code          -- 4b the code which correspondes to the forwding header's type (values are according to the PP_FWD_CODE 'define's).
 *    ecmp_lb_key_packet_data  -- 20b generated by the pmf to add mapping capabillities.
 *    lag_lb_key_packet_data   -- 20b generated by the pmf to add mapping capabillities.
 *    in_port                  -- 8b the packets in port.
 */
uint32 fer_lb_key(
   uint32               unit,
   uint32               *packet_header,
   uint32               parser_leaf_context,     /* 4b*/
   uint32               packet_format_code,      /* 6b*/
   uint32               *packet_qualifier,       /* 5x7b*/
   uint32               vsi,                     /* 16b*/
   uint32               *header_offset,          /* 6x7b*/
   uint32               forwarding_offset_index, /* 3b*/
   uint32               forwarding_offset_ext,   /* 2b*/
   uint32               forwarding_code,         /* 4b*/
   uint32               ecmp_lb_key_packet_data, /* 20b*/
   uint32               lag_lb_key_packet_data,  /* 20b*/
   uint32               in_port                  /* 8b*/
   ) {
   uint32 lb_header[7];  /*28*8 bit*/
   uint32 lb_vector[6];  /*24*8 bit*/
   uint32 hdr_offset_index;
   uint32  chunk_size;
   uint32 chunk_bitmap[2];
   uint32 chunk_bitmap_not_aligned[2];
   uint32 ecmp_lb_key;
   uint32 lag_lb_key;
   uint16 ecmp_lb_poly, lag_lb_poly;
   uint8  ecmp_lb_crc_width, lag_lb_crc_width;
   uint8  i_bos;
   uint8  last_is_fragmented = 0;
   uint8  last_is_mpls = 0;
   uint8  last_is_bos = 0;
   uint8  last_is_fc = 0;
   uint8  bos_search = 0;
   uint32 crc_vector[4];
   uint8  hdr_offset;
   uint8 i;
   uint32 reg32_val;
   uint32 hdr;
   uint32  rv;

   uint8 lb_mpls_control_word;           /* 1b     corresponds to field LB_MPLS_CONTROL_WORD*/
   uint8 lag_lb_key_use_in_port;         /* 1b     corresponds to field LAG_LB_KEY_USE_IN_PORT*/
   uint16 lag_lb_key_seed;               /* 16b    corresponds to field LAG_LB_KEY_SEED*/
   uint8 lag_hash_index;                 /* 4b     corresponds to field LAG_LB_HASH_INDEX*/
   uint8 lag_lb_key_shift;               /* 4b     corresponds to field LAG_LB_KEY_SHIFT*/
   uint8 lag_lb_key_start;               /* 1b     corresponds to field LAG_LB_KEY_START*/
   uint8 lag_lb_key_count;               /* 2b     corresponds to field LAG_LB_KEY_COUNT*/
   uint8 ecmp_lb_key_use_in_port;        /* 1b     corresponds to field ECMP_LB_KEY_USE_IN_PORT*/
   uint16 ecmp_lb_key_seed;              /* 16b    corresponds to field ECMP_LB_KEY_SEED*/
   uint8 ecmp_hash_index;                /* 4b     corresponds to field ECMP_LB_HASH_INDEX*/
   uint8 ecmp_lb_key_shift;              /* 4b     corresponds to field LAG_LB_KEY_SHIFT*/
   uint8 ecmp_lb_key_count;              /* 2b     corresponds to field ECMP_LB_KEY_COUNT*/
   uint8 lb_profile;                     /* 1b     corresponds to field LB_PROFILE*/
   uint8 lb_bos_search;                  /* 1b     corresponds to field LB_BOS_SEARCH*/
   uint32 lb_include_bos_hdr;            /* 1b     corresponds to field LB_BOS_SEARCH*/
   uint8 parser_leaf_context_profile;    /* 1b     according to register PARSER_LEAF_CONTEXT_PROFILE*/
   uint32 lb_pfc_profile_table_index;
   uint32 lb_pfc_profile_entry;          /* 5x4b   according to memory IHB_LB_PFC_PROFILE*/
   uint8 lb_vector_index;                /* 4b     corresponds to field LB_VECTOR_INDEX1 to 5*/
   uint32 lb_vector_program_map[2];      /* 49b    according to memory IHB_LB_VECTOR_PROGRAM_MAP*/

   if (!is_device_or_above(unit,ARAD)) {
	  printf("This cint is compatible with Arad and Arad+ only!");
   }

   /*------------------------------
   *Retrieving configuration data
   *-----------------------------*/
   reg32_val = common_read_reg(unit, "IHB_LAG_LB_KEY_CFG");
   lag_lb_key_seed        = get_field(reg32_val, 0,  15); /*LAG_LB_KEY_SEED*/
   lag_hash_index         = get_field(reg32_val, 16, 19); /*LAG_LB_HASH_INDEX*/
   lag_lb_key_use_in_port = get_field(reg32_val, 20, 20); /*LAG_LB_KEY_USE_IN_PORT*/
   lag_lb_key_shift       = get_field(reg32_val, 21, 24); /*LAG_LB_KEY_SHIFT*/
   lb_mpls_control_word   = get_field(reg32_val, 25, 25); /*LB_MPLS_CONTROL_WORD*/

   reg32_val = common_read_reg(unit, "IHB_ECMP_LB_KEY_CFG");
   ecmp_lb_key_seed        = get_field(reg32_val, 0, 15); /*ECMP_LB_KEY_SEED*/
   ecmp_hash_index         = get_field(reg32_val, 16, 19); /*ECMP_LB_HASH_INDEX*/
   ecmp_lb_key_use_in_port = get_field(reg32_val, 20, 20); /*ECMP_LB_KEY_USE_IN_PORT*/
   ecmp_lb_key_shift       = get_field(reg32_val, 21, 24); /*ECMP_LB_KEY_SHIFT*/

   common_read_table_entry(unit, "IHB_PINFO_FER", in_port, &reg32_val, 1);
   ecmp_lb_key_count  = get_field(reg32_val, 0, 0); /*ECMP_LB_KEY_COUNT*/
   lag_lb_key_start   = get_field(reg32_val, 1, 1); /*LAG_LB_KEY_START*/
   lag_lb_key_count   = get_field(reg32_val, 2, 3); /*LAG_LB_KEY_COUNT*/
   lb_profile         = get_field(reg32_val, 4, 4); /*LB_PROFILE*/
   lb_bos_search      = get_field(reg32_val, 5, 5); /*LB_BOS_SEARCH*/
   lb_include_bos_hdr = get_field(reg32_val, 6, 6); /*LB_INCLUDE_BOS_HDR*/

   reg32_val = common_read_reg(unit, "IHB_LB_KEY_PARSER_LEAF_CONTEXT_PROFILE");
   parser_leaf_context_profile = (reg32_val >> parser_leaf_context) & 1;
   lb_pfc_profile_table_index = ((lb_profile & 1) << 7) + ((parser_leaf_context_profile & 1) << 6)
	  + (packet_format_code & ((1 << 6) - 1));

   common_read_table_entry(unit, "IHB_LB_PFC_PROFILE", lb_pfc_profile_table_index, &lb_pfc_profile_entry, 1);

   if (is_device_or_above(unit,ARAD_PLUS)) {
	  ecmp_lb_crc_width = ARAD_P_ECMP_LB_CRC_WIDTH;
	  lag_lb_crc_width = ARAD_P_LAG_LB_CRC_WIDTH;
	  ecmp_lb_poly = ARAD_P_ECMP_LB_POL[ecmp_hash_index];
	  lag_lb_poly = ARAD_P_LAG_LB_POL[lag_hash_index];
   } else { /* is arad(unit) == 1*/
	  ecmp_lb_crc_width = ARAD_ECMP_LB_CRC_WIDTH;
	  lag_lb_crc_width = ARAD_LAG_LB_CRC_WIDTH;
	  ecmp_lb_poly = ARAD_ECMP_LB_POL[ecmp_hash_index];
	  lag_lb_poly = ARAD_LAG_LB_POL[lag_hash_index];
   }

   /*---------------------------------
   *Calculate ECMP-LB-Key & LAG-LB-Key
   *----------------------------------*/
   ecmp_lb_key = ecmp_lb_key_seed;
   lag_lb_key = lag_lb_key_seed;
   printf("Initial seeds: Lag_lb_key=0x%x, Ecmp_lb_key 0x%x\n", lag_lb_key, ecmp_lb_key);

   for (hdr = 0; hdr < 3; hdr++) { /*Loops through the headers*/
	  if (hdr == 2 && lag_lb_key_start == 0) hdr = -1; /* on last iteration, if bit is 1 then proccess forward header minus 1*/
	  if (hdr == 0) {
		 last_is_fragmented = 0;
		 last_is_mpls = 0;
		 last_is_bos = 0;
		 last_is_fc = 0;
		 bos_search = 0;
	  }

	  crc_vector[0] = 0; crc_vector[1] = 0; crc_vector[2] = 0; crc_vector[3] = 0;

	  /*Determine the header's offset*/
	  hdr_offset_index = forwarding_offset_index + hdr;
	  if (!bos_search || hdr == -1) hdr_offset = header_offset[hdr_offset_index];

	  /*determine what vector program to use*/
	  if (hdr_offset_index <= 5 && hdr_offset_index >= 1) lb_vector_index = get_field(lb_pfc_profile_entry, (hdr_offset_index - 1) * 4, (hdr_offset_index) * 4 - 1);
	  else lb_vector_index = IHP_PFC_PART_NONE;

	  /* This is the first header and this is mpls label 1*/
	  if (hdr == 0 && (lb_vector_index == LB_VECTOR_INDEX_MPLSX1 || lb_vector_index == LB_VECTOR_INDEX_MPLSX2_LABEL1 || lb_vector_index == LB_VECTOR_INDEX_MPLSX3_LABEL1)) lb_vector_index += forwarding_offset_ext;

	  if (bos_search) {
		 if (hdr == 1) {
			hdr_offset += (i_bos - 1) * IHP_MPLS_HEADER_LENGTH;
			if (!lb_include_bos_hdr) lb_vector_index = IHP_PFC_PART_NONE;
		 } else if (hdr == 2) {
			hdr_offset += IHP_MPLS_HEADER_LENGTH;
		 }
	  }

	  if (last_is_fragmented && hdr != -1) lb_vector_index = LB_VECTOR_INDEX_NONE;

	  /* Calc mode*/
	  if (lb_vector_index == LB_VECTOR_INDEX_CALC) {
		 if (hdr == 0 || hdr == -1) {
			lb_vector_index = LB_VECTOR_INDEX_NONE;
		 } else if (last_is_mpls) {
			if (!last_is_bos || (bos_search && hdr == 1 && lb_include_bos_hdr)) lb_vector_index = LB_VECTOR_INDEX_MPLSX1;
			else if (bit_unpack(packet_header, 8 * hdr_offset, 4) == 4) lb_vector_index = LB_VECTOR_INDEX_IPV4;
			else if (bit_unpack(packet_header, 8 * hdr_offset, 4) == 6) lb_vector_index = LB_VECTOR_INDEX_IPV6;
			else {
			   lb_vector_index = LB_VECTOR_INDEX_ETHERNET;
			   if (lb_mpls_control_word) hdr_offset += 4;
			}
		 } else if (last_is_fc) lb_vector_index = LB_VECTOR_INDEX_FC;
		 else lb_vector_index = LB_VECTOR_INDEX_NONE;
	  }
	  /* This is an IP packet*/
	  last_is_fragmented = (lb_vector_index == LB_VECTOR_INDEX_IPV4 || lb_vector_index == LB_VECTOR_INDEX_IPV6)
		 && (1 & (packet_qualifier[hdr_offset_index - 1]));
	  last_is_mpls = (lb_vector_index == LB_VECTOR_INDEX_MPLSX1)
		 ||    (lb_vector_index == LB_VECTOR_INDEX_MPLSX2_LABEL1)
		 ||    (lb_vector_index == LB_VECTOR_INDEX_MPLSX2_LABEL2)
		 ||    (lb_vector_index == LB_VECTOR_INDEX_MPLSX3_LABEL1)
		 ||    (lb_vector_index == LB_VECTOR_INDEX_MPLSX3_LABEL2)
		 ||    (lb_vector_index == LB_VECTOR_INDEX_MPLSX3_LABEL3);
	  last_is_fc = (lb_vector_index == LB_VECTOR_INDEX_ETHERNET) && (hdr_offset_index == 1)
		 && (((1 << 4) - 1) & (packet_qualifier[hdr_offset_index - 1] >> 4));

	  if (last_is_mpls) {
		 if (lb_bos_search && hdr == 0) {
			for (i_bos = 1; i_bos < 7 && !last_is_bos; i_bos++) { /*searching for BOS bit*/
			   /*{Label, Exp, BOS, TTL}*/
			   /*  20     3    1    8*/
			   last_is_bos = bit_unpack(packet_header, 8 * (hdr_offset + i_bos * IHP_MPLS_HEADER_LENGTH) + 23, 1);
			}
			last_is_bos = 1;
			bos_search = 1;
			lb_vector_index = LB_VECTOR_INDEX_NONE;
		 } else last_is_bos = packet_qualifier[hdr_offset_index - 1] & 1;
	  }

	  /*Extract the relevant header from the headers stack/packet and interleave if Eth or IPv6*/
	  if (lb_vector_index == LB_VECTOR_INDEX_ETHERNET) {
		 /* ethernet header for proccess:*/
		 /* vsi(16b) | 0(4b) | Next Protocol(4b) | DA [47:40]*/
		 lb_header[0] = (vsi << 16) + (((packet_qualifier[hdr_offset_index - 1]) & ((1 << 4) - 1)) << 8) + bit_unpack(packet_header, 8 * hdr_offset, 8);
		 /* DA [39:32] | SA[47:32] | DA[31:24] |*/
		 lb_header[1] = (bit_unpack(packet_header, 8 * (hdr_offset + 1), 8) << 24) + (bit_unpack(packet_header, 8 * (hdr_offset + 6), 16) << 8) + bit_unpack(packet_header, 8 * (hdr_offset + 2), 8);
		 /* DA[23:16] | SA[31:16] | DA[15:8]*/
		 lb_header[2] = (bit_unpack(packet_header, 8 * (hdr_offset + 3), 8) << 24) + (bit_unpack(packet_header, 8 * (hdr_offset + 8), 16) << 8) + bit_unpack(packet_header, 8 * (hdr_offset + 4), 8);
		 /* DA[7:0] | SA[15:0] | Ethernet Packet Byte 13*/
		 lb_header[3] = (bit_unpack(packet_header, 8 * (hdr_offset + 5), 8) << 24) + (bit_unpack(packet_header, 8 * (hdr_offset + 10), 16) << 8) + bit_unpack(packet_header, 8 * (hdr_offset + 12), 8);
		 /* Ethernet Packet Byte 14 - 25*/
		 for (i = 0; i < 3; i++) {
			lb_header[4 + i] = bit_unpack(packet_header, 8 * (hdr_offset + 13 + i * 4), 32);
		 }
	  } else if (lb_vector_index == LB_VECTOR_INDEX_IPV6) {
		 /* IPv6 header for process:*/
		 /* SIP[95:80]^SIP[47:32] | DIP[95:80]^SIP[47:32]*/
		 lb_header[0] = ((bit_unpack(packet_header, 8 * (hdr_offset + 8 + 4), 16) ^ bit_unpack(packet_header, 8 * (hdr_offset + 8 + 4 + 6), 16)) << 16) +
			(bit_unpack(packet_header, 8 * (hdr_offset + 24 + 4), 16) ^ bit_unpack(packet_header, 8 * (hdr_offset + 24 + 4 + 6), 16));
		 /* SIP[127:112]^SIP[79:64]^SIP[31:16] | DIP[127:112]^DIP[79:64]^DIP[31:16]*/
		 lb_header[1] = ((bit_unpack(packet_header, 8 * (hdr_offset + 8), 16) ^ bit_unpack(packet_header, 8 * (hdr_offset + 8 + 4 + 2), 16) ^ bit_unpack(packet_header, 8 * (hdr_offset + 8 + 4 + 6 + 2), 16)) << 16) +
			(bit_unpack(packet_header, 8 * (hdr_offset + 24), 16) ^ (bit_unpack(packet_header, 8 * (hdr_offset + 24 + 4 + 2), 16) ^ bit_unpack(packet_header, 8 * (hdr_offset + 24 + 4 + 6 + 2), 16)));
		 /* SIP[111:96]^SIP[63:48]^SIP[15:0] | DIP[111:96]^DIP[63:48]^DIP[15:0]*/
		 lb_header[2] = ((bit_unpack(packet_header, 8 * (hdr_offset + 8 + 2), 16) ^ bit_unpack(packet_header, 8 * (hdr_offset + 8 + 4 + 4), 16) ^ bit_unpack(packet_header, 8 * (hdr_offset + 8 + 4 + 6 + 4), 16)) << 16) +
			(bit_unpack(packet_header, 8 * (hdr_offset + 24 + 2), 16) ^ (bit_unpack(packet_header, 8 * (hdr_offset + 24 + 4 + 4), 16) ^ bit_unpack(packet_header, 8 * (hdr_offset + 24 + 4 + 6 + 4), 16)));
		 for (i = 0; i < 4; i++) {
			lb_header[3 + i] = bit_unpack(packet_header, 8 * (hdr_offset + i * 4), 32);
		 }
	  } else { /* If !Eth && !IPv6*/
		 for (i = 0; i < 7; i++) {
			lb_header[i] = bit_unpack(packet_header, 8 * (hdr_offset + i * 4), 32);
		 }
	  }

	  /* Load program map*/
	  lb_vector_program_map[0] = 0; lb_vector_program_map[1] = 0;
	  chunk_bitmap_not_aligned[0] = 0; chunk_bitmap_not_aligned[1] = 0;
	  if (lb_vector_index > 15) {
		 printf("Unknown LB Vector Index %d. Intializing LB Vector Map to all 0's.\n", lb_vector_index);
	  } else {
		 common_read_table_entry(unit, "IHB_LB_VECTOR_PROGRAM_MAP", lb_vector_index, &lb_vector_program_map, 2);
	  }
	  get_field_long(&lb_vector_program_map, &chunk_size, 48, 48, 1);
	  chunk_size = 4 * (1 + (chunk_size & 1));
	  /* Move bitmap to left-aligned data representation*/
	  get_field_long(&lb_vector_program_map, &chunk_bitmap_not_aligned, 0, 47, 2);
	  chunk_bitmap[0] = (((chunk_bitmap_not_aligned[1]) << 16) & ((1 << 32) - (1 << 16))) + (((chunk_bitmap_not_aligned[0]) >> 16) & ((1 << 16) - 1));
	  chunk_bitmap[1] = (((chunk_bitmap_not_aligned[0])) & ((1 << 16) - 1)) << 16;

	  /* Apply program map to the extracted header*/
	  for (i = 0; i < 6; i++) lb_vector[i] = 0;
	  for (i = 0; i < 24; i++) {
		 uint8  program = bit_unpack(chunk_bitmap, i * 2, 2);
		 uint8  data_temp[3] = { 0, 0, 0 };
		 uint8    lb_vector_temp = 0;

		 data_temp[0] = bit_unpack(lb_header, (i + 0) * chunk_size, chunk_size);
		 data_temp[1] = bit_unpack(lb_header, (i + 2) * chunk_size, chunk_size);
		 data_temp[2] = bit_unpack(lb_header, (i + 4) * chunk_size, chunk_size);

		 if        (program == 1) lb_vector_temp = data_temp[0];
		 else if (program == 2) lb_vector_temp = data_temp[0] ^ data_temp[1];
		 else if (program == 3) lb_vector_temp = data_temp[0] ^ data_temp[2];

		 if (chunk_size == 4)     lb_vector[i / 8] = ((lb_vector[i / 8] << 4) & ((1 << 32) - (1 << 4))) + lb_vector_temp;
		 else                     lb_vector[i / 4] = ((lb_vector[i / 4] << 8) & ((1 << 32) - (1 << 8))) + lb_vector_temp;
	  }
	  /* Align vector to the right, and if chunk_size=8, fold 192b vector to 120b*/
	  if (chunk_size == 8) {
		 crc_vector[0] = bit_unpack(lb_vector, 0, 32 - 8);
		 crc_vector[1] = ((bit_unpack(lb_vector, 32 - 8, 24) << 8) & ((1 << 32) - (1 << 8))) + (bit_unpack(lb_vector, 48, 8) ^ bit_unpack(lb_vector, 120, 8));
		 crc_vector[2] = bit_unpack(lb_vector, 64 - 8, 32) ^ bit_unpack(lb_vector, 128, 32);
		 crc_vector[3] = bit_unpack(lb_vector, 96 - 8, 32) ^ bit_unpack(lb_vector, 160, 32);
	  } else {
		 crc_vector[0] = 0;
		 crc_vector[1] = bit_unpack(lb_vector, 0, 32);
		 crc_vector[2] = bit_unpack(lb_vector, 32, 32);
		 crc_vector[3] = bit_unpack(lb_vector, 64, 32);
	  }

	  /*ecmp_lb_key_count 0: 1 hdr, 1: 2 hdr*/
	  if (hdr == 0 || (ecmp_lb_key_count == 1 && hdr == 1)) ecmp_lb_key = lb_lfsr(unit, 1, ecmp_lb_poly, ecmp_lb_key, ecmp_lb_crc_width, crc_vector, 120);

	  /*lag_lb_key_start 0: start from fwd, 1: start from fwd-1*/
	  /*lag_lb_key_count number of headers to take*/
	  if ((hdr == -1 && lag_lb_key_count > 0) ||
		  (hdr == 0 && lag_lb_key_start == 0 && lag_lb_key_count > 1) ||
		  (hdr == 0 && lag_lb_key_start == 1 && lag_lb_key_count > 0) ||
		  (hdr == 1 && lag_lb_key_start == 0 && lag_lb_key_count > 2) ||
		  (hdr == 1 && lag_lb_key_start == 1 && lag_lb_key_count > 1) ||
		  (hdr == 2 && lag_lb_key_start == 1 && lag_lb_key_count > 2)) lag_lb_key = lb_lfsr(unit, 0, lag_lb_poly, lag_lb_key, lag_lb_crc_width, crc_vector, 120);
	  if (hdr == -1)    hdr = 2;
   } /*for*/
   printf("Headers Digest is done.\n");

   /* Augment packet data & in_port*/
   printf("Processing Ecmp and Lag PMF packet data (and in-port, if required):\n");
   crc_vector[3] = 0; crc_vector[2] = 0; crc_vector[1] = 0; crc_vector[0] = 0;
   crc_vector[3] = ecmp_lb_key_packet_data;
   if (ecmp_lb_key_use_in_port) crc_vector[3] = ((in_port << 20) & ((1 << 32) - (1 << 20))) + crc_vector[3];

   ecmp_lb_key = lb_lfsr(unit, 1, ecmp_lb_poly, ecmp_lb_key, ecmp_lb_crc_width, crc_vector, 120);

   crc_vector[3] = 0; crc_vector[2] = 0; crc_vector[1] = 0; crc_vector[0] = 0;
   crc_vector[3] = lag_lb_key_packet_data;
   if (lag_lb_key_use_in_port)  crc_vector[3] = ((in_port << 20) & ((1 << 32) - (1 << 20))) + crc_vector[3];

   lag_lb_key = lb_lfsr(unit, 0, lag_lb_poly, lag_lb_key, lag_lb_crc_width, crc_vector, 120);

   /* Asserting a HW limitation which allows only one use at a time of each crc function(each polynomial):*/
   if (ecmp_hash_index == lag_hash_index)       {
	  printf("Misconfiguration: Ecmp and Lag are configured with similar hash indeices - overriding Lag result!\n");
	  lag_lb_key = ecmp_lb_key;
   }

   /* Shift LB keys*/
   if (ecmp_lb_key_shift || lag_lb_key_shift) printf("Shifting LB keys.\n");
   ecmp_lb_key = (ecmp_lb_key >> ecmp_lb_key_shift) | (ecmp_lb_key << (ECMP_LB_WIDTH - ecmp_lb_key_shift));
   ecmp_lb_key = ecmp_lb_key & ((1 << ECMP_LB_WIDTH) - 1);
   lag_lb_key  = (lag_lb_key  >> lag_lb_key_shift) | (lag_lb_key  << (LAG_LB_WIDTH - lag_lb_key_shift));
   lag_lb_key  = lag_lb_key  & ((1 << LAG_LB_WIDTH) - 1);

   printf("Final result: Lag_lb_key=0x%x, Ecmp_lb_key=0x%x\n", lag_lb_key, ecmp_lb_key);
   return ((lag_lb_key << ECMP_LB_WIDTH) + ecmp_lb_key);
}

/*
 * Function:
 *    lb_key_example
 * Purpose:
 *    Example for fer_lb_key use.
 *    The packet in the example is IPv6oEthernet with:
 *    Ethernet:
 *        DA=01:02:03:04:05:06, SA=07:08:09:0A:0B:0C
 *        Protocol=0x8100, UserPriority=2, VLAN=1, Type=0x86dd
 *    IP:
 *        Version=6, TC=1, HopLimit=0xff
 *        Source= 0009:000A:000B:000C:000D:000E:000F:0010
 *        Dest=      0001:0002:0003:0004:0005:0006:0007:0008
 * Configuration(optional): 
 *     The next script will enable Lag and Ecmp on the device:
		cint sdk/src/examples/dpp/cint_trunk.c                  
		cint sdk/src/examples/dpp/validate/cint_hw_access.c    
		cint sdk/src/examples/dpp/cint_trunk_ecmp_lb_key_and_member_retrieve.c          
		cint                                                                   
		print main_trunk(0, 1, 0, 14, 15, 16, 17, 13, 2);
		quit;
		w IHB_PINFO_FER 1 1 LAG_LB_KEY_START=1 LAG_LB_KEY_COUNT=2 ECMP_LB_KEY_COUNT=0
		s IHB_ECMP_LB_KEY_CFG ECMP_LB_KEY_SHIFT=1 ECMP_LB_KEY_SEED=31 ECMP_LB_HASH_INDEX=2
		s IHB_LAG_LB_KEY_CFG LAG_LB_KEY_USE_IN_PORT=1 LAG_LB_KEY_SEED=1
		w IHB_LB_VECTOR_PROGRAM_MAP 1 1 CHUNK_SIZE=1,CHUNK_BITMAP=0x554afaf55005
		w IHB_LB_VECTOR_PROGRAM_MAP 4 1 CHUNK_SIZE=0,CHUNK_BITMAP=0x0ff555a0515a
		cint
		fer_lb_key_IPv6oE_example();
 *      Expected result: Lag_lb_key=0x44d3, Ecmp_lb_key=0x5177
 */
void fer_lb_key_IPv6oE_example() {
   uint32 unit = 0;
   uint32 packet_header[17] =        { 0x01020304, 0x05060708, 0x090a0b0c, 0x81004001,
	  0x86dd6010, 0x00000006, 0x3bff0009, 0x000a000b,
	  0x000c000d, 0x000e000f, 0x00100001, 0x00020003,
	  0x00040005, 0x00060007, 0x00080001, 0x02030405, 0xd9b40242 };
   uint32 parser_leaf_context = 0xf;
   uint32 packet_format_code = 3;           /* IHP_PFC_IPv6oE*/
   uint32 packet_qualifier[5] = { 14, 0, 0, 0, 0 }; /* Ethernet qualifier = 14 (=next protocol is IPv6)*/
   uint32 vsi = 1;                            /* Defualt config = VLAN==VSI */
   uint32 header_offset[6] = { 0, 0, 0x12, 0x3a, 0x3a, 0x3a };         /* L1|ETH|IPV6|L5|DATA*/
   uint32 forwarding_offset_index = 1;      /* Index of the forwarding header (the Ethernet header)*/
   uint32 forwarding_offset_ext   = 0;      /* No Mpls headers*/
   uint32 forwarding_code = 0;                /* = PP_FWD_CODE_ETHERNET*/
   uint32 ecmp_lb_key_packet_data = 0, lag_lb_key_packet_data = 0;
   uint32 in_port = 16;

   fer_lb_key(unit, packet_header, parser_leaf_context, packet_format_code, packet_qualifier, vsi, header_offset,
			  forwarding_offset_index, forwarding_offset_ext, forwarding_code, ecmp_lb_key_packet_data, lag_lb_key_packet_data, in_port);
}
/*
 * Function:
 *    lb_key_example
 * Purpose:
 *    Example for fer_lb_key use.
 *    The packet in the example is IPv4oEthernet with:
 *    Ethernet:
 *        DA=01:02:03:04:05:06, SA=07:08:09:0A:0B:0C
 *        Protocol=0x8100, UserPriority=2, VLAN=1, Type=0x800
 *    IP:
 *        Version=4, TC=1
 *        SIP= 10.184.8.111
 *        DIP= 10.184.8.231
 * Configuration(optional): 
 *     The next script will enable Lag and Ecmp on the device:
		cd ../../../../src/examples/dpp/
    	cint validate/cint_hw_access.c
    	cint utility/cint_utils_global.c
		cint cint_trunk.c
		cint cint_trunk_ecmp_lb_key_and_member_retrieve.c
		cint                      
		print main_trunk(0, 1, 0, 14, 15, 16, 17, 13, 2);
		quit;
		w IHB_PINFO_FER 17 1 LAG_LB_KEY_START=1 LAG_LB_KEY_COUNT=2 ECMP_LB_KEY_COUNT=1
		s IHB_ECMP_LB_KEY_CFG ECMP_LB_KEY_SHIFT=1 ECMP_LB_KEY_SEED=31 ECMP_LB_HASH_INDEX=2
		s IHB_LAG_LB_KEY_CFG LAG_LB_KEY_USE_IN_PORT=1 LAG_LB_KEY_SEED=1 LAG_LB_HASH_INDEX=1
		w IHB_LB_VECTOR_PROGRAM_MAP 1 1 CHUNK_SIZE=1,CHUNK_BITMAP=0x0
		w IHB_LB_VECTOR_PROGRAM_MAP 4 1 CHUNK_SIZE=0,CHUNK_BITMAP=0x0
		cint
		fer_lb_key_IPv4oE_example();
 *      Expected result: Lag_lb_key=0xb3b6, Ecmp_lb_key=0x16f0
 */
void fer_lb_key_IPv4oE_example() {
   uint32 unit = 0;
   uint32 packet_header[17] =        { 0x01020304, 0x05060708, 0x090a0b0c, 0x81004001,
	  0x08004500, 0x002e0000, 0x000040ff, 0x530c0ab8,
	  0x086f0ab8, 0x08e70001, 0x02030405, 0x06070809,
	  0x0a0b0c0d, 0x0e0f1011, 0x12131415, 0x16171819, 0x4e29f638 };
   uint32 parser_leaf_context = 0xf;
   uint32 packet_format_code = 2;           /* IHP_PFC_IPv4oE*/
   uint32 packet_qualifier[5] = { 13, 0, 0, 0, 0 }; /* Ethernet qualifier = 13 (=next protocol is IPv4)*/
   uint32 vsi = 1;                            /* Defualt config = VLAN==VSI */
   uint32 header_offset[6] = { 0, 0, 0x12, 0x26, 0x26, 0x26 };         /* L1|ETH|IPV4|L5|DATA*/
   uint32 forwarding_offset_index = 1;      /* Index of the forwarding header (the Ethernet header)*/
   uint32 forwarding_offset_ext   = 0;      /* No Mpls headers*/
   uint32 forwarding_code = 0;                /* = PP_FWD_CODE_ETHERNET*/
   uint32 ecmp_lb_key_packet_data = 0, lag_lb_key_packet_data = 0;
   uint32 in_port = 13;

   fer_lb_key(unit, packet_header, parser_leaf_context, packet_format_code, packet_qualifier, vsi, header_offset,
			  forwarding_offset_index, forwarding_offset_ext, forwarding_code, ecmp_lb_key_packet_data, lag_lb_key_packet_data, in_port);
}

/*
 * Function:
 *    ecmp_resolution
 * Purpose:
 *    ecmp_resolution - ECMP resolution as described in the Arad PP Arch Spec under "FEC Resolution"
 * Parameters: 
 *    unit        -- unit number.
 *    ecmp_lb_key -- Ecmp load balancing key (the 16 LSBs of fer_lb_key's return value)
 *    fec_pointer -- packet's FEC pointer.
 */
int ecmp_resolution(int unit, uint32 ecmp_lb_key, uint32 fec_pointer) {
   int rv;
   uint32 reg32_val;
   if (fec_pointer < 1024) {
	  uint32 offset;
	  uint32 group_size;
	  uint32 protect;
	  uint32 group_start_pointer;
	  common_read_table_entry(unit, "IHB_FEC_ECMP", fec_pointer, &reg32_val, 1);
	  group_start_pointer = get_field(reg32_val, 0, 14);/*GROUP_SIZE*/
	  group_size = get_field(reg32_val, 15, 23);/*GROUP_SIZE*/
	  protect = get_field(reg32_val,24,24);/*PROTECTED*/
	  offset = (group_size * ecmp_lb_key) >> 16;
	  offset = offset * (1 << protect);
	  return group_start_pointer + offset;
   }
   return -1;
}
