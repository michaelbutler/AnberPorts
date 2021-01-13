#!/usr/bin/env python3

# Copyright (c) 2021
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, write to the
# Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA
#
# Authored by: Kris Henriksen <krishenriksen.work@gmail.com>
#
# AnberPorts (Rockchip rk3326 ports)
#

import asyncio
import evdev
import uinput

async def handle_event(device):
	async for event in device.async_read_loop():
		if device.name == "OpenSimHardware OSH PB Controller" or device.name == "OSH PB Controller":
			# a
			if event.code == 304 and event.value == 1:
				with uinput.Device([uinput.KEY_ENTER, uinput.KEY_ESC]) as udevice:
						udevice.emit_click(uinput.KEY_ENTER)

			# d-pad
			if event.type == evdev.ecodes.EV_ABS:
				if event.code == 17:
					if event.value == -1:
						# d-pad up
						with uinput.Device([uinput.KEY_UP, uinput.KEY_DOWN]) as udevice:
							udevice.emit_click(uinput.KEY_UP)

					elif event.value == 1:
						# d-pad down
						with uinput.Device([uinput.KEY_UP, uinput.KEY_DOWN]) as udevice:
							udevice.emit_click(uinput.KEY_DOWN)

if __name__ == "__main__":
	asyncio.ensure_future(handle_event(evdev.InputDevice("/dev/input/event2")))

	loop = asyncio.get_event_loop()

	try:
		loop.run_forever()
	finally:
		loop.close()