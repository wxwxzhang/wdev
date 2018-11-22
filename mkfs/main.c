/*
	妈个比, 连工具都还要自己写, allwinner的狗屎代码
*/



#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include "ff.h"

#ifdef USE_WL
#include "diskio_wl.h"
#else
#include "diskio_std.h"
#endif

#define SIZE   (SSIZE*SCOUNT)
#define LABEL   "web"
#define MAX_PATH 4096

static char *root=NULL;
///////////////////////////////////////////////////
#define CHK(a,b) printf("___%s, %s\n", a, my_err[b]);
#define DOTDIR(a) ((a[0]=='.' && a[1]==0) || (a[0]=='.' && a[1]=='.' && a[2]==0))


typedef struct {
    char    *info;
}err_t;

static char* my_err[]={
    "Succeeded",                                                                        //FR_OK,                     
    "A hard error occurred in the low level disk I/O layer",                            //FR_DISK_ERR,               
    "Assertion failed (check for corruption)",                                          //FR_INT_ERR,                    
    "The physical drive cannot work",                                                   //FR_NOT_READY,                          
    "Could not find the file",                                                          //FR_NO_FILE,                                
    "Could not find the path",                                                          //FR_NO_PATH,                
    "The path name format is invalid",                                                  //FR_INVALID_NAME,           
    "Access denied due to prohibited access or directory full",                         //FR_DENIED,                             
    "Destination file already exists",                                                  //FR_EXIST,                  
    "The file/directory object is invalid",                                             //FR_INVALID_OBJECT,                 
    "The physical drive is write protected",                                            //FR_WRITE_PROTECTED,                
    "The logical drive number is invalid",                                              //FR_INVALID_DRIVE,                      
    "The volume has no work area",                                                      //FR_NOT_ENABLED,                                    
    "There is no valid FAT volume",                                                     //FR_NO_FILESYSTEM,                      
    "f_mkfs() aborted due to a parameter error. Try adjusting the partition size.",     //FR_MKFS_ABORTED,                               
    "Could not get a grant to access the volume within defined period",                 //FR_TIMEOUT,                        
    "The operation is rejected according to the file sharing policy",                   //FR_LOCKED,                                 
    "LFN working buffer could not be allocated",                                        //FR_NOT_ENOUGH_CORE,                        
    "Number of open files > _FS_SHARE",                                                 //FR_TOO_MANY_OPEN_FILES,                                            
    "Invalid parameter",                                                                //FR_INVALID_PARAMETER,          
};


static int f_copy(char *path, int l)
{
    FIL fp;
    FILE *fd;
    FRESULT fr;
    char *buf;
    DIR *dir;
    char *rootdir=NULL;
    char tmp[MAX_PATH];
    struct stat st;
    struct dirent *ent=NULL;
    
    if(!path || !l || !path[0]) {
        return -1;
    }
    
    stat(path, &st);
    if(S_ISDIR(st.st_mode)) {
        if(path[l]) {
            f_mkdir(path+l);
            //printf("___mkdir %s\n", path+l);
        }
            
        dir = opendir(path);
        if(dir) {
            while((ent=readdir(dir))) {
                if(!DOTDIR(ent->d_name)) {
                    snprintf(tmp, sizeof tmp, "%s/%s", path, ent->d_name);  
                    f_copy(tmp, l);
                }
            }
            
            closedir(dir);
        }
    }
    else {
        fd = fopen(path, "rb");
        if(fd) {
            buf = (char *)malloc(st.st_size);
            fread(buf, 1, st.st_size, fd);
            fclose(fd);
            
            //printf("___copy file: %s\n", path+l);
            fr = f_open (&fp, path+l, FA_CREATE_NEW|FA_WRITE);
            //CHK("f_open", fr); 
            if(fr==FR_OK) {
                int wl;
                //printf("____ copy %s ...\n", dn.name);
                fr = f_write(&fp, buf, st.st_size, (UINT*)&wl);
                //CHK("f_write", fr);
                f_close(&fp);
            }
            
            free(buf);
        }
    }
}


static int f_scan(char *path)
{
    int l;
    char *fn;
    FDIR dir;
    FIL fp;
    FRESULT fr;
    FILINFO finfo;
    char npath[MAX_PATH];
    
    fn = finfo.fname;
    if (f_opendir(&dir,(const TCHAR*)path)==FR_OK) {
        while(f_readdir(&dir, &finfo)==FR_OK) {
            if (!fn[0]) break;  
            if (DOTDIR(fn)) continue;
            
            if (finfo.fattrib & AM_DIR) {
                l = strlen(path);
                if(path[l-1]=='/') {
                    sprintf(npath, "%s%s", path, fn);
                }
                else {
                    sprintf(npath, "%s/%s", path, fn);
                }
                f_scan(npath);
            }
        } 
        f_closedir(&dir);
    }    
    
    return 0;
}



static int mkfile(char *path, int len)
{
    #define BLEN 4096
    FILE *fd;
    char buf[BLEN];
    int  i,cnt,left;
    
    //remove(path);
    cnt = len/BLEN;
    left = len%BLEN;
    fd = fopen(path, "wb");
    if(!fd) {
        //printf("%s open failed\n");//strerror(errno));
        printf("%s open failed %s", strerror(errno));
        return -1;
    }
    
    for(i=0;i<cnt;i++) {
        fwrite(buf, 1, BLEN, fd);
    }
    fwrite(buf, 1, left, fd);
    fclose(fd);
    
    return 0;
}

static int f_mkimg()
{
    int fd;
    char *dir;
    FATFS fs;
    FRESULT fr;
    BYTE work[FF_MAX_SS];
    
    mkfile(IMAGE, SIZE);
    
    CHK("mkfs", f_mkfs("", FM_ANY | FM_SFD, SSIZE, work, sizeof work));

    CHK("mount", f_mount(&fs, IMAGE, 0));
    CHK("setlabel", f_setlabel(LABEL));
    dir = "./tmp";
    f_copy(dir, strlen(dir));
    CHK("unmount", f_unmount(IMAGE));
    
    return 0;
}


static int f_print()
{
    FATFS fs;
    FRESULT fr;
    
    CHK("mount", f_mount(&fs, IMAGE, 0));
    f_scan("/");
    CHK("unmount", f_unmount(IMAGE));
    
    return 0;
}


/////////////////////////////////////////
static esp_partition_t my_part={
    .type = ESP_PARTITION_TYPE_DATA,
    .subtype = ESP_PARTITION_SUBTYPE_DATA_FAT,
    .address = 0x200000,
    .size = 0x200000,
    .label = "web",
    .encrypted = 0,
};

#ifdef USE_WL
static wl_handle_t wl;
static int wl_init()
{
    esp_err_t r;
    BYTE d=0;
    char drv[3] = {(char)('0'+d), ':', 0};
    
    r = wl_mount(&my_part, &wl);
    diskio_wl_reg(d, wl);
    
    return 0;
}

static int wl_free()
{
    wl_unmount(wl);
    
    return 0;
}
#endif



int main(int argc, char **argv)
{
    int ch;

#if 0    
    while((ch = getopt(argc, argv, "ab:c:de::")) != -1) {
        printf("optind: %d\n", optind);
        switch(ch) {
            case 'a':
            printf("HAVE option: -a\n\n");   
            break;
            
            case 'b':
            printf("HAVE option: -b\n"); 
            printf("The argument of -b is %s\n\n", optarg);
            break;
            
            case 'c':
            printf("HAVE option: -c\n");
            printf("The argument of -c is %s\n\n", optarg);
            break;
           
            case 'd':
            printf("HAVE option: -d\n");
            break;
        }
    }
#endif

#ifdef USE_WL
    wl_init();
#endif

    f_mkimg();
    
 #ifdef USE_WL
    wl_free();
#endif   
    
    f_print();
    
    return 0;
}

