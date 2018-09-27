/*---------------------------------------------------------------------------/
/  FatFs - FAT file system module include file  R0.09b    (C)ChaN, 2013
/----------------------------------------------------------------------------/
/ FatFs module is a generic FAT file system module for small embedded systems.
/ This is a free software that opened for education, research and commercial
/ developments under license policy of following terms.
/
/  Copyright (C) 2013, ChaN, all right reserved.
/
/ * The FatFs module is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   personal, non-profit or commercial product UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/----------------------------------------------------------------------------*/

#ifndef _FATFS
#define _FATFS	82786	/* Revision ID */

#ifdef __cplusplus
extern "C" {
#endif

#include "integer.h"	/* Basic integer types */
#include "ffconf.h"		/* FatFs configuration options */

#if _FATFS != _FFCONF
#error Wrong configuration file (ffconf.h).
#endif



/* Definitions of volume management */

#if _MULTI_PARTITION		/* Multiple partition configuration */
typedef struct {
	BYTE pd;	/* Physical drive number */
	BYTE pt;	/* Partition: 0:Auto detect, 1-4:Forced partition) */
} PARTITION;
extern PARTITION VolToPart[];	/* Volume - Partition resolution table */
#define LD2PD(vol) (VolToPart[vol].pd)	/* Get physical drive number */
#define LD2PT(vol) (VolToPart[vol].pt)	/* Get partition index */

#else							/* Single partition configuration */
#define LD2PD(vol) (BYTE)(vol)	/* Each logical drive is bound to the same physical drive number */
#define LD2PT(vol) 0			/* Always mounts the 1st partition or in SFD */

#endif



/* Type of path name strings on FatFs API */

#if _LFN_UNICODE			/* Unicode string */
#if !_USE_LFN
#error _LFN_UNICODE must be 0 in non-LFN cfg.
#endif
#ifndef _INC_TCHAR
typedef WCHAR TCHAR;
#define _T(x) L ## x
#define _TEXT(x) L ## x
#endif

#else						/* ANSI/OEM string */
#ifndef _INC_TCHAR
typedef char TCHAR;
#define _T(x) x
#define _TEXT(x) x
#endif

#endif



/* File system object structure (FATFS) */
/* 文件系统对象结构体 */
typedef struct {
	BYTE	fs_type;		/* FAT sub-type (0:Not mounted, FS_FAT12/FS_FAT16/FS_FAT32) */
							/* FAT子类型，一般在mount时用，置0表示未挂载,FS_FAT12/FS_FAT16/FS_FAT32 */
	BYTE	drv;			/* Physical drive number */
							/* 物理驱动号，一般为0，SD卡 */
	BYTE	csize;			/* Sectors per cluster (1,2,4...128) */
							/* 每个簇包含的扇区数(1,2,4...128) */
	BYTE	n_fats;			/* Number of FAT copies (1,2) */
							/* 文件分配表的数目（1， 2） */
	BYTE	wflag;			/* win[] dirty flag (1:must be written back) */
							/* 标记文件是否被改动过，为1时要回写*/
	BYTE	fsi_flag;		/* fsinfo dirty flag (1:must be written back) */
							/* 标记文件系统信息是否被改动过，为1时要回写*/
	WORD	id;				/* File system mount ID */
							/* 文件系统挂载ID */
	WORD	n_rootdir;		/* Number of root directory entries (FAT12/16) */
							/* 根目录区入口(目录项)的个数(用于FAT12/16)*/
#if _MAX_SS != 512
	WORD	ssize;			/* Bytes per sector (512, 1024, 2048 or 4096) */
							/* 每扇区的字节数(用于扇区大于512Byte的flash) */
#endif
#if _FS_REENTRANT
	_SYNC_t	sobj;			/* Identifier of sync object */
							/* 允许重入，即定义同步对象，用在tiny中*/
#endif
#if !_FS_READONLY
	DWORD	last_clust;		/* Last allocated cluster */
							/* 最后一个被分配的簇*/
	DWORD	free_clust;		/* Number of free clusters */
							/* 空闲簇的个数*/
	DWORD	fsi_sector;		/* fsinfo sector (FAT32) */
							/* 存放fsinfo的扇区(用于FAT32) */
#endif
#if _FS_RPATH
	DWORD	cdir;			/* Current directory start cluster (0:root) */
							/* 允许相对路径时用，存储当前目录起始簇(0:root)*/
#endif
	DWORD	n_fatent;		/* Number of FAT entries (= number of clusters + 2) */
							/* FAT入口数(簇的数目 + 2)*/
	DWORD	fsize;			/* Sectors per FAT */
							/* 每个FAT所占扇区*/
	DWORD	volbase;		/* Volume start sector */
							 
	DWORD	fatbase;		/* FAT start sector */
							/* FAT起始扇区*/
	DWORD	dirbase;		/* Root directory start sector (FAT32:Cluster#) */
							/* 根目录起始扇区(FAT32:Cluster#) */
	DWORD	database;		/* Data start sector */
							/* 数据目录起始扇区*/
	DWORD	winsect;		/* Current sector appearing in the win[] */
							/* 当前缓冲区中存储的扇区号*/
	BYTE	win[_MAX_SS];	/* Disk access window for Directory, FAT (and Data on tiny cfg) */
							/* 单个扇区缓存*/
} FATFS;



/* File object structure (FIL) */
/* 文件对象结构体 */
typedef struct {
	FATFS*	fs;				/* Pointer to the related file system object (**do not change order**) */
							/* 文件所在的fs指针 */
	WORD	id;				/* Owner file system mount ID (**do not change order**) */
							/* 文件所在的Fs挂载号 */
	BYTE	flag;			/* File status flags */
							/* 文件的状态 */
	BYTE	pad1;
	DWORD	fptr;			/* File read/write pointer (0ed on file open) */
							/* 文件的读写指针 */
	DWORD	fsize;			/* File size */
							/* 文件大小 */
	DWORD	sclust;			/* File data start cluster (0:no data cluster, always 0 when fsize is 0) */
							/* 文件起始簇 */
	DWORD	clust;			/* Current cluster of fpter */
							/* 当前簇 */
	DWORD	dsect;			/* Current data sector of fpter */
							/* 当前数据扇区 */
#if !_FS_READONLY
	DWORD	dir_sect;		/* Sector containing the directory entry */
							/*包含目录项的扇区 */
	BYTE*	dir_ptr;		/* Pointer to the directory entry in the window */
							/* 目录入口指针 */
#endif
#if _USE_FASTSEEK
	DWORD*	cltbl;			/* Pointer to the cluster link map table (null on file open) */
							/* 指向簇链接映射表的指针 */
#endif
#if _FS_LOCK
	UINT	lockid;			/* File lock ID (index of file semaphore table Files[]) */
#endif
#if !_FS_TINY
	BYTE	buf[_MAX_SS];	/* File data read/write buffer */
							/* 文件读写缓存 */
#endif
} FIL;



/* Directory object structure (DIR) */
/* 目录对象结构体 */
typedef struct {
	FATFS*	fs;				/* Pointer to the owner file system object (**do not change order**) */
							/* 所在的fs指针 */
	WORD	id;				/* Owner file system mount ID (**do not change order**) */
							/* 所在的fs挂在编号 */
	WORD	index;			/* Current read/write index number */
							/* 当前读写索引号 */
	DWORD	sclust;			/* Table start cluster (0:Root dir) */
							/* 文件数据区起始簇 */
	DWORD	clust;			/* Current cluster */
							/* 当前簇 */
	DWORD	sect;			/* Current sector */
							/* 当前扇区 */
	BYTE*	dir;			/* Pointer to the current SFN entry in the win[] */
							/* 扇区缓存中当前SFN入口指针，SFN含义未知，猜测和LFN类似，与文件名相关 */
							
	BYTE*	fn;				/* Pointer to the SFN (in/out) {file[8],ext[3],status[1]} */
#if _USE_LFN
	WCHAR*	lfn;			/* Pointer to the LFN working buffer */
	WORD	lfn_idx;		/* Last matched LFN index number (0xFFFF:No LFN) */
#endif
} DIR;



/* File status structure (FILINFO) */
/* 文件状态结构体 */
typedef struct {
	DWORD	fsize;			/* File size */
	WORD	fdate;			/* Last modified date */
	WORD	ftime;			/* Last modified time */
	BYTE	fattrib;		/* Attribute */
	TCHAR	fname[13];		/* Short file name (8.3 format) */
#if _USE_LFN
	TCHAR*	lfname;			/* Pointer to the LFN buffer */
	UINT 	lfsize;			/* Size of LFN buffer in TCHAR */
#endif
} FILINFO;



/* File function return code (FRESULT) */

typedef enum {
	FR_OK = 0,				/* (0) Succeeded */
	FR_DISK_ERR,			/* (1) A hard error occurred in the low level disk I/O layer */
	FR_INT_ERR,				/* (2) Assertion failed */
	FR_NOT_READY,			/* (3) The physical drive cannot work */
	FR_NO_FILE,				/* (4) Could not find the file */
	FR_NO_PATH,				/* (5) Could not find the path */
	FR_INVALID_NAME,		/* (6) The path name format is invalid */
	FR_DENIED,				/* (7) Access denied due to prohibited access or directory full */
	FR_EXIST,				/* (8) Access denied due to prohibited access */
	FR_INVALID_OBJECT,		/* (9) The file/directory object is invalid */
	FR_WRITE_PROTECTED,		/* (10) The physical drive is write protected */
	FR_INVALID_DRIVE,		/* (11) The logical drive number is invalid */
	FR_NOT_ENABLED,			/* (12) The volume has no work area */
	FR_NO_FILESYSTEM,		/* (13) There is no valid FAT volume */
	FR_MKFS_ABORTED,		/* (14) The f_mkfs() aborted due to any parameter error */
	FR_TIMEOUT,				/* (15) Could not get a grant to access the volume within defined period */
	FR_LOCKED,				/* (16) The operation is rejected according to the file sharing policy */
	FR_NOT_ENOUGH_CORE,		/* (17) LFN working buffer could not be allocated */
	FR_TOO_MANY_OPEN_FILES,	/* (18) Number of open files > _FS_SHARE */
	FR_INVALID_PARAMETER	/* (19) Given parameter is invalid */
} FRESULT;



/*--------------------------------------------------------------*/
/* FatFs module application interface                           */

FRESULT f_mount (BYTE vol, FATFS* fs);								/* Mount/Unmount a logical drive */
FRESULT f_open (FIL* fp, const TCHAR* path, BYTE mode);				/* Open or create a file */
FRESULT f_read (FIL* fp, void* buff, UINT btr, UINT* br);			/* Read data from a file */
FRESULT f_lseek (FIL* fp, DWORD ofs);								/* Move file pointer of a file object */
FRESULT f_close (FIL* fp);											/* Close an open file object */
FRESULT f_opendir (DIR* dj, const TCHAR* path);						/* Open an existing directory */
FRESULT f_readdir (DIR* dj, FILINFO* fno);							/* Read a directory item */
FRESULT f_stat (const TCHAR* path, FILINFO* fno);					/* Get file status */
FRESULT f_write (FIL* fp, const void* buff, UINT btw, UINT* bw);	/* Write data to a file */
FRESULT f_getfree (const TCHAR* path, DWORD* nclst, FATFS** fatfs);	/* Get number of free clusters on the drive */
FRESULT f_truncate (FIL* fp);										/* Truncate file */
FRESULT f_sync (FIL* fp);											/* Flush cached data of a writing file */
FRESULT f_unlink (const TCHAR* path);								/* Delete an existing file or directory */
FRESULT f_mkdir (const TCHAR* path);								/* Create a new directory */
FRESULT f_chmod (const TCHAR* path, BYTE value, BYTE mask);			/* Change attribute of the file/dir */
FRESULT f_utime (const TCHAR* path, const FILINFO* fno);			/* Change times-tamp of the file/dir */
FRESULT f_rename (const TCHAR* path_old, const TCHAR* path_new);	/* Rename/Move a file or directory */
FRESULT f_chdrive (BYTE drv);										/* Change current drive */
FRESULT f_chdir (const TCHAR* path);								/* Change current directory */
FRESULT f_getcwd (TCHAR* buff, UINT len);							/* Get current directory */
FRESULT f_getlabel (const TCHAR* path, TCHAR* label, DWORD* sn);	/* Get volume label */
FRESULT f_setlabel (const TCHAR* label);							/* Set volume label */
FRESULT f_forward (FIL* fp, UINT(*func)(const BYTE*,UINT), UINT btf, UINT* bf);	/* Forward data to the stream */
FRESULT f_mkfs (BYTE vol, BYTE sfd, UINT au);						/* Create a file system on the drive */
FRESULT f_fdisk (BYTE pdrv, const DWORD szt[], void* work);			/* Divide a physical drive into some partitions */
int f_putc (TCHAR c, FIL* fp);										/* Put a character to the file */
int f_puts (const TCHAR* str, FIL* cp);								/* Put a string to the file */
int f_printf (FIL* fp, const TCHAR* str, ...);						/* Put a formatted string to the file */
TCHAR* f_gets (TCHAR* buff, int len, FIL* fp);						/* Get a string from the file */

#define f_eof(fp) (((fp)->fptr == (fp)->fsize) ? 1 : 0)
#define f_error(fp) (((fp)->flag & FA__ERROR) ? 1 : 0)
#define f_tell(fp) ((fp)->fptr)
#define f_size(fp) ((fp)->fsize)

#ifndef EOF
#define EOF (-1)
#endif




/*--------------------------------------------------------------*/
/* Additional user defined functions                            */

/* RTC function */
#if !_FS_READONLY
DWORD get_fattime (void);
#endif

/* Unicode support functions */
#if _USE_LFN							/* Unicode - OEM code conversion */
WCHAR ff_convert (WCHAR chr, UINT dir);	/* OEM-Unicode bidirectional conversion */
WCHAR ff_wtoupper (WCHAR chr);			/* Unicode upper-case conversion */
#if _USE_LFN == 3						/* Memory functions */
void* ff_memalloc (UINT msize);			/* Allocate memory block */
void ff_memfree (void* mblock);			/* Free memory block */
#endif
#endif

/* Sync functions */
#if _FS_REENTRANT
int ff_cre_syncobj (BYTE vol, _SYNC_t* sobj);	/* Create a sync object */
int ff_req_grant (_SYNC_t sobj);				/* Lock sync object */
void ff_rel_grant (_SYNC_t sobj);				/* Unlock sync object */
int ff_del_syncobj (_SYNC_t sobj);				/* Delete a sync object */
#endif




/*--------------------------------------------------------------*/
/* Flags and offset address                                     */


/* File access control and file status flags (FIL.flag) */

#define	FA_READ				0x01		/* 指定读访问对象 */
#define	FA_OPEN_EXISTING	0x00
#define FA__ERROR			0x80

#if !_FS_READONLY
#define	FA_WRITE			0x02		/* 指定写访问对象， 与FA_READ结合可以读写访问 */	
#define	FA_CREATE_NEW		0x04		/* 创建一个新文件，如果文件已经存在则创建失败 */
#define	FA_CREATE_ALWAYS	0x08		/* 创建一个新文件，如果文件已经存在则它被截断并覆盖 */
#define	FA_OPEN_ALWAYS		0x10		/* 打开文件如果文件不存在，则创建一个新文件， 此种方式
										   可以使用f_lseek函数对新打开的文件追加数据*/
#define FA__WRITTEN			0x20
#define FA__DIRTY			0x40
#endif


/* FAT sub type (FATFS.fs_type) */

#define FS_FAT12	1
#define FS_FAT16	2
#define FS_FAT32	3


/* File attribute bits for directory entry */

#define	AM_RDO	0x01	/* Read only */
#define	AM_HID	0x02	/* Hidden */
#define	AM_SYS	0x04	/* System */
#define	AM_VOL	0x08	/* Volume label */
#define AM_LFN	0x0F	/* LFN entry */
#define AM_DIR	0x10	/* Directory */
#define AM_ARC	0x20	/* Archive */
#define AM_MASK	0x3F	/* Mask of defined bits */


/* Fast seek feature */
#define CREATE_LINKMAP	0xFFFFFFFF



/*--------------------------------*/
/* Multi-byte word access macros  */

#if _WORD_ACCESS == 1	/* Enable word access to the FAT structure */
#define	LD_WORD(ptr)		(WORD)(*(WORD*)(BYTE*)(ptr))
#define	LD_DWORD(ptr)		(DWORD)(*(DWORD*)(BYTE*)(ptr))
#define	ST_WORD(ptr,val)	*(WORD*)(BYTE*)(ptr)=(WORD)(val)
#define	ST_DWORD(ptr,val)	*(DWORD*)(BYTE*)(ptr)=(DWORD)(val)
#else					/* Use byte-by-byte access to the FAT structure */
#define	LD_WORD(ptr)		(WORD)(((WORD)*((BYTE*)(ptr)+1)<<8)|(WORD)*(BYTE*)(ptr))
#define	LD_DWORD(ptr)		(DWORD)(((DWORD)*((BYTE*)(ptr)+3)<<24)|((DWORD)*((BYTE*)(ptr)+2)<<16)|((WORD)*((BYTE*)(ptr)+1)<<8)|*(BYTE*)(ptr))
#define	ST_WORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *((BYTE*)(ptr)+1)=(BYTE)((WORD)(val)>>8)
#define	ST_DWORD(ptr,val)	*(BYTE*)(ptr)=(BYTE)(val); *((BYTE*)(ptr)+1)=(BYTE)((WORD)(val)>>8); *((BYTE*)(ptr)+2)=(BYTE)((DWORD)(val)>>16); *((BYTE*)(ptr)+3)=(BYTE)((DWORD)(val)>>24)
#endif

#ifdef __cplusplus
}
#endif

#endif /* _FATFS */
