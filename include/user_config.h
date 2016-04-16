/** @file user_config.h
 *
 * @brief Hard wired firmware configuration.
 * 
 * @copyright
 * Copyright 2015 Martin Bo Kristensen Grønholdt <oblivion@@ace2>
 * 
 * @license
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */
#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

	// UART config
	#define SERIAL_BAUD_RATE 115200

	// ESP SDK config
	#define LWIP_OPEN_SRC
	#define USE_US_TIMER

	// Default types
	#define __CORRECT_ISO_CPP_STDLIB_H_PROTO
	#include <limits.h>
	#include <stdint.h>

	// Override c_types.h include and remove buggy espconn
	#define _C_TYPES_H_
	#define _NO_ESPCON_

	// Updated, compatible version of c_types.h
	// Just removed types declared in <stdint.h>
	#include <espinc/c_types_compatible.h>

	// System API declarations
	#include <esp_systemapi.h>

	// C++ Support
	#include <esp_cplusplus.h>
	// Extended string conversion for compatibility
	#include <stringconversion.h>
	// Network base API
	#include <espinc/lwip_includes.h>

	// Beta boards
	#define BOARD_ESP01
	
	/**
	 * @brief Project name.
	 */
	#define NAME "WakeLog"
	/**
	 * @brief Firmware version.
	 */
	#define VERSION "0.0.1"

	/**
	 * @brief Address in flash to save the log buffer.
	 */
	#define LOG_FLASH_SEC 0x3d

	#ifndef WIFI_SSID
		#error Please set WIFI_SSID.
	#endif

	#ifndef WIFI_PWD
		#error Please set WIFI_PWD.
	#endif

	/**
	 * @brief Interval between status print.
	 */
	#define STATUS_INT_SEC 60
	
	/**
	 * @brief Seconds between trying to connect to the network.
	 */
	#define WIFI_RETRY_SEC 15
	
	/**
	 * @brief mDNS name.
	 */
	#define MDNS_NAME "wakelog"

#ifdef __cplusplus
}
#endif

#endif
