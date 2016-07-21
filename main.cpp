// Transformers-Checksum
// REVISION 1.1

/*
Copyright (C) 2016 pedro-javierf

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
*/

#include <iostream> //I/O
#include <fstream>  //File I/O
#include <intrin.h> //Win32 only


//By JohnScipione, Thanks!
int logicalRightShift(int x, int n) {
	return (unsigned)x >> n;
}
int arithmeticRightShift(int x, int n) {
	if (x < 0 && n > 0)
		return x >> n | ~(~0U >> n);
	else
		return x >> n;
}

unsigned int CalcSlot1Checksum(char * memblock2, char * r12mem);
unsigned int CalcSlot2Checksum(char * memblock2, char * r12mem);
unsigned int CalcFileChecksum(char * memblock2, char * r12mem);

int main(int argc, char* argv[])
{
	
	//File object to read ROM from disk
	std::ifstream reader;
	//Open rom by its end (EOF) so we'll know it's exact size
	reader.open("R12.dat", std::ios::in | std::ios::binary | std::ios::ate);
	//Check
	if (!reader.is_open())
	{
		std::cout << " Can't Open R12.dat";
		exit(EXIT_FAILURE);
	}

	//Temporal buffer to store the rom before copying to emulated memory
	char * r12mem;
	//Get rom size
	std::streampos size = reader.tellg();
	std::cout << "R12 Size: " << size << " bytes" << std::endl;
	//Allocate memory for it
	r12mem = new char[size];
	//Seek to the start of the file
	reader.seekg(0, std::ios::beg);
	//Read the ROM
	reader.read(r12mem, size);
	std::cout << "R12 Loaded" << std::endl;

	
	

	std::ifstream reader2;
	reader2.open("Transformers.sav", std::ios::in | std::ios::binary | std::ios::ate);
	if (!reader2.is_open())
	{
		std::cout << " Can't Open Transformers.sav";
		exit(EXIT_FAILURE);
	}

	char * memblock2;
	std::streampos size2 = reader2.tellg();
	std::cout << "Savegame Size: " << size2 << " bytes" << std::endl;
	memblock2 = new char[size2];
	reader2.seekg(0, std::ios::beg);
	reader2.read(memblock2, size2);

	inicio:
	system("cls");
	int opt;
	std::cout << " ##### Transformers Checksum Tool #####" << std::endl << std::endl;
	std::cout << "1. Calc Slot 1 checksum" << std::endl;
	std::cout << "2. Calc Slot 2 checksum" << std::endl;
	std::cout << "3. Calc File checksum" << std::endl << std::endl;
	std::cout << "> ";
	std::cin >> opt; std::cout << std::endl;

	switch (opt)
	{
	case 1:
		CalcSlot1Checksum(memblock2, r12mem);
		std::cin.get();
		break;
	case 2:
		CalcSlot2Checksum(memblock2, r12mem);
		std::cin.get();
		break;
	case 3:
		CalcFileChecksum(memblock2, r12mem);
		std::cin.get();
		break;
	}
	goto inicio;

	
	
	

	//DATA
	//0x0000BEEF -> File Checksum seed - Counter: 0x1FD -r1 file start   OK
	//0x00000000 -> Slot 2 Checksum seed - Counter: 0xDF - r1  [.].!?New Game   OK  WE NEED TO POINT TO [] byte  0x100
	//0X00000000 -> Slot 1 Checksum seed - Counter: 0xDF - r1  [.]'..AAAAAAAB   OK  WE NEED TO POINT TO [] byte  0x1C

	return 0;
	
}

unsigned int CalcSlot1Checksum(char * memblock2, char * r12mem)
{

	//Simulates registers
	unsigned int r0 = 0x00000000; //Seed. Will also store final checksum
	unsigned int r1 = 0x0000001C; //Memory Location 
	unsigned int r2 = 0x000000DF; //Counter
	unsigned int r3 = 0x00000000; //Unknown, maybe a temporal register? stores the character pointed by r1!!

	
	bool breaker = false;

	while (1) //Simulate loop to calculate crc
	{

		r3 = memblock2[r1]; r1++; //Where r1 is address where savedata is 0x0 for slot 1  and 0x00000100 for slot 2 ||THIS FAILS||

		//THIS IS A LITTLE FIX//
		//SOMETIME THE CODE ABOVE LOADS THE BYTE AT R3 LIKE 0xFFFFFFAA instead of 0x00000AA so this dirty patch will solve that
		r3 = r3 << 0x18;
		r3 = logicalRightShift(r3, 0x18);
		//UPDATE WITH AND


		if (r2 == 0){ breaker = true; }
		r2 = r2 - 1;

		r3 = r3 ^ (arithmeticRightShift(r0, 0x8)); 

	
		r3 = r3 << 1;



		if (r3 >= 0x2710){
			std::cout << "R2: " << std::hex << r2 << std::endl;
			std::cout << "call to r12 mem very big: " << r3;
			std::cin.get();
		}
		//gets byte at r3, merges it with byte at r3+1
		r3 = ((unsigned char)r12mem[r3] << 8) | (unsigned char)r12mem[r3 + 1]; 
		r3 = _byteswap_ushort(r3); //Since nds is little endian and my PC uses big endian I'll convert it when reading from memory


		
		r0 = r3 ^ (r0 << 0x8); 
		
		r0 = r0 << 0x10;
		
		r0 = logicalRightShift(r0, 0x10);
		

		if (breaker == true){ break; }
	}
	std::cout << "Slot 1 Checksum(r0): " << std::hex << r0 << std::endl;

	std::cin.get();
	return r0;
}

unsigned int CalcSlot2Checksum(char * memblock2, char * r12mem)
{

	//Simulates registers
	unsigned int r0 = 0x00000000; //Seed. Will also store final checksum
	unsigned int r1 = 0x00000100; //Memory Location where data to be checksumed starts
	unsigned int r2 = 0x000000DF; //Counter
	unsigned int r3 = 0x00000000; //Stores the byte pointed by r1 to perform operations on it


	bool breaker = false;

	while (1) //Simulate loop to calculate crc
	{

		r3 = memblock2[r1]; r1++; //Where r1 is address where savedata is 0x0 for slot 1  and 0x00000100 for slot 2

		//THIS IS A LITTLE FIX//
		//SOMETIME THE CODE ABOVE LOADS THE BYTE AT R3 LIKE 0xFFFFFFAA instead of 0x00000AA so this dirty patch will solve that
		r3 = r3 << 0x18;
		r3 = logicalRightShift(r3, 0x18);



		if (r2 == 0){ breaker = true; }
		r2 = r2 - 1;

		r3 = r3 ^ (arithmeticRightShift(r0, 0x8));  


		r3 = r3 << 1;   



		if (r3 >= 0x2710){
			std::cout << "R2: " << std::hex << r2 << std::endl;
			std::cout << "call to r12 mem very big: " << r3;
			std::cin.get();
		}
		// gets byte at r3, merges it with byte at r3+1
		r3 = ((unsigned char)r12mem[r3] << 8) | (unsigned char)r12mem[r3 + 1]; 
		r3 = _byteswap_ushort(r3); //Since nds is little endian and my PC uses big endian I'll convert it when reading from memory



		r0 = r3 ^ (r0 << 0x8); //Yay! r3 = 0x00004084 this xor is the one that increments r0

		r0 = r0 << 0x10;

		r0 = logicalRightShift(r0, 0x10);


		if (breaker == true){ break; }
		//std::cin.get();
	}
	std::cout << "Slot 2 Checksum(r0): " << std::hex << r0 << std::endl;

	std::cin.get();
	return r0;
}

unsigned int CalcFileChecksum(char * memblock2, char * r12mem)
{

	//Simulates registers
	unsigned int r0 = 0x0000BEEF; //Seed. Will also store final checksum
	unsigned int r1 = 0x00000000; //Memory Location where data to be checksumed starts
	unsigned int r2 = 0x000001FD; //Counter
	unsigned int r3 = 0x00000000; //Stores the byte pointed by r1 to perform operations on it


	bool breaker = false;

	while (1) //Simulate loop to calculate crc
	{

		r3 = memblock2[r1]; r1++; //Where r1 is address where savedata is 0x0 for slot 1  and 0x00000100 for slot 2 

		//THIS IS A LITTLE FIX//
		//SOMETIME THE CODE ABOVE LOADS THE BYTE AT R3 LIKE 0xFFFFFFAA instead of 0x00000AA so this dirty patch will solve that
		r3 = r3 << 0x18;
		r3 = logicalRightShift(r3, 0x18);



		if (r2 == 0){ breaker = true; }
		r2 = r2 - 1;

		r3 = r3 ^ (arithmeticRightShift(r0, 0x8));  


		r3 = r3 << 1;        



		if (r3 >= 0x2710){
			std::cout << "R2: " << std::hex << r2 << std::endl;
			std::cout << "call to r12 mem very big: " << r3;
			std::cin.get();
		}
		//gets byte at r3, merges it with byte at r3+1
		r3 = ((unsigned char)r12mem[r3] << 8) | (unsigned char)r12mem[r3 + 1]; 
		r3 = _byteswap_ushort(r3); //Since nds is little endian and my PC uses big endian I'll convert it when reading from memory



		r0 = r3 ^ (r0 << 0x8); 

		r0 = r0 << 0x10;

		r0 = logicalRightShift(r0, 0x10);


		if (breaker == true){ break; }
	}
	std::cout << "File Checksum(r0): " << std::hex << r0 << std::endl;

	std::cin.get();
	return r0;
}