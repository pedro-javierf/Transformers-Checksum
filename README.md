# Transformers-Checksum
C++ tool to calculate savegame checksums from the game Transformers for the Nintendo DS.
The tool was created from reverse engineering the game, and is actually a prototype, even it
has complete functionality.

## Included Files
- raworiginal.s: Original assembly code from the game
- main.c: C++ code to calculate the checksum

## TODO
- Make tool portable (little endian snippet)
- Improve calculation code (move from close-to-asm emulation to efficient emulation)
- Add file reload (to calculate file global checksum)
- Improve general code (full comments, better memory management..)
- Delete single functions
- Create reliable C++ code
- Support for decepticons and EUR/JAP versions


## Licence
    Copyright (C) 2016  pedro-javierf
	
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
