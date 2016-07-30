// TransformersChecksum.cpp : Defines the entry point for the console application.
// REVISION 1.1

#include <iostream> //I/O
#include <fstream>  //File I/O
#include <intrin.h> //Win32 only, endian functions


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

//lol it is working now! 0x0000BEEF is for global checksum!!

int main(int argc, char* argv[])
{
	
	//File object to read ROM from disk
	//Used to be an ifstrem but now it's a fstream to both write and read
	std::ifstream InternalmemStream;
	//Open file by its end (EOF) so we'll know it's exact size
	InternalmemStream.open("internalmem.dat", std::ios::in | std::ios::binary | std::ios::ate);
	//Check
	if (!InternalmemStream.is_open())
	{
		std::cout << "[!] Can't Open internalmem.dat";
		exit(EXIT_FAILURE);
	}

	//Temporal buffer to store the rom before copying to emulated memory
	char * r12mem;
	//Get rom size
	std::streampos size = InternalmemStream.tellg();
	std::cout << "[>] Internal Memory Size: " << size << " bytes" << std::endl;
	//Allocate memory for it
	r12mem = new char[size];
	//Seek to the start of the file
	InternalmemStream.seekg(0, std::ios::beg);
	//Read the ROM
	InternalmemStream.read(r12mem, size);
	std::cout << "[>] Internal Memory Loaded" << std::endl;
	InternalmemStream.close();
	
	

	std::fstream saveStream;
	saveStream.open("Transformers.sav", std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
	if (!saveStream.is_open())
	{
		std::cout << "[!] Can't Open Transformers.sav";
		exit(EXIT_FAILURE);
	}

	char * memblock2;
	std::streampos size2 = saveStream.tellg();//Since we opened the file by it's end we can retrieve it's size
	std::cout << "[>] Savegame Size: " << size2 << " bytes" << std::endl;
	memblock2 = new char[size2];
	saveStream.seekg(0, std::ios::beg);//The we just seek (move) to the begining of the file (0x0)
	saveStream.read(memblock2, size2);
	//We are not closing the stream since we'll use it to write 

	unsigned int check1, check2, check3;

	inicio:
	system("cls");
	int opt;
	std::cout << " ##### Transformers Checksum Tool #####" << std::endl << std::endl;
	std::cout << "1. Calc Slot 1 checksum" << std::endl;
	std::cout << "2. Calc Slot 2 checksum" << std::endl;
	std::cout << "3. Calc File checksum" << std::endl;
	std::cout << "4. Save changes and exit" << std::endl << std::endl;
	std::cout << "> ";
	std::cin >> opt; std::cout << std::endl;

	switch (opt)
	{
	case 1:
		check1  = CalcSlot1Checksum(memblock2, r12mem);//Get checksum in little endian. Must be writen to file in big endian
		saveStream.seekg(0xFB);//Checksum 1 pos
		saveStream.write(reinterpret_cast<const char *>(&check1), sizeof(check1)); //Yep correct way of doing this
		std::cout << check1;
		std::cin.get();
		std::cin.get();
		break;
	case 2:
		check2 = CalcSlot2Checksum(memblock2, r12mem);
		saveStream.seekg(0x1E0);//Checksum 2 pos
		saveStream.write(reinterpret_cast<const char *>(&check2), sizeof(check2)); //Yep correct way of doing this
		std::cin.get();
		break;
	case 3:
		check3 = CalcFileChecksum(memblock2, r12mem);
		saveStream.seekg(0x1FE);//Checksum 3 pos
		saveStream.write(reinterpret_cast<const char *>(&check3), sizeof(check3)); //Yep correct way of doing this
		break;
	case 4:
		saveStream.close();
		exit(EXIT_SUCCESS);
		break;
	}
	goto inicio;

	
	
	

	//DATA
	//0x0000BEEF -> File Checksum seed - Counter: 0x1FD -r1 file start   OK
	//0x00000000 -> Slot 2 Checksum seed - Counter: 0xDF - r1  [.].!?New Game   OK  WE NEED TO POINT TO [] byte  0x100
	//0X00000000 -> Slot 1 Checksum seed - Counter: 0xDF - r1  [.]'..AAAAAAAB   OK  WE NEED TO POINT TO [] byte  0x1C

	
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

		//COOL PATCH TO CORRECTLY FIX/LOAD R3 ADDRESSES
		r3 = r3 & 0x000000FF;



		if (r2 == 0){ breaker = true; }
		r2 = r2 - 1;

		r3 = r3 ^ (arithmeticRightShift(r0, 0x8));  //GOOD UNTIL HERE

	
		r3 = r3 << 1;        //Yay! //GOOD UNTIL HERE



		if (r3 >= 0x2710){
			std::cout << "R2: " << std::hex << r2 << std::endl;
			std::cout << "call to r12 mem very big: " << r3;
			std::cin.get();
		}
		//YEEEEAH IT WORRKS! gets byte at r3, merges it with byte at r3+1
		r3 = ((unsigned char)r12mem[r3] << 8) | (unsigned char)r12mem[r3 + 1]; //forget this -> It's ok but compiler fucks it when using [r3+1] In addition you have to use little endian bitch
		r3 = _byteswap_ushort(r3); //Since nds is little endian and my PC uses big endian I'll convert it when reading from memory
		//Unsigned int is 2 bytes -> 16 bits -> ushort

		
		r0 = r3 ^ (r0 << 0x8); //Yay! r3 = 0x00004084 this xor is the one that increments r0
		
		r0 = r0 << 0x10;
		
		r0 = logicalRightShift(r0, 0x10);
		

		if (breaker == true){ break; }
		//std::cin.get();
	}
	std::cout << "Slot 1 Checksum(r0): " << std::hex << r0 << std::endl;

	//Actually checksum is returned as little endian
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

		r3 = memblock2[r1]; r1++; //Where r1 is address where savedata is 0x0 for slot 1  and 0x00000100 for slot 2 ||THIS FAILS||

		//COOL PATCH TO CORRECTLY FIX/LOAD R3 ADDRESSES
		r3 = r3 & 0x000000FF;



		if (r2 == 0){ breaker = true; }
		r2 = r2 - 1;

		r3 = r3 ^ (arithmeticRightShift(r0, 0x8));  //GOOD UNTIL HERE


		r3 = r3 << 1;        //Yay! //GOOD UNTIL HERE



		if (r3 >= 0x2710){
			std::cout << "R2: " << std::hex << r2 << std::endl;
			std::cout << "call to r12 mem very big: " << r3;
			std::cin.get();
		}
		//YEEEEAH IT WORRKS! gets byte at r3, merges it with byte at r3+1
		r3 = ((unsigned char)r12mem[r3] << 8) | (unsigned char)r12mem[r3 + 1]; //forget this -> It's ok but compiler fucks it when using [r3+1] In addition you have to use little endian bitch
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

		r3 = memblock2[r1]; r1++; //Where r1 is address where savedata is 0x0 for slot 1  and 0x00000100 for slot 2 ||THIS FAILS||

		//COOL PATCH TO CORRECTLY FIX/LOAD R3 ADDRESSES
		r3 = r3 & 0x000000FF;


		if (r2 == 0){ breaker = true; }
		r2 = r2 - 1;

		r3 = r3 ^ (arithmeticRightShift(r0, 0x8));  //GOOD UNTIL HERE


		r3 = r3 << 1;        //Yay! //GOOD UNTIL HERE



		if (r3 >= 0x2710){
			std::cout << "R2: " << std::hex << r2 << std::endl;
			std::cout << "call to r12 mem very big: " << r3;
			std::cin.get();
		}
		//YEEEEAH IT WORRKS! gets byte at r3, merges it with byte at r3+1
		r3 = ((unsigned char)r12mem[r3] << 8) | (unsigned char)r12mem[r3 + 1]; //forget this -> It's ok but compiler fucks it when using [r3+1] In addition you have to use little endian bitch
		r3 = _byteswap_ushort(r3); //Since nds is little endian and my PC uses big endian I'll convert it when reading from memory



		r0 = r3 ^ (r0 << 0x8); //Yay! r3 = 0x00004084 this xor is the one that increments r0

		r0 = r0 << 0x10;

		r0 = logicalRightShift(r0, 0x10);


		if (breaker == true){ break; }
		//std::cin.get();
	}
	std::cout << "File Checksum(r0): " << std::hex << r0 << std::endl;

	std::cin.get();
	return r0;
}