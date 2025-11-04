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

uint16_t readFATEntry(fstream& disk, const BootSector& boot, uint16_t cluster) { // evaluate
    streamoff fatOffset = calcFATOffset(boot) + cluster * 2;
    disk.seekg(fatOffset, ios::beg);
    uint16_t nextCluster;
    disk.read(reinterpret_cast<char*>(&nextCluster), sizeof(nextCluster));
    return nextCluster;
}