// Transformers Checksum Patcher (c) by pedro-javierf
// Rev 1.3.1

#include <iostream> //I/O
#include <fstream>  //File I/O

#define SWAP_UINT16(x) (((x) >> 8) | ((x) << 8))



unsigned int CalcSlotChecksum(unsigned int r1Init, char * memblock2, char * r12mem);
unsigned int CalcFileChecksum(char * memblock2, char * r12mem);
int logicalRightShift(int x, int n);
int arithmeticRightShift(int x, int n);
int menu(std::fstream &saveFile, char *memoryBlock, char *r12mem);

int main(int argc, char* argv[])
{
	std::string internalMemFile = "internalmem.dat";
	std::string savegameFile = "Transformers.sav";
	std::ifstream InternalmemStream;
	//Open file by its end (EOF) so we'll know it's exact size
	InternalmemStream.open(internalMemFile, std::ios::in | std::ios::binary | std::ios::ate);
	if (!InternalmemStream.is_open())
	{
		std::cout << "[!] Can't Open " << internalMemFile.c_str() << std::endl;
	}
	else
	{
		char * r12mem;
		std::streampos size = InternalmemStream.tellg();
		std::cout << "[>] Internal Memory Size: " << size << " bytes" << std::endl;
		//Allocate memory for it
		r12mem = new char[size];
		//Seek to the start of the file
		InternalmemStream.seekg(0, std::ios::beg);
		InternalmemStream.read(r12mem, size);
		std::cout << "[>] Internal Memory Loaded" << std::endl;
		InternalmemStream.close();


		std::fstream saveStream;
		saveStream.open(savegameFile, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
		if (!saveStream.is_open())
		{
			std::cout << "[!] Can't Open Transformers.sav";
		}
		else
		{
			char * memblock2;
			std::streampos size2 = saveStream.tellg();//Since we opened the file by it's end we can retrieve it's size
			std::cout << "[>] Savegame Size: " << size2 << " bytes" << std::endl;
			memblock2 = new char[size2];
			saveStream.seekg(0, std::ios::beg);//The we just seek (move) to the begining of the file (0x0)
			saveStream.read(memblock2, size2);
			//We are not closing the stream since we'll use it to write 

			int res = menu(saveStream, memblock2, r12mem);
		}

		InternalmemStream.close();		
	}
	
}

unsigned int CalcSlotChecksum(unsigned int r1Init, char * memblock2, char * r12mem)
{

	//Simulates registers
	unsigned int r0 = 0x00000000; //Seed. Will also store final checksum
	unsigned int r1 = r1Init; //Memory Location where data to be checksumed starts
	unsigned int r2 = 0x000000DF; //Counter
	unsigned int r3 = 0x00000000; //Stores the byte pointed by r1 to perform operations on it

	bool isDone = false;

	while (!isDone) //Simulate loop to calculate crc
	{

		r3 = memblock2[r1]; r1++; //Where r1 is address where savedata is 0x0 for slot 1  and 0x00000100 for slot 2 ||THIS FAILS||
		r3 = r3 & 0x000000FF;
		if (r2 == 0) { isDone = true; }
		r2 = r2 - 1;
		r3 = r3 ^ (arithmeticRightShift(r0, 0x8));  
		r3 = r3 << 1;

		if (r3 >= 0x2710) {
			std::cout << "R2: " << std::hex << r2 << std::endl;
			std::cout << "call to r12 mem very big: " << r3;
			std::cin.get();
		}
		//Gets byte at r3, merges it with byte at r3+1
		//Since nds is little endian and x86 uses big endian I'll convert it when reading from memory
		//ushort = 2 bytes -> unsigned 16 bits
		r3 = ((unsigned char)r12mem[r3] << 8) | (unsigned char)r12mem[r3 + 1]; 
		r3 = _byteswap_ushort(r3); 
		
		r0 = r3 ^ (r0 << 0x8); 
		r0 = r0 << 0x10;
		r0 = logicalRightShift(r0, 0x10);
	}

	std::cout << "Slot 2 CheckSum: " << std::hex << r0 << std::endl;
	return r0;
}


unsigned int CalcFileChecksum(char * memblock2, char * r12mem)
{

	//Simulates registers
	unsigned int r0 = 0x0000BEEF; //Seed. Will also store final checksum
	unsigned int r1 = 0x00000000; //Memory Location where data to be checksumed starts
	unsigned int r2 = 0x000001FD; //Counter
	unsigned int r3 = 0x00000000; //Stores the byte pointed by r1 to perform operations on it

	bool isDone = false;

	while (!isDone) //Simulate loop to calculate crc
	{

		r3 = memblock2[r1]; r1++; //Where r1 is address where savedata is 0x0 for slot 1  and 0x00000100 for slot 2 
		r3 = r3 & 0x000000FF;

		if (r2 == 0){ isDone = true; }
		r2 = r2 - 1;
		r3 = r3 ^ (arithmeticRightShift(r0, 0x8));  
		r3 = r3 << 1;

		if (r3 >= 0x2710){
			std::cout << "R2: " << std::hex << r2 << std::endl;
			std::cout << "call to r12 mem very big: " << r3;
			std::cin.get();
		}
		//Gets byte at r3, merges it with byte at r3+1
		r3 = ((unsigned char)r12mem[r3] << 8) | (unsigned char)r12mem[r3 + 1];
		r3 = SWAP_UINT16(r3); //r3 = _byteswap_ushort(r3); //Since nds is little endian and my PC uses big endian I'll convert it when reading from memory

		r0 = r3 ^ (r0 << 0x8); //Yay! r3 = 0x00004084 this xor is the one that increments r0
		r0 = r0 << 0x10;
		r0 = logicalRightShift(r0, 0x10);
		
	}
	std::cout << "File Checksum: "<< std::hex << r0 << std::endl;
	return r0;
}



//By JohnScipione
int logicalRightShift(int x, int n) {
	return (unsigned)x >> n;
}

int arithmeticRightShift(int x, int n) {
	if (x < 0 && n > 0)
		return x >> n | ~(~0U >> n);
	else
		return x >> n;
}

int menu(std::fstream &saveFile, char *memoryBlock, char *r12mem)
{
	unsigned int check = 0;
	int opt = -1;
	std::cout << " ##### Transformers Checksum Tool #####" << std::endl << std::endl;

	while (opt != 0)
	{
		std::cout << "1. Calc Slot 1 checksum" << std::endl;
		std::cout << "2. Calc Slot 2 checksum" << std::endl;
		std::cout << "3. Calc File checksum" << std::endl;
		std::cout << "0. Save file and exit" << std::endl;
		std::cout << "> ";
		std::cin >> opt; 


		switch (opt)
		{
		case 1:
			check = CalcSlotChecksum(0x0000001C, memoryBlock, r12mem);//Get checksum in little endian. Must be writen to file in big endian
			saveFile.seekg(0xFB);//Checksum 1 pos
			saveFile.write(reinterpret_cast<const char *>(&check), sizeof(check)); 
			std::cout << check << " patched" << std::endl;
			break;
		case 2:
			check = CalcSlotChecksum(0x00000100, memoryBlock, r12mem);
			saveFile.seekg(0x1E0);//Checksum 2 pos
			saveFile.write(reinterpret_cast<const char *>(&check), sizeof(check));
			std::cout << check << " patched" << std::endl;
			break;
		case 3:
			check = CalcFileChecksum(memoryBlock, r12mem);
			saveFile.seekg(0x1FE);//Checksum 3 pos
			saveFile.write(reinterpret_cast<const char *>(&check), sizeof(check));
			std::cout << "Whole file patched. Please save file now." << std::endl;
			break;
		case 0:
			saveFile.close();
			break;
		default:
			std::cout << "Invalid Option." << std::endl;
			break;
		}
		std::cin.ignore();
	}
	return opt;
}
