/*
 * Copyright 2016 Sony Corporation
 * File changed on 2016-01-20
 */
/*
 * digiamp.c
 *
 * digital amp interface
 *
 * Copyright (c) 2013 Sony Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <linux/kernel.h>
#include <sound/digiamp.h>

static struct digital_amp_interface * digiamp_interface = NULL;

int digiamp_register(struct digital_amp_interface * interface)
{
	digiamp_interface = interface;

	return(0);
}

int digiamp_get_type(void)
{
	if(digiamp_interface==NULL)
		return(0);

	return(digiamp_interface->type);
}

int digiamp_power_on(void)
{
	int rv;

	if(digiamp_interface!=NULL && digiamp_interface->power_on!=NULL){
		rv=digiamp_interface->power_on();
		if(rv<0)
			return(rv);
	}

	return(0);
}

int digiamp_power_off(void)
{
	int rv;

	if(digiamp_interface!=NULL && digiamp_interface->power_off!=NULL){
		rv=digiamp_interface->power_off();
		if(rv<0)
			return(rv);
	}

	return(0);
}

int digiamp_initialize(void)
{
	int rv;

	if(digiamp_interface!=NULL && digiamp_interface->initialize!=NULL){
		rv=digiamp_interface->initialize();
		if(rv<0)
			return(rv);
	}

	return(0);
}

int digiamp_shutdown(void)
{
	int rv;

	if(digiamp_interface!=NULL && digiamp_interface->shutdown!=NULL){
		rv=digiamp_interface->shutdown();
		if(rv<0)
			return(rv);
	}

	return(0);
}

int digiamp_enable(int level)
{
	int rv;

	if(digiamp_interface!=NULL && digiamp_interface->enable!=NULL){
		rv=digiamp_interface->enable(level);
		if(rv<0)
			return(rv);
	}

	return(0);
}

int digiamp_disable(int level)
{
	int rv;

	if(digiamp_interface!=NULL && digiamp_interface->disable!=NULL){
		rv=digiamp_interface->disable(level);
		if(rv<0)
			return(rv);
	}

	return(0);
}

int digiamp_fade_volume(unsigned int volume)
{
	int rv;

	if(digiamp_interface!=NULL && digiamp_interface->fade_volume!=NULL){
		rv=digiamp_interface->fade_volume(volume);
		if(rv<0)
			return(rv);
	}

	return(0);
}

int digiamp_set_volume(unsigned int volume)
{
	int rv;

	if(digiamp_interface!=NULL && digiamp_interface->set_volume!=NULL){
		rv=digiamp_interface->set_volume(volume);
		if(rv<0)
			return(rv);
	}

	return(0);
}

int digiamp_switch_sys_clock(int value)
{
	int rv;

	if(digiamp_interface!=NULL && digiamp_interface->switch_sys_clock!=NULL){
		rv=digiamp_interface->switch_sys_clock(value);
		if(rv<0)
			return(rv);
	}

	return(0);
}

int digiamp_switch_shunt_mute(int value)
{
	int rv;

	if(digiamp_interface!=NULL && digiamp_interface->switch_shunt_mute!=NULL){
		rv=digiamp_interface->switch_shunt_mute(value);
		if(rv<0)
			return(rv);
	}

	return(0);
}

