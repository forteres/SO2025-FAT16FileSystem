#ifndef SO2025_FAT16FILESYSTEM_MAIN_H
#define SO2025_FAT16FILESYSTEM_MAIN_H
#include <cstdint>
#include <iosfwd>
#include <vector>

using namespace std;

#pragma pack(push, 1)
struct BootSector {
    uint8_t  jumpInstruction[3]; // 00 |3|
    char     OEMName[8]; // 03 |8|
    uint16_t bytesPerSector; // 0B |2|
    uint8_t  sectorsPerCluster; // 0D |1|
    uint16_t reservedSectors; // 0E |2|
    uint8_t  numFATs; // 10 |1|
    uint16_t rootEntries; // 11 |2|
    uint16_t totalSectors16; // Alas smallNumberOfSectors 13 |2|
    uint8_t  mediaDescriptor; // 15 |1|
    uint16_t FATSize; // Alas sectorsPerFat 16 |2|
    uint16_t sectorsPerTrack; // 18 |2|
    uint16_t numHeads; // 1A |2|
    uint32_t hiddenSectors; // 1C |4|
    uint32_t totalSectors32; // Alas largeNumberOfSectors 20 |4|
    // Source: http://www.maverick-os.dk/FileSystemFormats/FAT16_FileSystem.html
};
#pragma pack(pop)

#pragma pack(push, 1)
struct DirectoryEntry {
    char     name[11]; // 00 |8| + 08 |3|
    uint8_t  attr; // 0B |1|
    uint8_t  reserved; // 0C |1|
    uint8_t  createTimeTenths; // 0D |1|
    uint16_t createTime; // 0E |2|
    uint16_t createDate; // 10 |2|
    uint16_t lastAccessDate; // 12 |2|
    uint16_t clusterHigh; // 14 |2|
    uint16_t lastWriteTime; // 16 |2|
    uint16_t lastWriteDate; // 18 |2|
    uint16_t firstCluster; // 1A |2|
    uint32_t fileSize; // 1C |4|
    // Source: http://www.maverick-os.dk/FileSystemFormats/FAT16_FileSystem.html
};
#pragma pack(pop)

bool readBootSector(fstream& disk, BootSector& boot);
void printBootInfo(const BootSector& boot);
bool readRootDirectory(fstream& disk, const BootSector& boot, vector<DirectoryEntry>& rootDirectoryEntries);
void printRootDirectory(const vector<DirectoryEntry>& entries);
bool readFat16Name(char name[11]);
uint16_t findFile(const vector<DirectoryEntry>& entries, const char* name);
uint16_t readFATEntry(fstream& disk, const BootSector& boot, uint16_t cluster);
void listFileContent(fstream& disk, const BootSector& boot, vector<DirectoryEntry>& entries);
void listAttributes(vector<DirectoryEntry> entries);
void printFileAttributes(const DirectoryEntry& entry);
void renameFile(fstream& disk, const BootSector& boot, vector<DirectoryEntry>& entries);
void deleteFile(fstream& disk, const BootSector& boot, vector<DirectoryEntry>& entries);

uint16_t findFreeEntry(const vector<DirectoryEntry>& entries);
uint16_t findFreeCluster(fstream& disk, const BootSector& boot);
void writeFATEntry(fstream& disk, const BootSector& boot, uint16_t cluster, uint16_t value);
void insertFile(fstream& disk, const BootSector& boot, vector<DirectoryEntry>& entries);

void evokeMenu();

#endif //SO2025_FAT16FILESYSTEM_MAIN_H