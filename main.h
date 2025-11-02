#ifndef SO2025_FAT16FILESYSTEM_MAIN_H
#define SO2025_FAT16FILESYSTEM_MAIN_H
#include <cstdint>
#include <iosfwd>

using namespace std;


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

struct DirectoryEntry {
    char     name[11];
    uint8_t  attr;
    uint8_t  reserved;
    uint8_t  createTimeTenths;
    uint16_t createTime;
    uint16_t createDate;
    uint16_t lastAccessDate;
    uint16_t clusterHigh;
    uint16_t lastWriteTime;
    uint16_t lastWriteDate;
    uint16_t firstCluster;
    uint32_t fileSize;
};

bool readBootSector(ifstream& img, BootSector& boot);
void printBootInfo(const BootSector& boot);
void listRootDirectory(ifstream& disk, const BootSector& boot);
void evokeMenu();

#endif //SO2025_FAT16FILESYSTEM_MAIN_H