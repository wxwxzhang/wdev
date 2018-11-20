/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2016        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include "ffconf.h"
#include "diskio.h"		/* FatFs lower layer API */

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */



/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return RES_OK;
}




/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/
int fno=-1;

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
    if (fno == -1)
        fno = open(IMAGE, O_SYNC|O_RDWR, S_IRUSR|S_IWUSR);
        
    lseek(fno, 0, SEEK_SET);
    return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res;
	int r;

	//printf("fno: %d, sector: %ld count: %d\n", fno, sector, count);
	lseek(fno, sector*SECTOR_SZ, SEEK_SET);
	r = read(fno, buff, count*SECTOR_SZ);

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res;
	int result;
	//printf("disk_write %d %ld %d\n", fno, sector, count);	
	lseek(fno, sector*SECTOR_SZ, SEEK_SET);
	write(fno, buff, count*SECTOR_SZ);
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	switch(cmd) {
	    case CTRL_SYNC:
	    break;
	    
	    case GET_SECTOR_COUNT:
	    *(DWORD*)buff = SECTOR_CNT;
	    break;
	    
	    case GET_SECTOR_SIZE:
	    *(WORD*)buff = SECTOR_SZ;
	    break;
	    
	    case GET_BLOCK_SIZE:
	    *(WORD*)buff = BLOCK_SZ;
	    break;
	    
	    default:
	    return RES_PARERR;
	}
	
	return RES_OK;
}


DWORD get_fattime() { return 0; };

void disk_flush() {
}