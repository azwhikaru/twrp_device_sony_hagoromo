/*
* Copyright (C) 2011-2015 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify it under the terms of the
* GNU General Public License version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/
/*
 */

enum tveeprom_audio_processor {
	/* No audio processor present */
	TVEEPROM_AUDPROC_NONE,
	/* The audio processor is internal to the video processor */
	TVEEPROM_AUDPROC_INTERNAL,
	/* The audio processor is a MSPXXXX device */
	TVEEPROM_AUDPROC_MSP,
	/* The audio processor is another device */
	TVEEPROM_AUDPROC_OTHER,
};

#include <linux/if_ether.h>

struct tveeprom {
	u32 has_radio;
	/* If has_ir == 0, then it is unknown what the IR capabilities are,
	   otherwise:
	   bit 0: 1 (= IR capabilities are known)
	   bit 1: IR receiver present
	   bit 2: IR transmitter (blaster) present */
	u32 has_ir;
	u32 has_MAC_address; /* 0: no MAC, 1: MAC present, 2: unknown */

	u32 tuner_type;
	u32 tuner_formats;
	u32 tuner_hauppauge_model;

	u32 tuner2_type;
	u32 tuner2_formats;
	u32 tuner2_hauppauge_model;

	u32 digitizer;
	u32 digitizer_formats;

	u32 audio_processor;
	u32 decoder_processor;

	u32 model;
	u32 revision;
	u32 serial_number;
	char rev_str[5];
	u8 MAC_address[ETH_ALEN];
};

void tveeprom_hauppauge_analog(struct i2c_client *c, struct tveeprom *tvee,
			       unsigned char *eeprom_data);

int tveeprom_read(struct i2c_client *c, unsigned char *eedata, int len);
