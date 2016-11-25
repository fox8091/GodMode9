#include "filetype.h"
#include "ff.h"

u32 IdentifyFileType(const char* path) {
    u8 header[0x200]; // minimum required size
    FIL file;
    if (f_open(&file, path, FA_READ | FA_OPEN_EXISTING) != FR_OK)
        return 0;
    f_lseek(&file, 0);
    f_sync(&file);
    UINT fsize = f_size(&file);
    UINT bytes_read;
    if ((f_read(&file, header, 0x200, &bytes_read) != FR_OK) || (bytes_read != 0x200)) {
        f_close(&file);
        return 0;
    }
    f_close(&file);
    
    if ((getbe32(header + 0x100) == 0x4E435344) && (getbe64(header + 0x110) == (u64) 0x0104030301000000) &&
        (getbe64(header + 0x108) == (u64) 0) && (fsize >= 0x8FC8000)) {
        return IMG_NAND; // NAND image
    } else if (getbe16(header + 0x1FE) == 0x55AA) { // migt be FAT or MBR
        if ((strncmp((char*) header + 0x36, "FAT12   ", 8) == 0) || (strncmp((char*) header + 0x36, "FAT16   ", 8) == 0) ||
            (strncmp((char*) header + 0x36, "FAT     ", 8) == 0) || (strncmp((char*) header + 0x52, "FAT32   ", 8) == 0)) {
            return IMG_FAT; // this is an actual FAT header
        } else if (((getle32(header + 0x1BE + 0x8) + getle32(header + 0x1BE + 0xC)) < (fsize / 0x200)) && // check file size
            (getle32(header + 0x1BE + 0x8) > 0) && (getle32(header + 0x1BE + 0xC) >= 0x800) && // check first partition sanity
            ((header[0x1BE + 0x4] == 0x1) || (header[0x1BE + 0x4] == 0x4) || (header[0x1BE + 0x4] == 0x6) || // filesystem type
             (header[0x1BE + 0x4] == 0xB) || (header[0x1BE + 0x4] == 0xC) || (header[0x1BE + 0x4] == 0xE))) {
            return IMG_FAT; // this might be an MBR -> give it the benefit of doubt
        }
    } // more to come
    
    return 0;
}
