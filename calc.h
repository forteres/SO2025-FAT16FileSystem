#ifndef SO2025_FAT16FILESYSTEM_CALC_H
#define SO2025_FAT16FILESYSTEM_CALC_H

streamoff calcRootDirOffset(const BootSector& boot);
streamoff calcFirstDataSector(const BootSector& boot);
streamoff calcClusterOffset(const BootSector& boot, uint16_t cluster);
streamoff calcFATOffset(const BootSector& boot);
uint16_t encodeFATTime(const tm& t);
uint16_t encodeFATDate(const tm& t);

#endif //SO2025_FAT16FILESYSTEM_CALC_H