#include <cstdint>
#include <fstream>
#include "main.h"

using namespace  std;

streamoff calcRootDirOffset(const BootSector& boot) {
    return streamoff(boot.reservedSectors + boot.numFATs * boot.FATSize) * boot.bytesPerSector;
}

streamoff calcFirstDataSector(const BootSector& boot) {
    return streamoff(calcRootDirOffset(boot) + boot.rootEntries * sizeof(DirectoryEntry));
}

streamoff calcClusterOffset(const BootSector& boot, uint16_t cluster) {
    return streamoff(calcFirstDataSector(boot) + (cluster - 2) * boot.sectorsPerCluster * boot.bytesPerSector);
}

streamoff calcFATOffset(const BootSector& boot) {
    return streamoff(boot.reservedSectors * boot.bytesPerSector);
}

uint16_t encodeFATTime(const tm& t) {
    return ((t.tm_hour & 0x1F) << 11) |
           ((t.tm_min & 0x3F) << 5)  |
           ((t.tm_sec / 2) & 0x1F);
}

uint16_t encodeFATDate(const tm& t) {
    return (((t.tm_year - 80) & 0x7F) << 9) |
           ((t.tm_mon + 1) << 5) |
           (t.tm_mday & 0x1F);
}