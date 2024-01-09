/**
 * Copyright:   2023, XT Tech. Co., Ltd.
 * File name:   main.c
 * Description: main模块实现
 * Author:      张海涛
 * Version:     0.0.0.1
 * Code:        UTF-8(无BOM)
 * Date:        2023-12-25
 * History:     2023-12-25 创建此文件。
 */

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <Windows.h>
#include <CommCtrl.h>

#ifndef bool
#define bool                    unsigned char
#define true                    1
#define false                   0
#endif

#define SIZEOF(x)               sizeof(x)/sizeof(x[0])
#define SP(...)                 _stprintf_s(txt, SIZEOF(txt), __VA_ARGS__)

#define FAT_SIZE                256     // FAT数量
#define MFT_SIZE                128     // MFT数量
#define MIRR_SIZE               4       // MIRR数量
#define MFT_ATTR_SIZE           8       // 每个MFT记录的属性数量
#define ATTR_LIST_SIZE          8       // 20属性列表,列表项数量
#define DIR_SIZE                32      // 目录项数量
#define UPCASE_SIZE             256     // 大小写映射项数量
#define BITMAP_SIZE             512     // 位图项数量
#define CLUSTER_SIZE            512     // 簇内数据数量
#define RUNDATA_SIZE            128     // 簇运行数量
#define INDEX_ROOT_ENTRY_SIZE   8       // 索引根中的索引项数量
#define INDEX_ALLOC_SIZE        4       // 索引分配数量
#define INDEX_ALLOC_ENTRY_SIZE  32      // 索引分配中的索引项数量

TCHAR *bus_type_text[]  = { _T("Unknown"),
                            _T("SCSI"),
                            _T("ATAPI"),
                            _T("ATA"),
                            _T("IEEE1394"),
                            _T("SSA"),
                            _T("Fibre Channel"),
                            _T("USB"),
                            _T("RAID"),
                            _T("iScsi"),
                            _T("Sas"),
                            _T("Sata"),
                            _T("Sd"),
                            _T("Mmc"),
                            _T("Max")};

TCHAR *media_type_text[] ={ _T("Unknown"),
                            _T("F5_1Pt2_512"),
                            _T("F3_1Pt44_512"),
                            _T("F3_2Pt88_512"),
                            _T("F3_20Pt8_512"),
                            _T("F3_720_512"),
                            _T("F5_360_512"),
                            _T("F5_320_512"),
                            _T("F5_320_1024"),
                            _T("F5_180_512"),
                            _T("F5_160_512"),
                            _T("RemovableMedia"),
                            _T("FixedMedia    "),
                            _T("F3_120M_512"),
                            _T("F3_640_512"),
                            _T("F5_640_512"),
                            _T("F5_720_512"),
                            _T("F3_1Pt2_512"),
                            _T("F3_1Pt23_1024"),
                            _T("F5_1Pt23_1024"),
                            _T("F3_128Mb_512"),
                            _T("F3_230Mb_512"),
                            _T("F8_256_128"),
                            _T("F3_200Mb_512"),
                            _T("F3_240M_512"),
                            _T("F3_32M_512")};

TCHAR *mft_file_text[]  = { _T("$MFT"),
                            _T("$MFTMirr"), // 备份MFT的前4个表项
                            _T("$LogFile"),
                            _T("$Volume"),
                            _T("$AttrDef"),
                            _T("$Root"),
                            _T("$Bitmap"),
                            _T("$Boot"),
                            _T("$BadClus"),
                            _T("$Secure"),
                            _T("$UpCase"),
                            _T("$Extend"),
                            _T("$Extend\\$Reparse"),
                            _T("$Extend\\$UsnJrnl"),
                            _T("$Extend\\$Quota"),
                            _T("$Extend\\$ObjId")};

TCHAR *mft_attr_text[]  = { _T(""),
                            _T("标准属性"),
                            _T("属性列表"),
                            _T("文件名属性"),
                            _T("对象ID属性"),
                            _T("安全描述符属性"),
                            _T("卷名属性"),
                            _T("卷信息属性"),
                            _T("文件数据属性"),
                            _T("索引根属性"),
                            _T("索引项属性"),
                            _T("位图属性"),
                            _T("重解析点属性"),
                            _T("扩展信息属性"),
                            _T("EA扩展属性"),
                            _T("EA扩展属性"),
                            _T("EFS加密属性")};

TCHAR *part_type_text[] = { _T(""),
                            _T("exFAT"), // 原来为FAT12
                            _T("XENIX root"),
                            _T("XENIX usr"),
                            _T("FAT16<32MB"),
                            _T("扩展分区"),
                            _T("FAT16>32MB"),
                            _T("HPFS/NTFS"),
                            _T("AIX"),
                            _T("AIX bootable"),
                            _T("OS/2 Boot Manage"),
                            _T("FAT32"),
                            _T("FAT32隐藏"),
                            _T(""),
                            _T("FAT16"),
                            _T("扩展分区>8GB"),
                            _T("OPUS"),
                            _T("FAT12隐藏"),
                            _T("Compaq diagnost"),
                            _T(""),
                            _T("FAT16<32MB隐藏"),
                            _T(""),
                            _T("FAT16>32MB隐藏"),
                            _T("HPFS/NTFS隐藏")};

#pragma pack (push, 1)

typedef struct _fat16_dbr                           // FAT16的DBR数据
{
    unsigned char           jump[3];                // 跳转指令EB 58 90
    unsigned char           oem[8];                 // 文件系统的ASIIC码:"NTFS,EXFAT,FAT32,FAT16"

    // BPB(BIOS Parameter Block)
    unsigned short          sector_byte_size;       // 每个扇区内的字节数量,一般为512
    unsigned char           cluster_sector_size;    // 每个簇占用的扇区数量,一般为8
    unsigned short          reserve_sector_count;   // 保留扇区数
    unsigned char           fat_count;              // FAT数量
    unsigned short          root_entries_count;     // 根目录项数
    unsigned short          small_sector_count;     // 小分区扇区数
    unsigned char           media_des;              // 介质描述:F0-软盘,F8-硬盘
    unsigned short          fat_sector_size;        // FAT表大小
    unsigned short          trark_sector_count;     // 每道扇区数
    unsigned short          disk_header_count;      // 磁头数
    unsigned int            hidden_count;           // 隐含扇区(从MBR到DBR的扇区总数)
    unsigned int            large_sector_count;     // 大分区扇区数

    // BPB extend
    unsigned char           physical_disk_type;     // 磁盘类型:00-软盘,80-硬盘
    unsigned char           reserved2;              // 保留
    unsigned char           extended_boot_signature;// 0x28 or 0x29
    unsigned int            volume_serial_number;   // 卷号
    unsigned char           volume_label[11];       // "NO NAME    "
    unsigned char           system_type[8];         // "FAT16   "

    // code
    unsigned char           code[448];              // 引导程序代码

    // sig
    unsigned short          sig;                    // 0xAA55

} fat16_dbr, *p_fat16_dbr;

typedef struct _fat16_dir_short                     // FAT16的短文件名目录项
{
    unsigned char           file[8];                // 文件名,第一个字节为0xE5,表示该项已被删除
    unsigned char           ext[3];                 // 扩展名
    unsigned char           attr;                   // 属性:F-长文件名,1-只读,2-隐藏,4-系统,8-卷标,10-目录,20-归档
    unsigned char           reserve[10];            // 保留
    unsigned short          update_time;            // 文件最近修改时间
    unsigned short          update_date;            // 文件最近修改日期
    unsigned short          cluster_id;             // 文件的首簇号
    unsigned int            size;                   // 文件长度大小

} fat16_dir_short, *p_fat16_dir_short;

typedef struct _fat_dir_long                        // FAT16/32的长文件名目录项
{
    unsigned char           flag;                   // 40-最后一个目录项,0-1F序号
    unsigned short          filename1[5];           // UNICODE
    unsigned char           attr;                   // 属性:F-长文件名,1-只读,2-隐藏,4-系统,8-卷标,10-目录,20-归档
    unsigned char           reserve;                // 保留
    unsigned char           check;                  // 校验码,根据短文件名计算得出
    unsigned short          filename2[6];           // UNICODE
    unsigned short          cluster_id;             // 文件的首簇号
    unsigned short          filename3[2];           // UNICODE

} fat_dir_long, *p_fat_dir_long;

typedef union  _fat16_dir                           // FAT16目录项
{
    fat_dir_long            l;
    fat16_dir_short         s;

} fat16_dir, *p_fat16_dir;

typedef struct _fat16_info                          // FAT16信息
{
    unsigned __int64        pos_dbr;                // 硬盘中的扇区位置
    unsigned __int64        pos_dir;
    unsigned __int64        pos_fat1;
    unsigned __int64        pos_fat2;
    unsigned __int64        pos_cluster;

    fat16_dbr               dbr;

    unsigned short          fat1[FAT_SIZE];         // FAT数据:0未用,FFF0~6-系统,FFF7-坏簇,FFF8~F-结束

    unsigned short          fat2[FAT_SIZE];

    fat16_dir               dir[DIR_SIZE];          // 目录项,不由FAT管理

    unsigned char           cluster[DIR_SIZE][CLUSTER_SIZE];// 簇数据

} fat16_info, *p_fat16_info;

typedef struct _fat32_dbr                           // FAT32的DBR数据
{
    unsigned char           jump[3];                // 跳转指令EB 58 90
    unsigned char           oem[8];                 // 文件系统的ASIIC码:"NTFS,EXFAT,FAT32,FAT16"

    // BPB(BIOS Parameter Block)
    unsigned short          sector_byte_size;       // 每个扇区内的字节数量,一般为512
    unsigned char           cluster_sector_size;    // 每个簇占用的扇区数量,一般为8
    unsigned short          reserve_sector_count;   // 保留扇区数
    unsigned char           fat_count;              // FAT数量
    unsigned short          root_entries_count;     // FAT12/FAT16 only, Fat32=0
    unsigned short          small_sector_count;     // FAT12/FAT16 only, Fat32=0
    unsigned char           media_des;              // 介质描述:F0-软盘, F8-硬盘
    unsigned short          fat16_sector_size;      // FAT12/FAT16 only, Fat32=0, FAT表大小
    unsigned short          trark_sector_count;     // 每道扇区数
    unsigned short          disk_header_count;      // 磁头数
    unsigned int            hide_sector_count;      // 隐含扇区(从MBR到DBR的扇区总数)
    unsigned int            large_sector_count;     // 扇区总数,即分区大小

    // FAT32
    unsigned int            fat32_sector_size;      // FAT32表大小
    unsigned short          extern_flag;            // 扩展标记
    unsigned short          file_system_version;    // 文件系统版本
    unsigned int            root_cluster_id;        // 第一个目录的簇号,2
    unsigned short          file_info_sector_number;// 文件信息所在扇区,1
    unsigned short          backup_boot_pos;        // mbr备份,6
    unsigned char           reserved1[12];          // 保留

    // BPB extend
    unsigned char           physical_disk_type;     // 磁盘类型:00-软盘,80-硬盘
    unsigned char           reserved2;              // 保留
    unsigned char           extended_boot_signature;// 0x28 or 0x29
    unsigned int            volume_serial_number;   // 卷号
    unsigned char           volume_label[11];       // "NO NAME    "
    unsigned char           system_type[8];         // "FAT32   "

    // code
    unsigned char           code[420];              // 引导程序代码

    // sig
    unsigned short          sig;                    // 0xAA55

} fat32_dbr, *p_fat32_dbr;

typedef struct _fat32_fsinfo                        // FAT32的文件信息,记录系统中空闲簇的数量以及下一可用簇
{
    unsigned char           flag1[4];               // RRaA
    unsigned char           reserve1[480];          // 未使用,全部置0
    unsigned char           flag2[4];               // rrAa
    unsigned int            unused_count;           // 空簇数
    unsigned int            next;                   // 下一可用簇号
    unsigned char           reserve2[14];           // 未使用
    unsigned short          sig;                    // 0xAA55

} fat32_fsinfo, *p_fat32_fsinfo;

typedef struct _fat32_dir_short                     // FAT32的短文件名目录项
{
    unsigned char           file[8];                // 文件名,第一个字节为0xE5,表示该项已被删除
    unsigned char           ext[3];                 // 扩展名
    unsigned char           attr;                   // 属性:F-长文件名,1-只读,2-隐藏,4-系统,8-卷标,10-目录,20-归档
    unsigned char           reserve;                // 保留
    unsigned char           create_millisecond;     // 创建时间的10毫秒位
    unsigned short          create_time;            // 创建时间
    unsigned short          create_date;            // 创建日期
    unsigned short          access_date;            // 最后访问日期
    unsigned short          cluster_id_high;        // 文件的首簇号高16位
    unsigned short          update_time;            // 文件最近修改时间
    unsigned short          update_date;            // 文件最近修改日期
    unsigned short          cluster_id;             // 文件的首簇号低16位
    unsigned int            size;                   // 文件长度大小

} fat32_dir_short, *p_fat32_dir_short;

typedef union  _fat32_dir                           // FAT32目录项
{
    fat_dir_long            l;
    fat32_dir_short         s;

} fat32_dir, *p_fat32_dir;

typedef struct _fat32_info                          // FAT32信息
{
    unsigned __int64        pos_dbr;                // 硬盘中的扇区位置
    unsigned __int64        pos_dir;
    unsigned __int64        pos_fat1;
    unsigned __int64        pos_fat2;
    unsigned __int64        pos_fsinfo;
    unsigned __int64        pos_cluster;

    fat32_dbr               dbr;

    fat32_fsinfo            fsinfo;

    unsigned int            fat1[FAT_SIZE];         // FAT数据:0未用,FFF0~6-系统,FFF7-坏簇,FFF8~F-结束

    unsigned int            fat2[FAT_SIZE];

    unsigned char           cluster[DIR_SIZE][CLUSTER_SIZE];// 簇数据

    fat32_dir               dir[DIR_SIZE];          // 目录项,由FAT管理,与FAT16不同

} fat32_info, *p_fat32_info;

// EXFAT区域名称    扇区位置
//
// 主启动区域
// 主启动扇区       0
// 主扩展启动扇区   1
// 主要 OEM 参数    9
// 主保留           10
// 主启动校验和     11
//
// 备份启动区域
// 备份启动扇区     12
// 备份扩展启动扇区 13
// 备份 OEM 参数    21
// 备份保留         22
// 备份启动校验和   23
//
// FAT 区域
// 对齐             24
// 第一个 FAT       FatOffset
// 第二个 FAT       FatOffset + FatLength
//
// 数据区域
// 对齐             FatOffset + FatLength * NumberOfFats
// 簇堆             ClusterHeapOffset
// 多余的空间       ClusterHeapOffset + ClusterCount * 2^SectorsPerClusterShift

typedef struct _exfat_dbr                           // exFAT的DBR数据
{
    unsigned char           jump[3];                // 跳转指令EB 58 90
    unsigned char           oem[8];                 // 文件系统的ASIIC码:"NTFS,EXFAT,FAT32,FAT16"

    unsigned char           MustBeZero[53];         // FAT12/16/32的BIOS参数块

    unsigned __int64        PartitionOffset;        // 扇区偏移量
    unsigned __int64        VolumeLength;           // 卷的大小
    unsigned int            FatOffset;              // FAT扇区偏移量
    unsigned int            FatLength;              // FAT表的长度
    unsigned int            ClusterHeapOffset;      // 簇相对于卷的扇区偏移量
    unsigned int            ClusterCount;           // 簇包含的群集数,应<FFFFFFF7
    unsigned int            ClusterOfRootDirectory; // 根目录的第一个群集的群集索引
    unsigned int            VolumeSerialNumber;     // 卷标
    unsigned short          FileSystemRevision;     // 主要和次要修订号

    unsigned short          VolumeFlags_ActiveFat:1;    // 活动FAT号
    unsigned short          VolumeFlags_VolumeDirty:1;  // 卷不一致
    unsigned short          VolumeFlags_MediaFailure:1; // 媒体故障
    unsigned short          VolumeFlags_ClearToZero:1;  // 修改目录,文件前应置为0

    unsigned char           BytesPerSectorShift;    // 每个扇区的字节数
    unsigned char           SectorsPerClusterShift; // 每个簇的扇区数
    unsigned char           NumberOfFats;           // FAT数和分配位图数
    unsigned char           DriveSelect;            // 驱动器选择字段
    unsigned char           PercentInUse;           // 簇0~100%
    unsigned char           reserve[7];             // 保留

    // code
    unsigned char           BootCode[390];          // 代码

    // sig
    unsigned short          sig;                    // 0xAA55

} exfat_dbr, *p_exfat_dbr;

typedef struct _Timestamp                          // 时间戳
{
    unsigned int            DoubleSeconds:5;        // 2秒的倍数
    unsigned int            Minute:6;               // 0~59
    unsigned int            Hour:5;                 // 0~23
    unsigned int            Day:5;                  // 1~31
    unsigned int            Month:4;                // 1~12
    unsigned int            Year:7;                 // 1980年起始

} Timestamp, *p_Timestamp;

typedef struct _UtcOffset                          // UTC偏移
{
    unsigned char           OffsetFromUtc:7;        // 单位15分钟,等效带符号十进制,第1位为符号位,负数反码加1
    unsigned char           OffsetValid:1;          // 无效

} UtcOffset, *p_UtcOffset;

typedef struct _exfat_dir_guid                      // 卷GUID目录
{
    unsigned char           SecondaryCount;                     // 0
    unsigned short          SetChecksum;
    unsigned short          GeneralPrimaryFlags_UseCluster:1;   // 使用簇,0-未使用磁盘空间
    unsigned short          GeneralPrimaryFlags_NoFatChain:1;   // 不使用簇链
    unsigned char           VolumeGuid[16];

} exfat_dir_guid, *p_exfat_dir_guid;

typedef struct _exfat_dir_ext                       // 流扩展目录
{
    unsigned char           GeneralSecondaryFlags_UseCluster:1; // 1
    unsigned char           GeneralSecondaryFlags_NoFatChain:1;
    unsigned char           Reserved1;
    unsigned char           NameLength;                         // Unicode字符串的长度
    unsigned short          NameHash;
    unsigned char           Reserved2[2];
    unsigned __int64        ValidDataLength;
    unsigned char           Reserved3[4];
    unsigned int            FirstCluster;           // 簇号
    unsigned __int64        DataLength;             // 数据长

} exfat_dir_ext, *p_exfat_dir_ext;

typedef struct _exfat_dir_bitmap                    // 位图目录
{
    unsigned char           BitmapFlags_BitmapIdentifier:1;     // 0-FAT1中分配,1-FAT2中分配
    unsigned char           CustomDefined[18];
    unsigned int            FirstCluster;           // 簇号
    unsigned __int64        DataLength;             // 数据长

} exfat_dir_bitmap, *p_exfat_dir_bitmap;

typedef struct _exfat_dir_filename                  // 文件名目录
{
    unsigned char           GeneralSecondaryFlags_UseCluster:1; // 0
    unsigned char           GeneralSecondaryFlags_NoFatChain:1;
    unsigned short          FileName[15];

} exfat_dir_filename, *p_exfat_dir_filename;

typedef struct _exfat_dir_upcase                    // 大小写映射目录,WINDOWS系统目录不区分大小写
{
    unsigned char           Reserved1[3];
    unsigned int            TableChecksum;
    unsigned char           Reserved2[12];
    unsigned int            FirstCluster;           // 簇号
    unsigned __int64        DataLength;             // 数据长

} exfat_dir_upcase, *p_exfat_dir_upcase;

typedef struct _exfat_dir_volume_label              // 卷标目录
{
    unsigned char           CharacterCount;         // UNICODE字符串长
    unsigned short          VolumeLabel[11];

} exfat_dir_volume_label, *p_exfat_dir_volume_label;

typedef struct _exfat_dir_file                      // 文件目录
{
    unsigned char           SecondaryCount;         // 辅助目录条目数
    unsigned short          SetChecksum;            // 校验和
    unsigned short          FileAttributes_ReadOnly:1;
    unsigned short          FileAttributes_Hidden:1;
    unsigned short          FileAttributes_System:1;
    unsigned short          FileAttributes_Reserved1:1;
    unsigned short          FileAttributes_Directory:1;
    unsigned short          FileAttributes_Archive:1;// 归档
    unsigned short          Reserved1;
    Timestamp               CreateTimestamp;
    Timestamp               LastModifiedTimestamp;
    Timestamp               LastAccessedTimestamp;
    unsigned char           Create10msIncrement;    // 10 毫秒的倍数,0~199,补足Timestamp.DoubleSeconds
    unsigned char           LastModified10msIncrement;
    UtcOffset               CreateUtcOffset;
    UtcOffset               LastModifiedUtcOffset;
    UtcOffset               LastAccessedUtcOffset;
    unsigned char           Reserved2[7];

} exfat_dir_file, *p_exfat_dir_file;

typedef struct _exfat_dir                           // 目录,32字节
{
    unsigned char           TypeCode:5;             // 类型
    unsigned char           TypeImportance:1;       // 0-重要,1-次要
    unsigned char           TypeCategory:1;         // 0-主要,1-辅助
    unsigned char           TypeInUse:1;            // 使用中

    union {
    exfat_dir_guid          guid;
    exfat_dir_ext           ext;
    exfat_dir_bitmap        bitmap;
    exfat_dir_filename      filename;
    exfat_dir_upcase        upcase;
    exfat_dir_volume_label  label;
    exfat_dir_file          file;
    };

} exfat_dir, *p_exfat_dir;

typedef struct _exfat_info                          // EXFAT信息
{
    unsigned __int64        pos_dbr;                // 硬盘中的扇区位置
    unsigned __int64        pos_dir;
    unsigned __int64        pos_fat1;
    unsigned __int64        pos_fat2;
    unsigned __int64        pos_bitmap;
    unsigned __int64        pos_upcase;
    unsigned __int64        pos_cluster;

    exfat_dbr               dbr;

    unsigned int            fat1[FAT_SIZE];         // FAT数据:0未用,FFF0~6-系统,FFF7-坏簇,FFF8~F-结束

    unsigned int            fat2[FAT_SIZE];

    unsigned char           cluster[DIR_SIZE][CLUSTER_SIZE];// 簇数据

    unsigned char           bitmap[BITMAP_SIZE];    // 位图

    unsigned short          upcase[UPCASE_SIZE];    // 大小写映射

    exfat_dir               dir[DIR_SIZE];          // 目录项

} exfat_info, *p_exfat_info;

typedef struct _cluster_rundata_head                // 簇流数据头
{
    unsigned char           len:4;                  // 使用簇号占用大小
    unsigned char           beg:4;                  // 起始簇号占用大小

} cluster_rundata_head, *p_cluster_rundata_head;

typedef struct _ntfs_mft_attr_body_10               // 标准属性
{
    unsigned __int64        create_time;            // 文件创建时间
    unsigned __int64        update_time;            // 文件修改时间
    unsigned __int64        mft_up_time;            // MFT变化时间
    unsigned __int64        access_time;            // 文件访问时间

    unsigned int            attr_readonly:1;        // 文件属性,只读
    unsigned int            attr_hide:1;            // 文件属性,隐藏
    unsigned int            attr_system:1;          // 文件属性,系统
    unsigned int            attr_reservie:2;        // 文件属性
    unsigned int            attr_archive:1;         // 文件属性,存档
    unsigned int            attr_device:1;          // 文件属性,设备
    unsigned int            attr_convention:1;      // 文件属性,常规
    unsigned int            attr_temporary:1;       // 文件属性,临时
    unsigned int            attr_few:1;             // 文件属性,稀疏
    unsigned int            attr_reparse:1;         // 文件属性,重解析点
    unsigned int            attr_compress:1;        // 文件属性,压缩
    unsigned int            attr_offline:1;         // 文件属性,脱机
    unsigned int            attr_index:1;           // 文件属性,索引
    unsigned int            attr_encrypt:1;         // 文件属性,加密

    unsigned int            max_version;            // 文件所允许的最大版本号(0表示未使用)
    unsigned int            version;                // 文件的版本号(最在版本号为0, 则他为0)
    unsigned int            class_id;               // 类ID(一个双向的类索引)
    unsigned int            owner_id;               // 所有者ID(表示文件的所有者, 是文件配额$QUOTA中$O和$Q索引的关键字, 为0表示未使用磁盘配额)
    unsigned int            secure_id;              // 安全ID是文件$SECURE中$SII和$SDS数据流的关键字, 注意不要与安全标识相混淆
    unsigned __int64        size;                   // 本文件所占用的字节数, 它是文件所有流占用的总字节数，为0表示未使用磁盘配额
    unsigned __int64        usn;                    // 更新系列号(USN), 是到文件$USNJRNL的一个直接的索引，为0表示USN日志未使用

} ntfs_mft_attr_body_10, *p_ntfs_mft_attr_body_10;

typedef struct _ntfs_mft_attr_body_20_attr_list     // 属性列表项
{
    unsigned int            type;                   // 类型
    unsigned short          len;                    // 记录长度
    unsigned char           name_len;               // 属性名长度(N, 为0表示没有属性名)
    unsigned char           name_offset;            // 属性名偏移(如果没有属性名，则指向属性内容)
    unsigned __int64        begin_cluster;          // 起始VCN(属性常驻时为0)
    unsigned __int64        ref_id;                 // 属性的基本文件记录中的文件参考号(所有MFT的文件都有一个文件索引号，引用到这个文件参考号，等价于引用这个文件记录，这个参考号在文件记录头中有定义)
    unsigned short          id;                     // 属性ID(每个属性都有一个唯一的ID号)
    unsigned short          name[256];              // Unicode属性名(如果有属性名, 大小2N)

} ntfs_mft_attr_body_20_attr_list, *p_ntfs_mft_attr_body_20_attr_list;

typedef struct _ntfs_mft_attr_body_20               // 属性列表
{
    ntfs_mft_attr_body_20_attr_list  attr_list[ATTR_LIST_SIZE];

    unsigned char                    attr_list_count;

} ntfs_mft_attr_body_20, *p_ntfs_mft_attr_body_20;

typedef struct _ntfs_mft_attr_body_30               // 文件名
{
    unsigned __int64        father_mft_id:48;       // 父目录的文件参考号（即父目录的基本文件记录号,分为两部分,前6个字节48位为父目录的文件记录号,此处为0x05,即根目录,所以$MFT的父目录为根目录,后2个字节为序列号)
    unsigned __int64        seq_num:16;             // 序列号
    unsigned __int64        create_time;            // 文件创建时间
    unsigned __int64        update_time;            // 文件修改时间
    unsigned __int64        mft_up_time;            // 最后一次MFT更新时间
    unsigned __int64        access_time;            // 最后一次访问时间
    unsigned __int64        alloc_size;             // 文件分配大小
    unsigned __int64        actual_size;            // 文件实际大小
    unsigned int            attr_readonly:1;        // 文件属性,只读
    unsigned int            attr_hide:1;            // 文件属性,隐藏
    unsigned int            attr_system:1;          // 文件属性,系统
    unsigned int            attr_reservie:2;        // 文件属性
    unsigned int            attr_archive:1;         // 文件属性,存档
    unsigned int            attr_device:1;          // 文件属性,设备
    unsigned int            attr_convention:1;      // 文件属性,常规
    unsigned int            attr_temporary:1;       // 文件属性,临时
    unsigned int            attr_few:1;             // 文件属性,稀疏
    unsigned int            attr_reparse:1;         // 文件属性,重解析点
    unsigned int            attr_compress:1;        // 文件属性,压缩
    unsigned int            attr_offline:1;         // 文件属性,脱机
    unsigned int            attr_index:1;           // 文件属性,索引
    unsigned int            attr_encrypt:1;         // 文件属性,加密
    unsigned int            eas;                    // 用于EAS和重解析点
    unsigned char           filename_len;           // 以字符计的文件名长度
    unsigned char           namespace;              // 文件名命名空间:0-POSIX命名空间,1-Win32命名空间,2-DOS命名空间;3-Win32&DOS命名空间
    unsigned short          name[256];              // 以Unicode方式表示的文件名

} ntfs_mft_attr_body_30, *p_ntfs_mft_attr_body_30;

typedef struct _ntfs_mft_attr_body_40               // 对象ID
{
    unsigned __int64        object_id;              // 对象ID
    unsigned __int64        valume_id;              // 卷ID
    unsigned __int64        original_object_id;     // 原始对象ID
    unsigned __int64        domain_id;              // 域ID

} ntfs_mft_attr_body_40, *p_ntfs_mft_attr_body_40;

typedef struct _ntfs_mft_attr_body_50               // 安全描述符
{
    int a;

} ntfs_mft_attr_body_50, *p_ntfs_mft_attr_body_50;

typedef struct _ntfs_mft_attr_body_60               // 卷名
{
    unsigned short          volume_name[127];

} ntfs_mft_attr_body_60, *p_ntfs_mft_attr_body_60;

typedef struct _ntfs_mft_attr_body_70               // 卷信息
{
    unsigned char           reserve1[8];            // 为0
    unsigned char           major_version;          // 主版本号
    unsigned char           minor_version;          // 次版本号
    unsigned short          flag_bad:1;             // 标志,坏区
    unsigned short          flag_resize:1;          // 标志,调整日志文件大小
    unsigned short          flag_reload:1;          // 标志,更新装载
    unsigned short          flag_nt4:1;             // 标志,装载到NT4
    unsigned short          flag_usn:1;             // 标志,删除进行中的USN
    unsigned short          flag_lds:1;             // 标志,修复对象lds
    unsigned short          flag_reserve:9;         // 标志
    unsigned short          flag_chkdisk:1;         // 标志,用chkdisk修正
    unsigned char           reserve2[4];            // 为0

} ntfs_mft_attr_body_70, *p_ntfs_mft_attr_body_70;

typedef struct _ntfs_mft_attr_body_80               // 文件数据
{
    unsigned char           data[32];               // 常驻,属性体是数据内容,非常驻,属性体是簇流运行

} ntfs_mft_attr_body_80, *p_ntfs_mft_attr_body_80;

typedef struct _ntfs_mft_attr_index_root_head       // 索引根头
{
    unsigned int            type;                   // 索引的属性类型
    unsigned int            rule;                   // 分类规则
    unsigned int            size_byte;              // 每个索引记录的大小(字节数)
    unsigned char           size_cluster;           // 每个索引记录的大小簇数
    unsigned char           reserve[3];             // 未用

} ntfs_mft_attr_index_root_head, *p_ntfs_mft_attr_index_root_head;

typedef struct _ntfs_mft_attr_index_root_body       // 索引根体
{
    unsigned int            pos_head;               // 索引项列表起始偏移(相对于节点头起始处)
    unsigned int            pos_tail_used;          // 索引项列表已用部分的结尾偏移(相对于节点头起始处)
    unsigned int            pos_tail_buf;           // 索引项列表缓冲区的结尾偏移(相对于节点头起始处)
    unsigned int            flag;                   // 标志,如果有其它节点指向该节点,此值为0x01

} ntfs_mft_attr_index_root_body, *p_ntfs_mft_attr_index_root_body;

typedef struct _ntfs_mft_attr_index_entry           // 索引项
{
    unsigned __int64        mft_id:48;              // 目录索引中用于记录文件的MFT文件参考号
    unsigned __int64        seq_num:16;             // 序列号
    unsigned short          len;                    // 本索引项的长度(从索引项起始处)
    unsigned short          data_len;               // 内容长度(目录索引中用于记录文件名属性长度)
    unsigned short          flag_havechild:1;       // 标志:有子节点
    unsigned short          flag_last:1;            // 标志:最后一项
    unsigned short          reserve2;               // 未使用

    union {
    ntfs_mft_attr_body_30   attr_body_30;           // 有可能没有此段
    unsigned char           data[32];               // 索引分配中的子节点的VCN(只有当标志为有子节点才使用)
    };

} ntfs_mft_attr_index_entry, *p_ntfs_mft_attr_index_entry;

typedef struct _ntfs_mft_attr_body_90               // 索引根,常驻
{
    ntfs_mft_attr_index_root_head   head;
    ntfs_mft_attr_index_root_body   body;

    ntfs_mft_attr_index_entry       entry[INDEX_ROOT_ENTRY_SIZE];
    unsigned char                   entry_count;

} ntfs_mft_attr_body_90, *p_ntfs_mft_attr_body_90;

typedef struct _ntfs_mft_attr_index_alloc_head      // 索引分配头
{
    unsigned char           name[4];                // "INDX"
    unsigned short          update_sn_offset;       // 更新序列号偏移
    unsigned short          update_sn_size;         // 更新序列号长度
    unsigned __int64        logfile_sn;             // 日志文件序列号
    unsigned __int64        vcn;                    // 本索引记录在索引分配中的VCN
    unsigned int            entry_offset;           // 索引项偏移
    unsigned int            entry_size;             // 索引项大小
    unsigned int            entry_size_alloc;       // 索引项分配大小
    unsigned char           flag;                   // 标志:1-有子节点
    unsigned char           zero[3];                // 0
    unsigned short          update_sn;              // 更新序列
    unsigned char           update_array[16];       // 更新序列数组

} ntfs_mft_attr_index_alloc_head, *p_ntfs_mft_attr_index_alloc_head;

typedef struct _ntfs_index_alloc                    // 索引组
{
    ntfs_mft_attr_index_alloc_head  head;

    ntfs_mft_attr_index_entry       entry[INDEX_ALLOC_ENTRY_SIZE];

    unsigned int                    entry_count;

} ntfs_index_alloc, *p_ntfs_index_alloc;

typedef struct _ntfs_mft_attr_body_A0               // 索引分配树节点,非常驻
{
    int a;

} ntfs_mft_attr_body_A0, *p_ntfs_mft_attr_body_A0;

typedef struct _ntfs_mft_attr_body_B0               // $MFT文件和索引的位图
{
    unsigned char           data[1024];             // 常驻,属性体是数据内容,非常驻,属性体是簇流运行

} ntfs_mft_attr_body_B0, *p_ntfs_mft_attr_body_B0;

typedef struct _ntfs_mft_attr_body_C0               // 用于符号链接，卷的挂载点
{
    unsigned int            type;                   // 重解析属性类型标志
    unsigned short          size;                   // 重解析数据的大小
    unsigned short          reserve1;               // 未使用
    unsigned short          name_pos;               // 目标名字的偏移
    unsigned short          name_len;               // 目标名字的长度
    unsigned int            reserve2;               // 未知
    unsigned short          name[32];               // 目标名字

} ntfs_mft_attr_body_C0, *p_ntfs_mft_attr_body_C0;  // 重解析点

typedef struct _ntfs_mft_attr_body_D0               // 扩展属性信息
{
    int a;

} ntfs_mft_attr_body_D0, *p_ntfs_mft_attr_body_D0;

typedef struct _ntfs_mft_attr_body_E0               // 扩展属性
{
    int a;

} ntfs_mft_attr_body_E0, *p_ntfs_mft_attr_body_E0;

typedef struct _ntfs_mft_attr_body_F0               // 早期NTFS属性
{
    int a;

} ntfs_mft_attr_body_F0, *p_ntfs_mft_attr_body_F0;

typedef struct _ntfs_mft_attr_body_100              // EFS加密属性,加密只对属性的内容时行加密,不对其属性头
{
    unsigned char           data[1024];

} ntfs_mft_attr_body_100, *p_ntfs_mft_attr_body_100;

typedef struct _ntfs_mft_attr_head_up               // 属性头前部16字节
{
    unsigned int            type;                   // 属性类型
    unsigned int            size;                   // 属性长度(包含属性头)
    unsigned char           unresident;             // 非常驻属性标记:0-常驻,1-非常驻
    unsigned char           name_len;               // 属性名长度
    unsigned short          name_pos;               // 属性名偏移
    unsigned short          flag_compress:1;        // 标志:1-压缩
    unsigned short          flag_reserve:13;        // 标志
    unsigned short          flag_encryption:1;      // 标志:4000-加密
    unsigned short          flag_few:1;             // 标志:8000-稀疏
    unsigned short          id;                     // 属性ID

} ntfs_mft_attr_head_up, *p_ntfs_mft_attr_head_up;

typedef struct _ntfs_mft_attr_head_at               // 常驻属性头8字节
{
    unsigned int            body_size;              // 属性体长度
    unsigned short          body_pos;               // 属性体位置
    unsigned char           index;                  // 索引标志
    unsigned char           reserve;                // 填充

} ntfs_mft_attr_head_at, *p_ntfs_mft_attr_head_at;

typedef struct _ntfs_mft_attr_head_un               // 非常驻属性头
{
    unsigned __int64        cluster_beg;            // 起始簇号VCN
    unsigned __int64        cluster_end;            // 结束簇号VCN
    unsigned short          datarun_pos;            // 属性体位置
    unsigned short          compress_unit;          // 压缩单位大小，2的N次方
    unsigned int            reserve;                // 填充
    unsigned __int64        size_alloc;             // 属性分配大小
    unsigned __int64        size_actual;            // 属性实际大小
    unsigned __int64        size_original;          // 属性原始大小

    unsigned char           data[RUNDATA_SIZE];     // 属性名和簇流数据

    unsigned int            rundata_count;              // 簇流数量
    unsigned __int64        rundata_beg[RUNDATA_SIZE];  // 起始簇号
    unsigned __int64        rundata_len[RUNDATA_SIZE];  // 使用簇量

} ntfs_mft_attr_head_un, *p_ntfs_mft_attr_head_un;

typedef struct _ntfs_mft_attr                       // MFT属性
{
    unsigned short          name[16];               // 属性名

    ntfs_mft_attr_head_up   head_up;                // 属性头前部

    union {
    ntfs_mft_attr_head_at   head_at;                // 常驻
    ntfs_mft_attr_head_un   head_un;                // 非 常驻
    };

    union {
    ntfs_mft_attr_body_10   body_10;
    ntfs_mft_attr_body_20   body_20;
    ntfs_mft_attr_body_30   body_30;
    ntfs_mft_attr_body_40   body_40;
    ntfs_mft_attr_body_50   body_50;
    ntfs_mft_attr_body_60   body_60;
    ntfs_mft_attr_body_70   body_70;
    ntfs_mft_attr_body_80   body_80;
    ntfs_mft_attr_body_90   body_90;
    ntfs_mft_attr_body_A0   body_A0;
    ntfs_mft_attr_body_B0   body_B0;
    ntfs_mft_attr_body_C0   body_C0;
    ntfs_mft_attr_body_D0   body_D0;
    ntfs_mft_attr_body_E0   body_E0;
    ntfs_mft_attr_body_F0   body_F0;
    ntfs_mft_attr_body_100  body_100;
    };

} ntfs_mft_attr, *p_ntfs_mft_attr;

typedef struct _ntfs_mft_head                       // MFT数据
{
    unsigned char           name[4];                // 固定值,一定是"FILE"
    unsigned short          update_sn_pos;          // 更新序列号的偏移
    unsigned short          update_sn_size;         // 更新序列号与更新数组以字为单位大小(S)
    unsigned __int64        logfile_sn;             // 日志文件序列号(每次记录被修改，都将导致该序列号加1)
    unsigned short          sn;                     // 序列号(记录本文件记录被重复使用的次数,每次文件删除时加1,跳过0值,如果为0,则保持为0)
    unsigned short          hard_link_count;        // 硬连接数,只出现在基本文件记录中,目录所含项数要使用到它
    unsigned short          attr_pos;               // 第一个属性流的偏移地址
    unsigned short          flag_used:1;            // 标志字节,使用中
    unsigned short          flag_dir:1;             // 标志字节,目录
    unsigned int            size_actual;            // 记录实际大小(填充到8字节,即以8字节为边界)
    unsigned int            size_alloc;             // 记录分配大小(填充到8字节,即以8字节为边界)
    unsigned __int64        ref_id;                 // 基本文件记录的文件参考号(扩展文件记录中使用,基本文件记录中为0,在基本文件记录的属性列表0x20属性存储中扩展文件记录的相关信息)
    unsigned short          next_attr_id;           // 下一个自由ID号,当增加新的属性时,将该值分配给新属性,然后该值增加,如果MFT记录重新使用,则将它置0,第一个实例总是0
    unsigned short          xp_check_id;            // windows xp中使用,也就是本记录使用的两个扇区的最后两个字节的值
    unsigned int            xp_record_id;           // windows xp中使用,本MFT记录号
    unsigned short          check_id;               // 校验码(此2字节必须与本扇区最后2字节相等否则为非法节点)
    unsigned int            update_seq_id;          // 更新序列数组

} ntfs_mft_head, *p_ntfs_mft_head;

typedef struct _ntfs_mft                            // MFT信息
{
    ntfs_mft_head           head;

    ntfs_mft_attr           attr[MFT_ATTR_SIZE];

} ntfs_mft, *p_ntfs_mft;

typedef struct _ntfs_dbr                            // NTFS的DBR数据
{
    unsigned char           jump[3];                    // 跳转指令EB 58 90
    unsigned char           oem[8];                     // 文件系统的ASIIC码:"NTFS,EXFAT,FAT32,FAT16"

    unsigned short          sector_byte_size;           // 每个扇区内的字节数量,一般为512
    unsigned char           cluster_sector_size;        // 每个簇占用的扇区数量,一般为8
    unsigned short          reserve_sector_count;       // 保留扇区数
    unsigned char           unuse1[4];                  // 不使用,总为0
    unsigned short          type;                       // 介质描述,硬盘为F8
    unsigned char           unuse2[2];                  // 不使用,总为0
    unsigned short          header_sector_count;        // 每磁头扇区数
    unsigned short          cluster_header_count;       // 每柱面磁头数
    unsigned int            hide_count;                 // 隐含扇区(从MBR到DBR的扇区总数)
    unsigned char           unuse3[4];                  // 不使用,总为0
    unsigned char           unuse4[4];                  // 不使用,总为80 00 80 00
    unsigned __int64        sector_count;               // 扇区总数,即分区大小
    unsigned __int64        mft_cluster_id;             // $MFT的开始簇号
    unsigned __int64        mirr_cluster_id;            // $MFTmirr的开始簇号
    unsigned int            mft_record_cluster_count;   // 每个MFT记录的簇数
    unsigned int            index_record_cluster_count; // 每个索引的簇数
    unsigned char           part_id[8];                 // 分区的逻辑序列号
    unsigned int            check_id;                   // 校验和

    // code
    unsigned char           code[426];                  // 代码

    // sig
    unsigned short          sig;                      // 0xAA55

} ntfs_dbr, *p_ntfs_dbr;

typedef struct _ntfs_info                           // NTFS信息
{
    unsigned __int64        pos_dbr;                // 硬盘中的扇区位置
    unsigned __int64        pos_cluster;
    unsigned __int64        pos_mft;
    unsigned __int64        pos_mirr;
    unsigned __int64        pos_logfile;
    unsigned __int64        pos_volume;
    unsigned __int64        pos_attrdef;
    unsigned __int64        pos_root;
    unsigned __int64        pos_bitmap;
    unsigned __int64        pos_boot;
    unsigned __int64        pos_badclus;
    unsigned __int64        pos_secure;
    unsigned __int64        pos_upcase;

    ntfs_dbr                dbr;

    ntfs_index_alloc        root[INDEX_ALLOC_SIZE];

    ntfs_mft                mft[MFT_SIZE];          // 主文件表

    ntfs_mft                mirr[MIRR_SIZE];        // 备份文件表(前4项)

    unsigned char           cluster[DIR_SIZE][CLUSTER_SIZE];// 簇数据

    unsigned char           bitmap[BITMAP_SIZE];    // 位图

    unsigned short          upcase[UPCASE_SIZE];    // 大小写映射

    unsigned __int64        mft_id[MFT_SIZE];       // MFT序号

    unsigned int            mft_count;              // MFT数量

} ntfs_info, *p_ntfs_info;

typedef struct _part_table_item                     // 分区表数据
{
    unsigned char           system;                 // 80存在操作系统的00不存在操作系统
    unsigned char           sector_start[3];        // 开始磁头-第1字节,扇区-第2字节的低6位,柱面–第2字节高2位+第3字节8位
    unsigned char           type;                   // 07-NTFS,05-扩展分区,0F-扩展分区>8G
    unsigned char           sector_end[3];          // 同sector_start,当空间不足时 FE FF FF
    unsigned int            offset;                 // 该分区相对上一级分区的位移量
    unsigned int            size;                   // 该分区所占扇区数量

} part_table_item, *p_part_table_item;

typedef struct _mbr_data                            // 主启动分区数据
{
    unsigned char           code[440];              // 引导代码

    unsigned char           disk_id[6];             // 磁盘签名

    part_table_item         part_table[4];          // 分区表

    // sig
    unsigned short          sig;                    // 0xAA55

} mbr_data, *p_mbr_data;

typedef struct _mbr_list_node                       // 分区链表
{
    unsigned __int64        pos;                    // 硬盘中的扇区位置

    mbr_data                mbr;

    union {
        fat16_info          fat16;                  // FAT16系统

        fat32_info          fat32;                  // FAT32系统

        exfat_info          exfat;                  // exFAT系统

        ntfs_info           ntfs;                   // NTFS系统

    } file_system[4];

    struct _mbr_list_node  *next;                   // EBR扩展分区使用

} mbr_list_node, *p_mbr_list_node;

typedef struct _disk_info {                         // 磁盘信息
    int                     bus_type;               // 总线类型

    int                     media_type;             // 介质类型

    TCHAR                   product_id[MAX_PATH];   // 产品ID

    mbr_list_node           mbr_list;               // 分区链表

    HTREEITEM               tree_node;              // 树节点

} disk_info, *p_disk_info;

#pragma pack(pop)

unsigned int    g_disk_count    = 0;
disk_info       g_disk[4]       = {0};

TCHAR          *g_title         = _T("ntfs");
HFONT           g_font          = NULL;
HWND            g_tree          = NULL;
TCHAR           txt[512];

/**
 * \brief   将字符串转成UNICODE字符串
 * \param   [out]   WCHAR               *out            unicode字符串
 * \param   [in]    const char          *in             字符串
 * \param   [in]    unsigned int        len             字符串长度
 * \return  无
 */
void get_unicode_str(WCHAR *out, const char *in, unsigned int len)
{
    unsigned int i, head = wcslen(txt);

    for (i = 0; i < len && *in != '\0'; i++, in++)
    {
        out[head + i] = *in;
    }

    out[head + i] = 0;
}

/**
 * \brief   得到簇流数据,第1字节前4位为使用簇数量长度大小,后4位为起始簇位置数据大小
 *          例:32 00 01 56 34 12 00
 *          使用簇数量长度大小: 3
 *          起始簇位置数据大小: 2
 *          起始簇位置: 123456 出现80~FF为首的为负数
 *          使用簇数量: 0100
 *          结束标志: 00
 * \param   [in]    unsigned char       *data           得到簇流数据
 * \param   [out]   unsigned int        *beg            簇开始
 * \param   [out]   unsigned int        *len            簇数量
 * \return  无
 */
void get_cluster_rundata(unsigned char *data, p_ntfs_mft_attr_head_un head_un)
{
    unsigned __int64 beg;
    unsigned __int64 len;
    p_cluster_rundata_head head;

    for (int i = 0; i < RUNDATA_SIZE; i++)
    {
        head = (p_cluster_rundata_head)data;
        beg = 0;
        len = 0;

        if (0 == head->beg && 0 == head->len)
        {
            head_un->rundata_count = i;
            return;
        }

        data++;

        for (int j = 0; j < head->len; j++)
        {
            len += data[j] << (j * 8);
        }

        data += head->len;

        for (int j = 0; j < head->beg; j++) // 可能出现负值
        {
            beg += data[j] << (j * 8);
        }

        data += head->beg;

        head_un->rundata_beg[i] = beg + ((i > 0) ? head_un->rundata_beg[i - 1] : 0);
        head_un->rundata_len[i] = len;
    }
}

/**
 * \brief   从磁盘读取数据
 * \param   [in]    HANDLE              device          硬盘句柄
 * \param   [in]    unsigned __int64    pos             扇区位置
 * \param   [in]    unsigned int        len             数据长度
 * \param   [out]   unsigned char*                      数据缓冲区
 * \return  0-成功,!0-失败
 */
int get_sector_data(HANDLE device, unsigned __int64 pos, unsigned int len, unsigned char *buf)
{
    if (NULL == buf)
    {
        return -1;
    }

    unsigned int  h = (unsigned int)(pos >> 23);    // pos*512,取前32位
    unsigned int  l = (unsigned int)(pos << 9);     // pos*512,取后32位
    int  r = 0;                            // 读取到的数据长度

    SetFilePointer(device, l, &h, FILE_BEGIN);

    ReadFile(device, buf, len, &r, NULL);

    if (r != len)
    {
        SP(_T("get_sector_data ReadFile device:%I64X pos:0x%I64X len:0x%X read:%d"), (unsigned __int64)device, pos, len, r);
        MessageBox(NULL, txt, g_title, MB_OK);
        return -2;
    }

    return 0;
}

/**
 * \brief   得到MFT数据
 *          -------------------------------------------------------------------
 *          |MFT Head|                  ATTR                   | ... |FFFFFFFF|
 *          -------------------------------------------------------------------
 *          |MFT Head|         ATTR Head      |    ATTR Body   |     |属性结束|
 *          -------------------------------------------------------------------
 *          |mft.data|head_up(head_at|head_un)|body_10~body_100|
 *          -------------------------------------------------------------------
 * \param   [in]    HANDLE              device          硬盘句柄
 * \param   [in]    unsigned __int64    pos             数据在硬盘的位置
 * \param   [out]   ntfs_mft            mft             文件表信息
 * \param   [in]    bool                mirr            是否是mirr
 * \return          0-成功,~0-失败
 */
int get_ntfs_mft_data(HANDLE device, unsigned __int64 pos, p_ntfs_mft mft, unsigned __int64 mft_id[MFT_SIZE], unsigned int *count)
{
    unsigned char            buf[1024];
    unsigned char           *buf_head_up;
    unsigned char           *buf_head_un;
    unsigned char           *buf_head_at;
    unsigned char           *buf_body;

    p_ntfs_mft_head          head;
    p_ntfs_mft_attr          attr;
    p_ntfs_mft_attr_head_up  head_up;
    p_ntfs_mft_attr_head_at  head_at;
    p_ntfs_mft_attr_head_un  head_un;
    p_ntfs_mft_attr_body_10  body_10;
    p_ntfs_mft_attr_body_20  body_20;
    p_ntfs_mft_attr_body_30  body_30;
    p_ntfs_mft_attr_body_40  body_40;
    p_ntfs_mft_attr_body_50  body_50;
    p_ntfs_mft_attr_body_60  body_60;
    p_ntfs_mft_attr_body_70  body_70;
    p_ntfs_mft_attr_body_80  body_80;
    p_ntfs_mft_attr_body_90  body_90;
    p_ntfs_mft_attr_body_A0  body_A0;
    p_ntfs_mft_attr_body_B0  body_B0;
    p_ntfs_mft_attr_body_C0  body_C0;
    p_ntfs_mft_attr_body_D0  body_D0;
    p_ntfs_mft_attr_body_E0  body_E0;
    p_ntfs_mft_attr_body_F0  body_F0;
    p_ntfs_mft_attr_body_100 body_100;

    for (unsigned int i = 0; i < *count && i < MFT_SIZE; i++, mft++)
    {
        get_sector_data(device, pos + mft_id[i] * 2, 1024, buf);

        mft->head    = *(p_ntfs_mft_head)buf;
        head         = &(mft->head);
        head->ref_id = mft_id[i];

        if (0 != strncmp(head->name, "FILE", 4))
        {
            SP(_T("get_ntfs_mft_data id:%d pos:0x%I64X mft_id[%d]:0x%I64X  isn't mft"), i, pos, i, mft_id[i]);
            MessageBox(NULL, txt, g_title, MB_OK);
            continue;
        }

        attr = mft->attr;
        buf_head_up = buf + head->attr_pos;

        for (int j = 0; j < MFT_ATTR_SIZE; buf_head_up += head_up->size, j++, attr++)
        {
            head_up  = &(attr->head_up);
            head_at  = &(attr->head_at);
            head_un  = &(attr->head_un);

            body_10  = &(attr->body_10);
            body_20  = &(attr->body_20);
            body_30  = &(attr->body_30);
            body_40  = &(attr->body_40);
            body_50  = &(attr->body_50);
            body_60  = &(attr->body_60);
            body_70  = &(attr->body_70);
            body_80  = &(attr->body_80);
            body_90  = &(attr->body_90);
            body_A0  = &(attr->body_A0);
            body_B0  = &(attr->body_B0);
            body_C0  = &(attr->body_C0);
            body_D0  = &(attr->body_D0);
            body_E0  = &(attr->body_E0);
            body_F0  = &(attr->body_F0);
            body_100 = &(attr->body_100);

            buf_head_at = buf_head_up + sizeof(ntfs_mft_attr_head_up);
            buf_head_un = buf_head_up + sizeof(ntfs_mft_attr_head_up);

            *head_up = *(p_ntfs_mft_attr_head_up)buf_head_up;

            if (head_up->type == 0xFFFFFFFF) // 属性结束标记
            {
                head_up->type = 0;
                break;
            }

            if (head_up->name_len > 0)      // 属性名
            {
                wcsncpy_s(attr->name, SIZEOF(attr->name), (WCHAR*)(buf_head_up + head_up->name_pos), head_up->name_len);
            }

            if (head_up->unresident)        // 非常驻属性头
            {
                *head_un = *(p_ntfs_mft_attr_head_un)buf_head_un;
                get_cluster_rundata(buf_head_up + head_un->datarun_pos, head_un);
            }
            else                            // 常驻属性头
            {
                *head_at = *(p_ntfs_mft_attr_head_at)buf_head_at;
                buf_body = buf_head_up + head_at->body_pos;

                switch (head_up->type)
                {
                    case 0x10:  *body_10  = *(p_ntfs_mft_attr_body_10)buf_body;  break;
                    case 0x40:  *body_40  = *(p_ntfs_mft_attr_body_40)buf_body;  break;
                    case 0x50:  *body_50  = *(p_ntfs_mft_attr_body_50)buf_body;  break;
                    case 0x70:  *body_70  = *(p_ntfs_mft_attr_body_70)buf_body;  break;
                    case 0x80:  *body_80  = *(p_ntfs_mft_attr_body_80)buf_body;  break;
                    case 0xB0:  *body_B0  = *(p_ntfs_mft_attr_body_B0)buf_body;  break;
                    case 0x100: *body_100 = *(p_ntfs_mft_attr_body_100)buf_body; break;/*
                    case 0xA0:  *body_A0  = *(p_ntfs_mft_attr_body_A0)buf_body;  break;
                    case 0xC0:  *body_C0  = *(p_ntfs_mft_attr_body_C0)buf_body;  break;
                    case 0xD0:  *body_D0  = *(p_ntfs_mft_attr_body_D0)buf_body;  break;
                    case 0xE0:  *body_E0  = *(p_ntfs_mft_attr_body_E0)buf_body;  break;
                    case 0xF0:  *body_F0  = *(p_ntfs_mft_attr_body_F0)buf_body;  break;*/
                    case 0x20:
                    {
                        p_ntfs_mft_attr_body_20_attr_list attr_list = body_20->attr_list;

                        for (unsigned int i = 0, pos = 0; i < ATTR_LIST_SIZE && pos < head_at->body_size; i++, pos += attr_list++->len)
                        {
                            *attr_list = *(p_ntfs_mft_attr_body_20_attr_list)(buf_body + pos);
                            attr_list->name[attr_list->name_len] = 0;

                            body_20->attr_list_count++;
                        }
                        break;
                    }
                    case 0x30:
                    {
                        *body_30 = *(p_ntfs_mft_attr_body_30)buf_body;
                        body_30->name[body_30->filename_len] = 0;
                        break;
                    }
                    case 0x60:
                    {
                        *body_60 = *(p_ntfs_mft_attr_body_60)buf_body;
                        attr->body_60.volume_name[attr->head_at.body_size] = 0;
                        break;
                    }
                    case 0x90:
                    {
                        body_90->head = *(p_ntfs_mft_attr_index_root_head)buf_body;
                        body_90->body = *(p_ntfs_mft_attr_index_root_body)(buf_body + sizeof(ntfs_mft_attr_index_root_head));

                        unsigned int len = body_90->body.pos_tail_used - body_90->body.pos_head; // 索引项总长
                        unsigned int pos = sizeof(ntfs_mft_attr_index_root_head) + sizeof(ntfs_mft_attr_index_root_body); // 指向第2个

                        p_ntfs_mft_attr_index_entry entry = body_90->entry;

                        for (int k = 0; k < INDEX_ROOT_ENTRY_SIZE && pos < len; k++, pos += entry++->len)
                        {
                            *entry = *(p_ntfs_mft_attr_index_entry)(buf_body + pos);

                            body_90->entry_count++;

                            if (entry->data_len < 0x40)
                            {
                                continue;
                            }

                            entry->attr_body_30.name[entry->attr_body_30.filename_len] = 0;

                            mft_id[*count] = entry->mft_id;
                            *count = *count + 1;
                        }
                        break;
                    }
                    default:
                    {
                        SP(_T("head_up->type:0x%X"), head_up->type);
                        MessageBox(NULL, txt, g_title, MB_OK);
                        break;
                    }
                }
            }
        }
    }

    return 0;
}

/**
 * \brief   得到ROOT数据
 * \param   [in]    HANDLE              device          硬盘句柄
 * \param   [in]    unsigned __int64    pos             数据在硬盘的位置
 * \param   [in]    unsigned __int64    len             数量
 * \param   [in]    p_ntfs_root         root            根目录
 * \return          0-成功,~0-失败
 */
int get_ntfs_root_data(HANDLE device, unsigned __int64 pos, unsigned __int64 len, p_ntfs_index_alloc root)
{
    unsigned char *buf = (unsigned char *)malloc((size_t)len);
    get_sector_data(device, pos, (unsigned int)len, buf);

    p_ntfs_mft_attr_index_alloc_head  head;
    p_ntfs_mft_attr_index_entry       entry;

    for (int i = 0; i < INDEX_ALLOC_SIZE && i < len / 0x1000; i++, root++)
    {
        head  = &(root->head);
        entry = root->entry;

        *head = *(p_ntfs_mft_attr_index_alloc_head)(buf + i * 0x1000);

        if (0 != strncmp(head->name, "INDX", 4))
        {
            SP(_T("i:%d INDX error"), i);
            MessageBox(NULL, txt, g_title, MB_OK);
        }

        pos = head->entry_offset + 0x18;

        for (int j = 0; j < INDEX_ALLOC_ENTRY_SIZE && pos < head->entry_size; j++, pos += entry++->len)
        {
            *entry = *(p_ntfs_mft_attr_index_entry)(buf + i * 0x1000 + pos);

            entry->attr_body_30.name[entry->attr_body_30.filename_len] = 0;

            root->entry_count++;
        }
    }

    free(buf);
    return 0;
}

/**
 * \brief   得到MBR和EBR数据
 * \param   [in]    HANDLE              device          硬盘句柄
 * \param   [in]    unsigned __int64    pos             数据在硬盘的位置
 * \param   [out]   p_mbr_list_node     mbr_list         扩展分区信息
 * \return          0-成功,其它-失败
 */
int get_mbr_ebr(HANDLE device, unsigned __int64 pos, p_mbr_list_node mbr_list)
{
    mbr_list->pos = pos;
    get_sector_data(device, pos, 512, (unsigned char*)&(mbr_list->mbr));

    if (mbr_list->mbr.sig != 0xAA55)
    {
        SP(_T("get_mbr_ebr pos:%I64x AA55 error:%x"), pos, mbr_list->mbr.sig);
        MessageBox(NULL, txt, g_title, MB_OK);
    }

    p_part_table_item item = mbr_list->mbr.part_table;

    for (int i = 0; i < 4 && item->type != 0; i++, item++)
    {
        if (item->type != 0x05 && item->type != 0x0F) // 05-扩展分区,0F-扩展分区>8G,一个分区表内只能有一个扩展分区
        {
            continue;
        }

        mbr_list->next = (p_mbr_list_node)malloc(sizeof(mbr_list_node));
        memset(mbr_list->next, 0, sizeof(mbr_list_node));

        get_mbr_ebr(device, pos + item->offset, mbr_list->next);
    }

    return 0;
}

/**
 * \brief   得到文件系统数据
 * \param   [in]    HANDLE              device          磁盘句柄
 * \param   [out]   p_disk_info         disk            磁盘数据
 * \return  0-成功,!0-失败
 */
int get_file_system(HANDLE device, p_disk_info disk)
{
    p_mbr_list_node mbr_list = &(disk->mbr_list);

    get_mbr_ebr(device, 0, mbr_list);

    unsigned __int64 pos;
    unsigned char buf[512];
    p_part_table_item item;

    do
    {
        item = mbr_list->mbr.part_table;

        for (int i = 0; i < 4 && item->type != 0; i++, item++)
        {
            if (item->type == 0x05 || item->type == 0x0F) // 跳过扩展分区
            {
                continue;
            }

            pos = mbr_list->pos + item->offset;
            get_sector_data(device, pos, 512, buf); // dbr

            if (buf[510] != 0x55 || buf[511] != 0xAA)
            {
                SP(_T("get_file_system pos:%I64u dbr 55AA error"), pos);
                MessageBox(NULL, txt, g_title, MB_OK);
            }

            if (item->type == 0x07 && 0 == strncmp(buf + 3, "EXFAT", 5)) // NTFS,EXFAT的分区类型都是0x07
            {
                item->type = 0x01; // EXFAT的分区类型置为1
            }

            switch (item->type)
            {
                case 0x06:
                case 0x16: // FAT16
                {
                    p_fat16_info fat16 = &(mbr_list->file_system[i].fat16);
                    p_fat16_dbr dbr = (p_fat16_dbr)buf;
                    fat16->dbr = *dbr;
                    fat16->pos_dbr = pos;

                    pos += dbr->reserve_sector_count;
                    fat16->pos_fat1 = pos;
                    get_sector_data(device, pos, sizeof(fat16->fat1), (unsigned char*)fat16->fat1);

                    pos += dbr->fat_sector_size;
                    fat16->pos_fat2 = pos;
                    get_sector_data(device, pos, sizeof(fat16->fat2), (unsigned char*)fat16->fat2);

                    pos += dbr->fat_sector_size;
                    fat16->pos_dir = pos;
                    get_sector_data(device, pos, sizeof(fat16->dir), (unsigned char*)fat16->dir);

                    pos += dbr->root_entries_count * sizeof(fat16_dir_short) / 512;
                    fat16->pos_cluster = pos;

                    for (int i = 0; i < DIR_SIZE; i++, pos += dbr->cluster_sector_size)
                    {
                        get_sector_data(device, pos, CLUSTER_SIZE, fat16->cluster[i]);
                    }

                    break;
                }
                case 0x0B: // FAT32
                {
                    p_fat32_info fat32 = &(mbr_list->file_system[i].fat32);
                    p_fat32_dbr dbr = (p_fat32_dbr)buf;
                    fat32->dbr = *dbr;
                    fat32->pos_dbr = pos;

                    fat32->pos_fsinfo = pos + dbr->file_info_sector_number;
                    get_sector_data(device, fat32->pos_fsinfo, sizeof(fat32->fsinfo), (unsigned char*)&(fat32->fsinfo));

                    pos += dbr->reserve_sector_count;
                    fat32->pos_fat1 = pos;
                    get_sector_data(device, pos, sizeof(fat32->fat1), (unsigned char*)fat32->fat1);

                    pos += dbr->fat32_sector_size;
                    fat32->pos_fat2 = pos;
                    get_sector_data(device, pos, sizeof(fat32->fat2), (unsigned char*)fat32->fat2);

                    pos += dbr->fat32_sector_size; // 簇数据
                    fat32->pos_cluster = pos;

                    for (int i = 0; i < DIR_SIZE; i++, pos += dbr->cluster_sector_size)
                    {
                        get_sector_data(device, pos, CLUSTER_SIZE, fat32->cluster[i]);
                    }

                    pos = fat32->pos_cluster + dbr->cluster_sector_size * (fat32->dbr.root_cluster_id - 2); // 根目录,簇号从2开始
                    fat32->pos_dir = pos;
                    get_sector_data(device, pos, sizeof(fat32->dir), (unsigned char*)fat32->dir);
                    break;
                }
                case 0x01: // EXFAT
                {
                    p_exfat_info exfat = &(mbr_list->file_system[i].exfat);
                    p_exfat_dbr dbr = (p_exfat_dbr)buf;
                    exfat->dbr = *dbr;
                    exfat->pos_dbr = pos;

                    pos += dbr->FatOffset;
                    exfat->pos_fat1 = pos;
                    get_sector_data(device, pos, sizeof(exfat->fat1), (unsigned char*)exfat->fat1);

                    if (dbr->NumberOfFats > 1)
                    {
                        pos += dbr->FatLength;
                        exfat->pos_fat2 = pos;
                        get_sector_data(device, pos, sizeof(exfat->fat2), (unsigned char*)exfat->fat2);
                    }

                    int cluster_sector_size = (int)pow(2, dbr->SectorsPerClusterShift);

                    pos = exfat->pos_dbr + dbr->ClusterHeapOffset; // 簇数据
                    exfat->pos_cluster = pos;

                    for (int i = 0; i < DIR_SIZE; i++, pos += cluster_sector_size)
                    {
                        get_sector_data(device, pos, CLUSTER_SIZE, exfat->cluster[i]);
                    }

                    pos = exfat->pos_cluster + cluster_sector_size * (dbr->ClusterOfRootDirectory - 2); // 根目录
                    exfat->pos_dir = pos;
                    get_sector_data(device, pos, sizeof(exfat->dir), (unsigned char*)exfat->dir);

                    pos = exfat->pos_cluster + cluster_sector_size * (exfat->dir[1].bitmap.FirstCluster - 2); // 根目录项1,簇2
                    exfat->pos_bitmap = pos;
                    get_sector_data(device, pos, sizeof(exfat->bitmap), (unsigned char*)exfat->bitmap);

                    pos = exfat->pos_cluster + cluster_sector_size * (exfat->dir[2].upcase.FirstCluster - 2); // 根目录项2
                    exfat->pos_upcase = pos;
                    get_sector_data(device, pos, sizeof(exfat->upcase), (unsigned char*)exfat->upcase);

                    break;
                }
                case 0x07: // NTFS
                {
                    p_ntfs_info ntfs = &(mbr_list->file_system[i].ntfs);
                    p_ntfs_dbr dbr = (p_ntfs_dbr)buf;
                    ntfs->dbr = *dbr;
                    ntfs->pos_dbr = pos;

                    pos = ntfs->pos_dbr + dbr->reserve_sector_count ; // 簇数据
                    ntfs->pos_cluster = pos;

                    for (int i = 0; i < DIR_SIZE; i++, pos += dbr->cluster_sector_size)
                    {
                        get_sector_data(device, pos, CLUSTER_SIZE, ntfs->cluster[i]);
                    }

                    ntfs->mft_id[0]  = 0;
                    ntfs->mft_id[1]  = 1;
                    ntfs->mft_id[2]  = 2;
                    ntfs->mft_id[3]  = 3;
                    ntfs->mft_id[4]  = 4;
                    ntfs->mft_id[5]  = 5;
                    ntfs->mft_id[6]  = 6;
                    ntfs->mft_id[7]  = 7;
                    ntfs->mft_id[8]  = 8;
                    ntfs->mft_id[9]  = 9;
                    ntfs->mft_id[10] = 10;
                    ntfs->mft_id[11] = 11;
                    ntfs->mft_id[12] = 12;
                    ntfs->mft_id[13] = 13;
                    ntfs->mft_id[14] = 14;
                    ntfs->mft_id[15] = 15;
                    ntfs->mft_count  = 16;

                    pos = ntfs->pos_cluster + dbr->mft_cluster_id * dbr->cluster_sector_size;
                    ntfs->pos_mft = pos;
                    get_ntfs_mft_data(device, pos, ntfs->mft, ntfs->mft_id, &(ntfs->mft_count));

                    int count = MIRR_SIZE;
                    pos = ntfs->pos_cluster + dbr->mirr_cluster_id * dbr->cluster_sector_size;
                    ntfs->pos_mirr = pos;
                    get_ntfs_mft_data(device, pos, ntfs->mirr, ntfs->mft_id, &count);

                    ntfs->pos_logfile = ntfs->pos_cluster + ntfs->mft[2].attr[2].head_un.rundata_beg[0] * dbr->cluster_sector_size;
                    ntfs->pos_attrdef = ntfs->pos_cluster + ntfs->mft[4].attr[2].head_un.rundata_beg[0] * dbr->cluster_sector_size;
                    ntfs->pos_root    = ntfs->pos_cluster + ntfs->mft[5].attr[4].head_un.rundata_beg[0] * dbr->cluster_sector_size;
                    ntfs->pos_bitmap  = ntfs->pos_cluster + ntfs->mft[6].attr[2].head_un.rundata_beg[0] * dbr->cluster_sector_size;
                    ntfs->pos_upcase  = ntfs->pos_cluster + ntfs->mft[10].attr[2].head_un.rundata_beg[0] * dbr->cluster_sector_size;

                    get_sector_data(device, ntfs->pos_bitmap, sizeof(ntfs->bitmap), (unsigned char*)ntfs->bitmap);
                    get_sector_data(device, ntfs->pos_upcase, sizeof(ntfs->upcase), (unsigned char*)ntfs->upcase);

                    get_ntfs_root_data(device, ntfs->pos_root, ntfs->mft[5].attr[4].head_un.rundata_len[0] * ntfs->dbr.cluster_sector_size * 512, ntfs->root);
                    break;
                }
            }
        }

    } while (mbr_list = mbr_list->next);

    return 0;
}

/**
 * \brief   得到硬盘信息
 * \param   [in]    int                 max             硬盘最大数量
 * \param   [out]   p_disk_info         disk_array      硬盘信息
 * \return  硬盘数量
 */
int get_disk_info(int max, p_disk_info disk_array)
{
    int i;
    int ret;
    unsigned int read;
    DISK_GEOMETRY pdg;
    BYTE buf[MAX_PATH];
    STORAGE_PROPERTY_QUERY query;
    STORAGE_DEVICE_DESCRIPTOR *desc;

    query.QueryType = PropertyStandardQuery;
    query.PropertyId = StorageDeviceProperty;

    desc = (STORAGE_DEVICE_DESCRIPTOR*)buf;
    desc->Size = sizeof(desc);

    p_disk_info disk = disk_array;

    for (i = 0; i < max; i++, disk++)
    {
        SP(_T("\\\\.\\PHYSICALDRIVE%d"), i);

        HANDLE device = CreateFile(txt,             // drive to open
                                   GENERIC_READ,    // access to the drive, GENERIC_READ | GENERIC_WRITE
                                   FILE_SHARE_READ, // share mode, FILE_SHARE_READ | FILE_SHARE_WRITE
                                   NULL,            // default security attributes
                                   OPEN_EXISTING,   // disposition
                                   0,               // file attributes
                                   NULL             // do not copy file attribute
                                   );

        if (device == INVALID_HANDLE_VALUE)
        {
            SP(_T("CreateFile(%d) Error: %ld"), i, GetLastError());
            //MessageBox(NULL, txt, g_title, MB_OK);
            break;
        }

        ret = DeviceIoControl(device,
                              IOCTL_DISK_GET_DRIVE_GEOMETRY,
                              NULL,
                              0,
                              &pdg,
                              sizeof(pdg),
                              &read,
                              NULL);

        if (!ret)
        {
            SP(_T("IOCTL_DISK_GET_DRIVE_GEOMETRY Error: %ld"), GetLastError());
            MessageBox(NULL, txt, g_title, MB_OK);
            CloseHandle(device);
            continue;
        }

        disk->media_type = pdg.MediaType;

        ret = DeviceIoControl(device,
                              IOCTL_STORAGE_QUERY_PROPERTY,
                              &query,
                              sizeof(query),
                              buf,
                              sizeof(buf),
                              &read,
                              NULL);

        if (!ret)
        {
            SP(_T("IOCTL_STORAGE_QUERY_PROPERTY Error: %ld"), GetLastError());
            MessageBox(NULL, txt, g_title, MB_OK);
            CloseHandle(device);
            continue;
        }

        for (int j = 0, k = desc->ProductIdOffset; buf[k] != 0; k++)
        {
            disk->product_id[j++] = buf[k];
            disk->product_id[j] = 0;
        }

        disk->bus_type = desc->BusType;

        get_file_system(device, disk);

        CloseHandle(device);
    }

    return i;
}

/**
 * \brief   在父节点的最后插入子节点
 * \param   [in]    HTREEITEM           parent           父句柄
 * \param   [in]    TCHAR               *txt             文本
 * \return  节点句柄
 */
HTREEITEM tree_insert_sub(HTREEITEM parent, TCHAR *txt)
{
    TVINSERTSTRUCT tv;
    tv.hParent          = parent;
    tv.hInsertAfter     = TVI_LAST;
    tv.item.mask        = TVIF_TEXT;
    tv.item.pszText     = txt;

    return TreeView_InsertItem(g_tree, &tv);
}

/**
 * \brief   在父节点排序插入子节点
 * \param   [in]    HTREEITEM           parent           父句柄
 * \param   [in]    TCHAR               *txt             文本
 * \return  节点句柄
 */
HTREEITEM tree_insert_sort(HTREEITEM parent, TCHAR *txt)
{
    TVINSERTSTRUCT tv;
    tv.hParent          = parent;
    tv.hInsertAfter     = TVI_SORT;
    tv.item.mask        = TVIF_TEXT;
    tv.item.pszText     = txt;

    return TreeView_InsertItem(g_tree, &tv);
}

void tree_data_bit8(HTREEITEM parent, unsigned __int64 pos, TCHAR *name, unsigned char *data, unsigned int count)
{
    if (0 == pos)
    {
        return;
    }

    SP(_T("扇区:%08I64X %s"), pos, name);
    HTREEITEM node = tree_insert_sort(parent, txt);

    for (unsigned int i = 0; i < count; i += 16)
    {
        SP(_T("%03X: %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X"),
               i,
               data[i],   data[i+1], data[i+2],  data[i+3],  data[i+4],  data[i+5],  data[i+6],  data[i+7],
               data[i+8], data[i+9], data[i+10], data[i+11], data[i+12], data[i+13], data[i+14], data[i+15]);

        tree_insert_sub(node, txt);
    }
}

void tree_data_bit16(HTREEITEM parent, unsigned __int64 pos, TCHAR *name, unsigned short *data, unsigned int count)
{
    if (0 == pos)
    {
        return;
    }

    SP(_T("扇区:%08I64X %s"), pos, name);
    HTREEITEM node = tree_insert_sort(parent, txt);

    for (unsigned int i = 0; i < count; i += 8)
    {
        SP(_T("%03X: %04X %04X %04X %04X  %04X %04X %04X %04X"),
               i * 2,
               data[i], data[i+1], data[i+2], data[i+3], data[i+4], data[i+5], data[i+6], data[i+7]);

        tree_insert_sub(node, txt);
    }
}

void tree_data_bit32(HTREEITEM parent, unsigned __int64 pos, TCHAR *name, unsigned int *data, unsigned int count)
{
    if (0 == pos)
    {
        return;
    }

    SP(_T("扇区:%08I64X %s"), pos, name);
    HTREEITEM node = tree_insert_sort(parent, txt);

    for (unsigned int i = 0; i < count; i += 4)
    {
        SP(_T("%03X: %08X %08X  %08X %08X"),
              i * 4,
              data[i], data[i+1], data[i+2], data[i+3]);

        tree_insert_sub(node, txt);
    }
}

void tree_data_cluster(HTREEITEM parent, unsigned __int64 pos, unsigned char cluster_sector_size, unsigned char cluster[DIR_SIZE][CLUSTER_SIZE], bool ntfs)
{
    TCHAR title[128];

    SP(_T("扇区:%08I64X CLUSTER"), pos);
    HTREEITEM node = tree_insert_sort(parent, txt);

    for (int i = 0; i < DIR_SIZE; i++)
    {
        _stprintf_s(title, SIZEOF(title), _T("簇:%X"), i + (ntfs ? 0 : 2));

        tree_data_bit8(node, pos + i * cluster_sector_size, title, cluster[i], CLUSTER_SIZE);
    }
}

void tree_fat16_dbr(HTREEITEM parent, unsigned __int64 pos, p_fat16_dbr dbr, int type)
{
    SP(_T("扇区:%08I64X DBR---------%s"), pos, part_type_text[type]);
    HTREEITEM node = tree_insert_sort(parent, txt);

    SP(_T("跳转指令:     %02X %02X %02X"), dbr->jump[0], dbr->jump[1], dbr->jump[2]);
    tree_insert_sub(node, txt);

    SP(_T("OEM           "));
    get_unicode_str(txt, dbr->oem, 8);
    tree_insert_sub(node, txt);

    SP(_T("扇区字节数:   %X"), dbr->sector_byte_size);
    tree_insert_sub(node, txt);

    SP(_T("簇内扇区数:   %X"), dbr->cluster_sector_size);
    tree_insert_sub(node, txt);

    SP(_T("保留扇区数:   %X"), dbr->reserve_sector_count);
    tree_insert_sub(node, txt);

    SP(_T("FAT 表数量:   %X"), dbr->fat_count);
    tree_insert_sub(node, txt);

    SP(_T("根目录项数:   %X"), dbr->root_entries_count);
    tree_insert_sub(node, txt);

    SP(_T("小分区扇区数: %X"), dbr->small_sector_count);
    tree_insert_sub(node, txt);

    SP(_T("介质描述:     %X        F0-软盘,F8-硬盘"), dbr->media_des);
    tree_insert_sub(node, txt);

    SP(_T("FAT表大小:    %X"), dbr->fat_sector_size);
    tree_insert_sub(node, txt);

    SP(_T("每道扇区数:   %X"), dbr->trark_sector_count);
    tree_insert_sub(node, txt);

    SP(_T("磁头数量:     %X"), dbr->disk_header_count);
    tree_insert_sub(node, txt);

    SP(_T("隐含扇区数:   %X"), dbr->disk_header_count);
    tree_insert_sub(node, txt);

    SP(_T("大分区扇区数: %X"), dbr->hidden_count);
    tree_insert_sub(node, txt);

    SP(_T("磁盘类型:     %X        00-软盘,80-硬盘"), dbr->physical_disk_type);
    tree_insert_sub(node, txt);

    SP(_T("扩展标记:     %X        28,29"), dbr->extended_boot_signature);
    tree_insert_sub(node, txt);

    SP(_T("卷标:         "));
    get_unicode_str(txt, dbr->volume_label, 11);
    tree_insert_sub(node, txt);

    SP(_T("系统类型:     "));
    get_unicode_str(txt, dbr->system_type, 8);
    tree_insert_sub(node, txt);

    SP(_T("签名:         %X"), dbr->sig);
    tree_insert_sub(node, txt);
}

void tree_fat16_dir(HTREEITEM parent, unsigned __int64 pos, p_fat16_dir dir)
{
    WCHAR filename[256];

    SP(_T("扇区:%08I64X DIR"), pos);
    HTREEITEM node, file = tree_insert_sort(parent, txt);

    for (int i = 0; i < DIR_SIZE; i++, dir++)
    {
        if (dir->s.file[0] == 0x00 || dir->s.file[0] == 0xE5) // 删除标记
        {
            continue;
        }

        if (dir->s.attr != 0x0F) // 短文件名
        {
            SP(_T("序号:%02X 文件名:"), i);
            get_unicode_str(txt, dir->s.file, 8);
            node = tree_insert_sub(file, txt);

            SP(_T("文件名:   "));
            get_unicode_str(txt, dir->s.file, 8);
            tree_insert_sub(node, txt);

            SP(_T("扩展名:   "));
            get_unicode_str(txt, dir->s.ext, 3);
            tree_insert_sub(node, txt);

            SP(_T("属性:     %X F-长文件名,1-只读,2-隐藏,4-系统,8-卷标,10-目录,20-归档"), dir->s.attr);
            tree_insert_sub(node, txt);

            SP(_T("修改时间: %X"), dir->s.update_time);
            tree_insert_sub(node, txt);

            SP(_T("修改日期: %X"), dir->s.update_date);
            tree_insert_sub(node, txt);

            SP(_T("首簇号:   %X"), dir->s.cluster_id);
            tree_insert_sub(node, txt);

            SP(_T("大小:     %X"), dir->s.size);
            tree_insert_sub(node, txt);
        }
        else
        {
            unsigned int id = (dir->l.flag & 0x1F);
            unsigned int end = dir->l.flag & 0x40;

            int pt = --id * 13;
            memcpy(&filename[pt],      dir->l.filename1, 10);
            memcpy(&filename[pt + 5],  dir->l.filename2, 12);
            memcpy(&filename[pt + 11], dir->l.filename3, 4);

            if (0 == id)
            {
                SP(_T("序号:%02X 文件名:%s"), i, filename);
                node = tree_insert_sub(file, txt);

                SP(_T("文件名:   %s"), filename);
                tree_insert_sub(node, txt);

                SP(_T("校验码:   %X"), dir->l.check);
                tree_insert_sub(node, txt);
            }
        }
    }
}

void tree_fat32_dbr(HTREEITEM parent, unsigned __int64 pos, p_fat32_dbr dbr, int type)
{
    SP(_T("扇区:%08I64X DBR---------%s"), pos, part_type_text[type]);
    HTREEITEM node = tree_insert_sort(parent, txt);

    SP(_T("跳转指令:     %02X %02X %02X"), dbr->jump[0], dbr->jump[1], dbr->jump[2]);
    tree_insert_sub(node, txt);

    SP(_T("OEM           "));
    get_unicode_str(txt, dbr->oem, 8);
    tree_insert_sub(node, txt);

    SP(_T("扇区字节数:   %X"), dbr->sector_byte_size);
    tree_insert_sub(node, txt);

    SP(_T("簇内扇区数:   %X"), dbr->cluster_sector_size);
    tree_insert_sub(node, txt);

    SP(_T("保留扇区数:   %X"), dbr->reserve_sector_count);
    tree_insert_sub(node, txt);

    SP(_T("FAT 表数量:   %X"), dbr->fat_count);
    tree_insert_sub(node, txt);

    SP(_T("根目录项数:   %X"), dbr->root_entries_count);
    tree_insert_sub(node, txt);

    SP(_T("小分区扇区数: %X"), dbr->small_sector_count);
    tree_insert_sub(node, txt);

    SP(_T("介质描述:     %X        F0-软盘,F8-硬盘"), dbr->media_des);
    tree_insert_sub(node, txt);

    SP(_T("FAT表大小:    %X"), dbr->fat32_sector_size);
    tree_insert_sub(node, txt);

    SP(_T("每道扇区数:   %X"), dbr->trark_sector_count);
    tree_insert_sub(node, txt);

    SP(_T("磁头数量:     %X"), dbr->disk_header_count);
    tree_insert_sub(node, txt);

    SP(_T("隐含扇区数:   %X"), dbr->disk_header_count);
    tree_insert_sub(node, txt);

    SP(_T("大分区扇区数: %X"), dbr->hide_sector_count);
    tree_insert_sub(node, txt);

    SP(_T("FAT表大小:    %X"), dbr->fat32_sector_size);
    tree_insert_sub(node, txt);

    SP(_T("文件系统版本: %X"), dbr->file_system_version);
    tree_insert_sub(node, txt);

    SP(_T("根目录的簇号: %X"), dbr->root_cluster_id);
    tree_insert_sub(node, txt);

    SP(_T("文件信息扇区: %X"), dbr->file_info_sector_number);
    tree_insert_sub(node, txt);

    SP(_T("mbr备份位置:  %X"), dbr->backup_boot_pos);
    tree_insert_sub(node, txt);

    SP(_T("磁盘类型:     %X        00-软盘,80-硬盘"), dbr->physical_disk_type);
    tree_insert_sub(node, txt);

    SP(_T("扩展标记:     %X        28,29"), dbr->extended_boot_signature);
    tree_insert_sub(node, txt);

    SP(_T("卷标:         "));
    get_unicode_str(txt, dbr->volume_label, 11);
    tree_insert_sub(node, txt);

    SP(_T("系统类型:     "));
    get_unicode_str(txt, dbr->system_type, 8);
    tree_insert_sub(node, txt);

    SP(_T("签名:         %X"), dbr->sig);
    tree_insert_sub(node, txt);
}

void tree_fat32_fsinfo(HTREEITEM parent, unsigned __int64 pos, p_fat32_fsinfo info)
{
    SP(_T("扇区:%08I64X FSINFO"), pos);
    HTREEITEM node = tree_insert_sort(parent, txt);

    SP(_T("RRaA:         "));
    get_unicode_str(txt, info->flag1, 4);
    tree_insert_sub(node, txt);

    SP(_T("rrAa:         "));
    get_unicode_str(txt, info->flag2, 4);
    tree_insert_sub(node, txt);

    SP(_T("空簇数:       %X"), info->unused_count);
    tree_insert_sub(node, txt);

    SP(_T("下一可用簇:   %X"), info->next);
    tree_insert_sub(node, txt);

    SP(_T("签名:         %X"), info->sig);
    tree_insert_sub(node, txt);
}

void tree_fat32_dir(HTREEITEM parent, unsigned __int64 pos, p_fat32_dir dir)
{
    WCHAR filename[256];

    SP(_T("扇区:%08I64X DIR"), pos);
    HTREEITEM node, file = tree_insert_sort(parent, txt);

    for (int i = 0; i < DIR_SIZE; i++, dir++)
    {
        if (dir->s.file[0] == 0x00 || dir->s.file[0] == 0xE5) // 删除标记
        {
            continue;
        }

        if (dir->s.attr != 0x0F) // 短文件名
        {
            SP(_T("序号:%02X 文件名:"), i);
            get_unicode_str(txt, dir->s.file, 8);
            node = tree_insert_sub(file, txt);

            SP(_T("文件名:   "));
            get_unicode_str(txt, dir->s.file, 8);
            tree_insert_sub(node, txt);

            SP(_T("扩展名:   "));
            get_unicode_str(txt, dir->s.ext, 3);
            tree_insert_sub(node, txt);

            SP(_T("属性:     %X F-长文件名,1-只读,2-隐藏,4-系统,8-卷标,10-目录,20-归档"), dir->s.attr);
            tree_insert_sub(node, txt);

            SP(_T("创建时间: %X     10毫秒"), dir->s.create_millisecond);
            tree_insert_sub(node, txt);

            SP(_T("创建时间: %X"), dir->s.create_time);
            tree_insert_sub(node, txt);

            SP(_T("创建日期: %X"), dir->s.create_date);
            tree_insert_sub(node, txt);

            SP(_T("修改时间: %X"), dir->s.update_time);
            tree_insert_sub(node, txt);

            SP(_T("修改日期: %X"), dir->s.update_date);
            tree_insert_sub(node, txt);

            SP(_T("首簇号:   %X%04X"), dir->s.cluster_id_high, dir->s.cluster_id);
            tree_insert_sub(node, txt);

            SP(_T("大小:     %X"), dir->s.size);
            tree_insert_sub(node, txt);
        }
        else
        {
            unsigned int id = (dir->l.flag & 0x1F);
            unsigned int end = dir->l.flag & 0x40;

            int pt = --id * 13;
            memcpy(&filename[pt],      dir->l.filename1, 10);
            memcpy(&filename[pt + 5],  dir->l.filename2, 12);
            memcpy(&filename[pt + 11], dir->l.filename3, 4);

            if (0 == id)
            {
                SP(_T("序号:%02X 文件名:%s"), i, filename);
                node = tree_insert_sub(file, txt);

                SP(_T("文件名:   %s"), filename);
                tree_insert_sub(node, txt);

                SP(_T("校验码:   %X"), dir->l.check);
                tree_insert_sub(node, txt);
            }
        }
    }
}

void tree_exfat_dbr(HTREEITEM parent, unsigned __int64 pos, p_exfat_dbr dbr, int type)
{
    SP(_T("扇区:%08I64X DBR---------%s"), pos, part_type_text[type]);
    HTREEITEM node = tree_insert_sort(parent, txt);

    SP(_T("跳转指令:     %02X %02X %02X"), dbr->jump[0], dbr->jump[1], dbr->jump[2]);
    tree_insert_sub(node, txt);

    SP(_T("OEM           "));
    get_unicode_str(txt, dbr->oem, 8);
    tree_insert_sub(node, txt);

    SP(_T("分区偏移量:   %I64X"), dbr->PartitionOffset);
    tree_insert_sub(node, txt);

    SP(_T("卷的大小:     %I64X"), dbr->VolumeLength);
    tree_insert_sub(node, txt);

    SP(_T("FAT偏移量:    %X"), dbr->FatOffset);
    tree_insert_sub(node, txt);

    SP(_T("FAT表大小:    %X"), dbr->FatLength);
    tree_insert_sub(node, txt);

    SP(_T("簇偏移量:     %X"), dbr->ClusterHeapOffset);
    tree_insert_sub(node, txt);

    SP(_T("簇数量:       %X"), dbr->ClusterCount);
    tree_insert_sub(node, txt);

    SP(_T("根目录簇:     %X"), dbr->ClusterOfRootDirectory);
    tree_insert_sub(node, txt);

    SP(_T("卷标:         %X"), dbr->VolumeSerialNumber);
    tree_insert_sub(node, txt);

    SP(_T("版本号:       %X"), dbr->FileSystemRevision);
    tree_insert_sub(node, txt);

    SP(_T("活动FAT号:    %X"), dbr->VolumeFlags_ActiveFat);
    tree_insert_sub(node, txt);

    SP(_T("卷不一致:     %X"), dbr->VolumeFlags_VolumeDirty);
    tree_insert_sub(node, txt);

    SP(_T("媒体故障:     %X"), dbr->VolumeFlags_MediaFailure);
    tree_insert_sub(node, txt);

    SP(_T("应置为0:      %X"), dbr->VolumeFlags_ClearToZero);
    tree_insert_sub(node, txt);

    SP(_T("扇区字节数:   %X         2^N"), dbr->BytesPerSectorShift);
    tree_insert_sub(node, txt);

    SP(_T("簇的扇区数:   %X         2^N"), dbr->SectorsPerClusterShift);
    tree_insert_sub(node, txt);

    SP(_T("FAT数:        %X"), dbr->NumberOfFats);
    tree_insert_sub(node, txt);

    SP(_T("磁盘类型:     %X        00-软盘,80-硬盘"), dbr->DriveSelect);
    tree_insert_sub(node, txt);

    SP(_T("簇占比:       %02d        0~100%%"), dbr->PercentInUse);
    tree_insert_sub(node, txt);

    SP(_T("签名:         %X"), dbr->sig);
    tree_insert_sub(node, txt);
}

void tree_exfat_dir(HTREEITEM parent, unsigned __int64 pos, p_exfat_dir dir)
{
    WCHAR filename[256];

    SP(_T("扇区:%08I64X DIR"), pos);
    HTREEITEM node, file = tree_insert_sort(parent, txt);

    for (int i = 0; i < DIR_SIZE; i++, dir++)
    {
        switch (dir->TypeCode)
        {
            case 0: // 卷GUID,流扩展
            {
                if (dir->TypeImportance == 1 && dir->TypeCategory == 0)
                {
                    SP(_T("序号:%02X 类型:%X 重要:%X 主要:%X 使用:%X 卷GUID"), i, dir->TypeCode, dir->TypeImportance, dir->TypeCategory, dir->TypeInUse);
                    node = tree_insert_sub(file, txt);

                    SP(_T("次项数量:   %X"), dir->guid.SecondaryCount);
                    tree_insert_sub(node, txt);
                    SP(_T("校验码:     %X"), dir->guid.SetChecksum);
                    tree_insert_sub(node, txt);
                    SP(_T("使用簇:     %X      0-未使用磁盘空间"), dir->guid.GeneralPrimaryFlags_UseCluster);
                    tree_insert_sub(node, txt);
                    SP(_T("不使用簇链:     %X"), dir->guid.GeneralPrimaryFlags_NoFatChain);
                    tree_insert_sub(node, txt);
                    SP(_T("GUID:     %02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"),
                           dir->guid.VolumeGuid[0],
                           dir->guid.VolumeGuid[1],
                           dir->guid.VolumeGuid[2],
                           dir->guid.VolumeGuid[3],
                           dir->guid.VolumeGuid[4],
                           dir->guid.VolumeGuid[5],
                           dir->guid.VolumeGuid[6],
                           dir->guid.VolumeGuid[7],
                           dir->guid.VolumeGuid[8],
                           dir->guid.VolumeGuid[9],
                           dir->guid.VolumeGuid[10],
                           dir->guid.VolumeGuid[11],
                           dir->guid.VolumeGuid[12],
                           dir->guid.VolumeGuid[13],
                           dir->guid.VolumeGuid[14],
                           dir->guid.VolumeGuid[15]);
                    tree_insert_sub(node, txt);
                }
                else if (dir->TypeImportance == 0 && dir->TypeCategory == 1)
                {
                    SP(_T("序号:%02X 类型:%X 重要:%X 主要:%X 使用:%X 流扩展"), i, dir->TypeCode, dir->TypeImportance, dir->TypeCategory, dir->TypeInUse);
                    node = tree_insert_sub(file, txt);

                    SP(_T("使用簇:     %X      0-未使用磁盘空间"), dir->ext.GeneralSecondaryFlags_UseCluster);
                    tree_insert_sub(node, txt);
                    SP(_T("不使用簇链: %X"), dir->ext.GeneralSecondaryFlags_NoFatChain);
                    tree_insert_sub(node, txt);
                    SP(_T("名字长度:   %X"), dir->ext.NameLength);
                    tree_insert_sub(node, txt);
                    SP(_T("名字校验:   %X"), dir->ext.NameHash);
                    tree_insert_sub(node, txt);
                    SP(_T("有效数据长: %I64X"), dir->ext.ValidDataLength);
                    tree_insert_sub(node, txt);
                    SP(_T("簇号:       %X"), dir->ext.FirstCluster);
                    tree_insert_sub(node, txt);
                    SP(_T("数据长度:   %I64X"), dir->ext.DataLength);
                    tree_insert_sub(node, txt);
                }
                break;
            }
            case 1: // 位图,文件名
            {
                if (dir->TypeImportance == 0 && dir->TypeCategory == 0)
                {
                    SP(_T("序号:%02X 类型:%X 重要:%X 主要:%X 使用:%X 位图"), i, dir->TypeCode, dir->TypeImportance, dir->TypeCategory, dir->TypeInUse);
                    node = tree_insert_sub(file, txt);

                    SP(_T("FAT ID:     %X"), dir->bitmap.BitmapFlags_BitmapIdentifier);
                    tree_insert_sub(node, txt);
                    SP(_T("簇号:       %X"), dir->bitmap.FirstCluster);
                    tree_insert_sub(node, txt);
                    SP(_T("数据长度:   %I64X"), dir->bitmap.DataLength);
                    tree_insert_sub(node, txt);
                }
                else if (dir->TypeImportance == 0 && dir->TypeCategory == 1)
                {
                    SP(_T("序号:%02X 类型:%X 重要:%X 主要:%X 使用:%X 文件名"), i, dir->TypeCode, dir->TypeImportance, dir->TypeCategory, dir->TypeInUse);
                    node = tree_insert_sub(file, txt);

                    SP(_T("使用簇:     %X      0-未使用磁盘空间"), dir->filename.GeneralSecondaryFlags_UseCluster);
                    tree_insert_sub(node, txt);
                    SP(_T("不使用簇链: %X"), dir->filename.GeneralSecondaryFlags_NoFatChain);
                    tree_insert_sub(node, txt);
                    memcpy(&filename[pos], dir->filename.FileName, 30);
                    pos += 15;
                    SP(_T("文件名:     %.15s"), dir->filename.FileName);
                    tree_insert_sub(node, txt);
                    SP(_T("文件名:     %s"), filename);
                    tree_insert_sub(node, txt);
                }
                break;
            }
            case 2: // 大写
            {
                SP(_T("序号:%02X 类型:%X 重要:%X 主要:%X 使用:%X 大小写映射"), i, dir->TypeCode, dir->TypeImportance, dir->TypeCategory, dir->TypeInUse);
                node = tree_insert_sub(file, txt);

                SP(_T("校验码:     %X"), dir->upcase.TableChecksum);
                tree_insert_sub(node, txt);
                SP(_T("簇号:       %X"), dir->upcase.FirstCluster);
                tree_insert_sub(node, txt);
                SP(_T("数据长度:   %I64X"), dir->upcase.DataLength);
                tree_insert_sub(node, txt);
                break;
            }
            case 3: // 卷标
            {
                SP(_T("序号:%02X 类型:%X 重要:%X 主要:%X 使用:%X 卷标"), i, dir->TypeCode, dir->TypeImportance, dir->TypeCategory, dir->TypeInUse);
                node = tree_insert_sub(file, txt);

                SP(_T("长度:       %X"), dir->label.CharacterCount);
                tree_insert_sub(node, txt);
                SP(_T("卷标:       %s"), dir->label.VolumeLabel);
                tree_insert_sub(node, txt);
                break;
            }
            case 5: // 文件
            {
                SP(_T("序号:%02X 类型:%X 重要:%X 主要:%X 使用:%X 文件"), i, dir->TypeCode, dir->TypeImportance, dir->TypeCategory, dir->TypeInUse);
                node = tree_insert_sub(file, txt);

                SP(_T("次项数量:   %X"), dir->file.SecondaryCount);
                tree_insert_sub(node, txt);
                SP(_T("校验码:     %X"), dir->file.SetChecksum);
                tree_insert_sub(node, txt);
                SP(_T("只读:       %X"), dir->file.FileAttributes_ReadOnly);
                tree_insert_sub(node, txt);
                SP(_T("隐藏:       %X"), dir->file.FileAttributes_Hidden);
                tree_insert_sub(node, txt);
                SP(_T("系统:       %X"), dir->file.FileAttributes_System);
                tree_insert_sub(node, txt);
                SP(_T("目录:       %X"), dir->file.FileAttributes_Directory);
                tree_insert_sub(node, txt);
                SP(_T("归档:       %X"), dir->file.FileAttributes_Archive);
                tree_insert_sub(node, txt);
                int ms = dir->file.Create10msIncrement * 10;
                int second = ms / 1000;
                ms %= 1000;
                SP(_T("创建日期:   %d-%02d-%02d %02d:%02d:%02d.%03d"),
                       dir->file.CreateTimestamp.Year + 1980,
                       dir->file.CreateTimestamp.Month,
                       dir->file.CreateTimestamp.Day,
                       dir->file.CreateTimestamp.Hour,
                       dir->file.CreateTimestamp.Minute,
                       dir->file.CreateTimestamp.DoubleSeconds * 2 + second,
                       ms);
                tree_insert_sub(node, txt);
                SP(_T("修改日期:   %d-%02d-%02d %02d:%02d:%02d"),
                       dir->file.LastModifiedTimestamp.Year + 1980,
                       dir->file.LastModifiedTimestamp.Month,
                       dir->file.LastModifiedTimestamp.Day,
                       dir->file.LastModifiedTimestamp.Hour,
                       dir->file.LastModifiedTimestamp.Minute,
                       dir->file.LastModifiedTimestamp.DoubleSeconds * 2);
                tree_insert_sub(node, txt);
                SP(_T("访问日期:   %d-%02d-%02d %02d:%02d:%02d"),
                       dir->file.LastAccessedTimestamp.Year + 1980,
                       dir->file.LastAccessedTimestamp.Month,
                       dir->file.LastAccessedTimestamp.Day,
                       dir->file.LastAccessedTimestamp.Hour,
                       dir->file.LastAccessedTimestamp.Minute,
                       dir->file.LastAccessedTimestamp.DoubleSeconds * 2);
                tree_insert_sub(node, txt);

                pos = 0;
                break;
            }
            default:
            {
                SP(_T("类型:%X"), dir->TypeCode);
                MessageBox(NULL, txt, g_title, MB_OK);
                break;
            }
        }
    }
}

void tree_ntfs_dbr(HTREEITEM parent, unsigned __int64 pos, p_ntfs_dbr dbr, int type)
{
    SP(_T("扇区:%08I64X DBR---------%s"), pos, part_type_text[type]);
    HTREEITEM node = tree_insert_sort(parent, txt);

    SP(_T("跳转指令:     %02X %02X %02X"), dbr->jump[0], dbr->jump[1], dbr->jump[2]);
    tree_insert_sub(node, txt);

    SP(_T("OEM           "));
    get_unicode_str(txt, dbr->oem, 8);
    tree_insert_sub(node, txt);

    SP(_T("扇区字节数:   %X"), dbr->sector_byte_size);
    tree_insert_sub(node, txt);

    SP(_T("簇扇区数量:   %X"), dbr->reserve_sector_count);
    tree_insert_sub(node, txt);

    SP(_T("保留扇区数:   %X"), dbr->reserve_sector_count);
    tree_insert_sub(node, txt);

    SP(_T("介质描述:     %X      F8-硬盘"), dbr->type);
    tree_insert_sub(node, txt);

    SP(_T("每磁道扇区数: %X"), dbr->header_sector_count);
    tree_insert_sub(node, txt);

    SP(_T("磁头数:       %X"), dbr->cluster_header_count);
    tree_insert_sub(node, txt);

    SP(_T("隐含扇区:     %X"), dbr->hide_count);
    tree_insert_sub(node, txt);

    SP(_T("扇区总数:     %I64X"), dbr->sector_count);
    tree_insert_sub(node, txt);

    SP(_T("$MFT簇号:     %I64X"), dbr->mft_cluster_id);
    tree_insert_sub(node, txt);

    SP(_T("$MFTmirr簇号: %I64X"), dbr->mirr_cluster_id);
    tree_insert_sub(node, txt);

    SP(_T("MFT的簇数:    %X"), dbr->mft_record_cluster_count);
    tree_insert_sub(node, txt);

    SP(_T("索引的簇数:   %X"), dbr->index_record_cluster_count);
    tree_insert_sub(node, txt);

    SP(_T("分区序列号:   %02X%02X%02X%02X%02X%02X%02X%02X"), dbr->part_id[0], dbr->part_id[1], dbr->part_id[2], dbr->part_id[3], dbr->part_id[4], dbr->part_id[5], dbr->part_id[6], dbr->part_id[7]);
    tree_insert_sub(node, txt);

    SP(_T("校验码:       %X"), dbr->check_id);
    tree_insert_sub(node, txt);

    SP(_T("签名:         %X"), dbr->sig);
    tree_insert_sub(node, txt);
}

void tree_ntfs_attr_body_30(HTREEITEM parent, p_ntfs_mft_attr_body_30 body_30)
{
    SP(_T("属性体-父MFT号:       %I64X"), body_30->father_mft_id);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-序列号:        %I64X"), body_30->seq_num);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-创建时间:      %I64X     1601-01-01 00:00:00起,单位100纳秒"), body_30->create_time);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-修改时间:      %I64X"), body_30->update_time);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-访问时间:      %I64X"), body_30->access_time);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-MFT时间:       %I64X"), body_30->mft_up_time);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-分配大小:      %I64X"), body_30->alloc_size);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-实际大小:      %I64X"), body_30->actual_size);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-只读:          %X"), body_30->attr_readonly);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-隐藏:          %X"), body_30->attr_hide);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-系统:          %X"), body_30->attr_system);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-存档:          %X"), body_30->attr_archive);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-设备:          %X"), body_30->attr_device);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-常规:          %X"), body_30->attr_convention);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-临时:          %X"), body_30->attr_temporary);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-稀疏:          %X"), body_30->attr_few);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-重解析点:      %X"), body_30->attr_reparse);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-压缩:          %X"), body_30->attr_compress);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-脱机:          %X"), body_30->attr_offline);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-索引:          %X"), body_30->attr_index);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-加密:          %X"), body_30->attr_encrypt);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-EAS:           %X"), body_30->eas);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-文件名长:      %X"), body_30->filename_len);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-命名空间:      %X        0-POSIX,1-Win32,2-DOS;3-Win32&DOS"), body_30->namespace);
    tree_insert_sub(parent, txt);

    SP(_T("属性体-文件名:        %s"), body_30->name);
    tree_insert_sub(parent, txt);
}

void tree_ntfs_index_entry(HTREEITEM parent, p_ntfs_mft_attr_index_entry entry)
{
    SP(_T("索引项:%s"), (entry->data_len > 0x40) ? entry->attr_body_30.name : _T(""));
    HTREEITEM node = tree_insert_sub(parent, txt);

    SP(_T("文件MFT号:            %I64X"), entry->mft_id);
    tree_insert_sub(node, txt);

    SP(_T("文件序列号:           %I64X"), entry->seq_num);
    tree_insert_sub(node, txt);

    SP(_T("索引项长度:           %X"), entry->len);
    tree_insert_sub(node, txt);

    SP(_T("数据长度:             %X"), entry->data_len);
    tree_insert_sub(node, txt);

    SP(_T("有子节点:             %X"), entry->flag_havechild);
    tree_insert_sub(node, txt);

    SP(_T("最后一项:             %X"), entry->flag_last);
    tree_insert_sub(node, txt);

    if (entry->data_len > 0x40)
    {
        tree_ntfs_attr_body_30(node, &(entry->attr_body_30));
    }
    else
    {
        SP(_T("数据:                 %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X"),
               entry->data[0],  entry->data[1],  entry->data[2],   entry->data[3],
               entry->data[4],  entry->data[5],  entry->data[6],   entry->data[7],
               entry->data[8],  entry->data[9],  entry->data[10],  entry->data[11],
               entry->data[12], entry->data[13], entry->data[144], entry->data[15]);
    tree_insert_sub(node, txt);
    }
}

void tree_ntfs_mft(HTREEITEM parent, unsigned __int64 pos, p_ntfs_mft mft, unsigned int count, bool mirr)
{
    HTREEITEM                node_file_data;
    HTREEITEM                node_attr_data;
    HTREEITEM                node_attr_sub;
    unsigned short          *name;
    p_ntfs_mft_head          head;
    p_ntfs_mft_attr          attr;
    p_ntfs_mft_attr_head_up  head_up;
    p_ntfs_mft_attr_head_at  head_at;
    p_ntfs_mft_attr_head_un  head_un;
    p_ntfs_mft_attr_body_10  body_10;
    p_ntfs_mft_attr_body_20  body_20;
    p_ntfs_mft_attr_body_30  body_30;
    p_ntfs_mft_attr_body_40  body_40;
    p_ntfs_mft_attr_body_50  body_50;
    p_ntfs_mft_attr_body_60  body_60;
    p_ntfs_mft_attr_body_70  body_70;
    p_ntfs_mft_attr_body_80  body_80;
    p_ntfs_mft_attr_body_90  body_90;
    p_ntfs_mft_attr_body_A0  body_A0;
    p_ntfs_mft_attr_body_B0  body_B0;
    p_ntfs_mft_attr_body_C0  body_C0;
    p_ntfs_mft_attr_body_D0  body_D0;
    p_ntfs_mft_attr_body_E0  body_E0;
    p_ntfs_mft_attr_body_F0  body_F0;
    p_ntfs_mft_attr_body_100 body_100;

    SP(_T("扇区:%08I64X %s"), pos, mirr ? _T("MIRR") : _T("MFT"));
    HTREEITEM file = tree_insert_sort(parent, txt);

    for (unsigned int i = 0; i < count; i++, mft++)
    {
        head = &(mft->head);
        attr = mft->attr;

        if (attr[2].head_up.type == 0x30) // 文件名属性
        {
            name = attr[2].body_30.name;
        }
        else if (attr[1].head_up.type == 0x30)
        {
            name = attr[1].body_30.name;
        }
        else
        {
            name = mft_file_text[i];
        }

        SP(_T("序号:%02X 参考号:%08I64X 名称:%s"), i, head->ref_id, name);
        node_file_data = tree_insert_sub(file, txt);

        SP(_T("固定值:              "));
        get_unicode_str(txt, head->name, 4);
        tree_insert_sub(node_file_data, txt);

        SP(_T("更新序列号偏移:      %X"), head->update_sn_pos);
        tree_insert_sub(node_file_data, txt);

        SP(_T("更新序列号大小:      %X"), head->update_sn_size);
        tree_insert_sub(node_file_data, txt);

        SP(_T("日志文件序列号:      %I64X"), head->logfile_sn);
        tree_insert_sub(node_file_data, txt);

        SP(_T("序列号:              %X"), head->sn);
        tree_insert_sub(node_file_data, txt);

        SP(_T("硬连接数:            %X"), head->hard_link_count);
        tree_insert_sub(node_file_data, txt);

        SP(_T("属性偏移:            %X"), head->attr_pos);
        tree_insert_sub(node_file_data, txt);

        SP(_T("使用中:              %X"), head->flag_used);
        tree_insert_sub(node_file_data, txt);

        SP(_T("目录:                %X"), head->flag_dir);
        tree_insert_sub(node_file_data, txt);

        SP(_T("记录实际大小:        %X"), head->size_actual);
        tree_insert_sub(node_file_data, txt);

        SP(_T("记录分配大小:        %X"), head->size_alloc);
        tree_insert_sub(node_file_data, txt);

        SP(_T("文件参考号:          %I64X"), head->ref_id);
        tree_insert_sub(node_file_data, txt);

        SP(_T("下一属性号:          %X"), head->next_attr_id);
        tree_insert_sub(node_file_data, txt);

        SP(_T("校验码:              %X"), head->check_id);
        tree_insert_sub(node_file_data, txt);

        SP(_T("更新序列号:          %X"), head->update_seq_id);
        tree_insert_sub(node_file_data, txt);

        for (int j = 0; ; j++, attr++)
        {
            name     = attr->name;
            head_up  = &(attr->head_up);
            head_at  = &(attr->head_at);
            head_un  = &(attr->head_un);
            body_10  = &(attr->body_10);
            body_20  = &(attr->body_20);
            body_30  = &(attr->body_30);
            body_40  = &(attr->body_40);
            body_50  = &(attr->body_50);
            body_60  = &(attr->body_60);
            body_70  = &(attr->body_70);
            body_80  = &(attr->body_80);
            body_90  = &(attr->body_90);
            body_A0  = &(attr->body_A0);
            body_B0  = &(attr->body_B0);
            body_C0  = &(attr->body_C0);
            body_D0  = &(attr->body_D0);
            body_E0  = &(attr->body_E0);
            body_F0  = &(attr->body_F0);
            body_100 = &(attr->body_100);

            if (head_up->type == 0)
            {
                break;
            }

            SP(_T("属性:                %s"), mft_attr_text[head_up->type>>4]);
            node_attr_data = tree_insert_sub(node_file_data, txt);

            SP(_T("类型:             %X"), head_up->type);
            tree_insert_sub(node_attr_data, txt);

            SP(_T("长度:             %X"), head_up->size);
            tree_insert_sub(node_attr_data, txt);

            SP(_T("非常驻:           %X"), head_up->unresident);
            tree_insert_sub(node_attr_data, txt);

            SP(_T("属性名长度:       %X"), head_up->name_len);
            tree_insert_sub(node_attr_data, txt);

            SP(_T("属性名偏移:       %X"), head_up->name_pos);
            tree_insert_sub(node_attr_data, txt);

            SP(_T("压缩:             %X"), head_up->flag_compress);
            tree_insert_sub(node_attr_data, txt);

            SP(_T("加密:             %X"), head_up->flag_encryption);
            tree_insert_sub(node_attr_data, txt);

            SP(_T("稀疏:             %X"), head_up->flag_few);
            tree_insert_sub(node_attr_data, txt);

            SP(_T("属性名:           %s"), attr->name);
            tree_insert_sub(node_attr_data, txt);

            if (0 == head_up->unresident) // 常驻
            {
                SP(_T("常驻-属性体长度:  %X"), head_at->body_size);
                tree_insert_sub(node_attr_data, txt);

                SP(_T("常驻-属性体位置:  %X"), head_at->body_pos);
                tree_insert_sub(node_attr_data, txt);

                SP(_T("常驻-索引标志:    %X"), head_at->index);
                tree_insert_sub(node_attr_data, txt);
            }
            else
            {
                SP(_T("非常驻-起始簇:    %I64X"), head_un->cluster_beg);
                tree_insert_sub(node_attr_data, txt);

                SP(_T("非常驻-结束簇:    %I64X"), head_un->cluster_beg);
                tree_insert_sub(node_attr_data, txt);

                SP(_T("非常驻-簇流位置:  %X"), head_un->datarun_pos);
                tree_insert_sub(node_attr_data, txt);

                SP(_T("非常驻-压缩单位:  %X        2的N次方"), head_un->compress_unit);
                tree_insert_sub(node_attr_data, txt);

                SP(_T("非常驻-分配大小:  %I64X"), head_un->size_alloc);
                tree_insert_sub(node_attr_data, txt);

                SP(_T("非常驻-实际大小:  %I64X"), head_un->size_actual);
                tree_insert_sub(node_attr_data, txt);

                SP(_T("非常驻-原始大小:  %I64X"), head_un->size_original);
                tree_insert_sub(node_attr_data, txt);

                SP(_T("非常驻-簇流数量:  %X"), head_un->rundata_count);
                tree_insert_sub(node_attr_data, txt);

                for (unsigned int k = 0; k < head_un->rundata_count; k++)
                {
                    SP(_T("非常驻-起始簇号%d: %I64X"), k, head_un->rundata_beg[k]);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("非常驻-使用簇量%d: %I64X"), k, head_un->rundata_len[k]);
                    tree_insert_sub(node_attr_data, txt);
                }

                SP(_T("非常驻-簇流:      %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X "),
                       head_un->data[head_up->name_len * 2 + 0],
                       head_un->data[head_up->name_len * 2 + 1],
                       head_un->data[head_up->name_len * 2 + 2],
                       head_un->data[head_up->name_len * 2 + 3],
                       head_un->data[head_up->name_len * 2 + 4],
                       head_un->data[head_up->name_len * 2 + 5],
                       head_un->data[head_up->name_len * 2 + 6],
                       head_un->data[head_up->name_len * 2 + 7],
                       head_un->data[head_up->name_len * 2 + 8],
                       head_un->data[head_up->name_len * 2 + 9],
                       head_un->data[head_up->name_len * 2 + 10],
                       head_un->data[head_up->name_len * 2 + 11],
                       head_un->data[head_up->name_len * 2 + 12],
                       head_un->data[head_up->name_len * 2 + 13],
                       head_un->data[head_up->name_len * 2 + 14],
                       head_un->data[head_up->name_len * 2 + 15]);
                tree_insert_sub(node_attr_data, txt);
            }

            switch (head_up->type)
            {
                case 0x10:
                {
                    SP(_T("属性体-创建时间:  %I64X     1601-01-01 00:00:00起,单位100纳秒"), body_10->create_time);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-修改时间:  %I64X"), body_10->update_time);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-访问时间:  %I64X"), body_10->access_time);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-MFT时间:   %I64X"), body_10->mft_up_time);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-只读:      %X"), body_10->attr_readonly);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-隐藏:      %X"), body_10->attr_hide);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-系统:      %X"), body_10->attr_system);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-存档:      %X"), body_10->attr_archive);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-设备:      %X"), body_10->attr_device);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-常规:      %X"), body_10->attr_convention);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-临时:      %X"), body_10->attr_temporary);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-稀疏:      %X"), body_10->attr_few);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-重解析点:  %X"), body_10->attr_reparse);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-压缩:      %X"), body_10->attr_compress);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-脱机:      %X"), body_10->attr_offline);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-索引:      %X"), body_10->attr_index);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-加密:      %X"), body_10->attr_encrypt);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-最大版本:  %X"), body_10->max_version);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-当前版本:  %X"), body_10->version);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-类ID:      %X"), body_10->class_id);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-所有者ID:  %X"), body_10->owner_id);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-安全ID:    %X"), body_10->secure_id);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-文件大小:  %I64X"), body_10->size);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-更新系列号:%I64X"), body_10->usn);
                    tree_insert_sub(node_attr_data, txt);
                    break;
                }
                case 0x20:
                {
                    p_ntfs_mft_attr_body_20_attr_list attr_list = body_20->attr_list;

                    for (int k = 0; k < body_20->attr_list_count; k++, attr_list++)
                    {
                        SP(_T("属性体-类型:      %X"), attr_list->type);
                        node_attr_sub = tree_insert_sub(node_attr_data, txt);

                        SP(_T("长度:           %X"), attr_list->len);
                        tree_insert_sub(node_attr_sub, txt);

                        SP(_T("属性名长度:     %X"), attr_list->name_len);
                        tree_insert_sub(node_attr_sub, txt);

                        SP(_T("属性名偏移:     %X"), attr_list->name_offset);
                        tree_insert_sub(node_attr_sub, txt);

                        SP(_T("簇号:           %I64X"), attr_list->begin_cluster);
                        tree_insert_sub(node_attr_sub, txt);

                        SP(_T("文件参考号:     %I64X"), attr_list->ref_id);
                        tree_insert_sub(node_attr_sub, txt);

                        SP(_T("属性ID:         %X"), attr_list->id);
                        tree_insert_sub(node_attr_sub, txt);

                        SP(_T("属性名:         %s"), attr_list->name);
                        tree_insert_sub(node_attr_sub, txt);
                    }
                    break;
                }
                case 0x30:
                {
                    tree_ntfs_attr_body_30(node_attr_data, body_30);
                    break;
                }
                case 0x40:
                {
                    SP(_T("属性体-对象ID:    %I64X"), body_40->object_id);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-原始对象ID:%I64X"), body_40->original_object_id);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-卷ID:      %I64X"), body_40->valume_id);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-域ID:      %I64X"), body_40->domain_id);
                    tree_insert_sub(node_attr_data, txt);
                    break;
                }
                case 0x50:
                {
                    break;
                }
                case 0x60:
                {
                    SP(_T("属性体-卷名:      %s"), body_60->volume_name);
                    tree_insert_sub(node_attr_data, txt);
                    break;
                }
                case 0x70:
                {
                    SP(_T("属性体-主版本号:  %X"), body_70->major_version);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-次版本号:  %X"), body_70->minor_version);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-坏区:      %X"), body_70->flag_bad);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-调整大小:  %X"), body_70->flag_resize);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-更新装载:  %X"), body_70->flag_reload);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-装载到NT4: %X"), body_70->flag_nt4);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-删除进USN: %X"), body_70->flag_usn);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-修复lds:   %X"), body_70->flag_lds);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-需chkdisk: %X"), body_70->flag_chkdisk);
                    tree_insert_sub(node_attr_data, txt);
                    break;
                }
                case 0x80:
                {
                    if (0 == head_up->unresident && head_at->body_size > 0) // 常驻
                    {
                        for (unsigned int k = 0; k < head_at->body_size; k += 16)
                        {
                            SP(_T("常驻-数据:        %04X:%02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X "),
                                  k,
                                  body_80->data[k + 0],
                                  body_80->data[k + 1],
                                  body_80->data[k + 2],
                                  body_80->data[k + 3],
                                  body_80->data[k + 4],
                                  body_80->data[k + 5],
                                  body_80->data[k + 6],
                                  body_80->data[k + 7],
                                  body_80->data[k + 8],
                                  body_80->data[k + 9],
                                  body_80->data[k + 10],
                                  body_80->data[k + 11],
                                  body_80->data[k + 12],
                                  body_80->data[k + 13],
                                  body_80->data[k + 14],
                                  body_80->data[k + 15]);
                            tree_insert_sub(node_attr_data, txt);
                        }
                    }
                    break;
                }
                case 0x90:
                {
                    SP(_T("索引头-索引类型:  %X"), body_90->head.type);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("索引头-分类规则:  %X"), body_90->head.rule);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("索引头-索引大小:  %X"), body_90->head.size_byte);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("索引头-索引簇数:  %X"), body_90->head.size_cluster);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("索引体-索引偏移:  %X"), body_90->body.pos_head);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("索引体-已用结尾:  %X"), body_90->body.pos_tail_used);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("索引体-缓冲区结尾:%X"), body_90->body.pos_tail_buf);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("索引体-指向该节点:%X"), body_90->body.flag);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("索引项数量:       %X"), body_90->entry_count);
                    tree_insert_sub(node_attr_data, txt);

                    p_ntfs_mft_attr_index_entry  entry = body_90->entry;

                    for (int k = 0; k < body_90->entry_count; k++, entry++)
                    {
                        tree_ntfs_index_entry(node_attr_data, entry);
                    }
                    break;
                }
                case 0xA0:
                {
                    if (0 == head_up->unresident)
                    {
                        SP(_T("常驻索引项可能出错了 %d %d"), i, j);
                        MessageBox(NULL, txt, g_title, MB_OK);
                    }
                    break;
                }
                case 0xB0:
                {
                    if (0 == head_up->unresident && head_at->body_size > 0) // 常驻
                    {
                        for (unsigned int k = 0; k < head_at->body_size; k += 16)
                        {
                            SP(_T("常驻-位图:        %04X:%02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X "),
                                  k,
                                  body_80->data[k + 0],
                                  body_80->data[k + 1],
                                  body_80->data[k + 2],
                                  body_80->data[k + 3],
                                  body_80->data[k + 4],
                                  body_80->data[k + 5],
                                  body_80->data[k + 6],
                                  body_80->data[k + 7],
                                  body_80->data[k + 8],
                                  body_80->data[k + 9],
                                  body_80->data[k + 10],
                                  body_80->data[k + 11],
                                  body_80->data[k + 12],
                                  body_80->data[k + 13],
                                  body_80->data[k + 14],
                                  body_80->data[k + 15]);
                            tree_insert_sub(node_attr_data, txt);
                        }
                    }
                    break;
                }
                case 0xC0:
                {
                    SP(_T("C0 %d %d"), i, j);
                    MessageBox(NULL, txt, g_title, MB_OK);

                    SP(_T("属性体-类型:      %X"), body_C0->type);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-大小:      %X"), body_C0->size);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-名字偏移:  %X"), body_C0->name_pos);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-名字长度:  %X"), body_C0->name_len);
                    tree_insert_sub(node_attr_data, txt);

                    SP(_T("属性体-名字:      %s"), body_C0->name);
                    tree_insert_sub(node_attr_data, txt);
                    break;
                }
                case 0x100:
                {
                    if (0 == head_up->unresident && head_at->body_size > 0) // 常驻
                    {
                        for (unsigned int k = 0; k < head_at->body_size; k += 16)
                        {
                            SP(_T("常驻-数据:        %04X:%02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X "),
                                  k,
                                  body_100->data[k + 0],
                                  body_100->data[k + 1],
                                  body_100->data[k + 2],
                                  body_100->data[k + 3],
                                  body_100->data[k + 4],
                                  body_100->data[k + 5],
                                  body_100->data[k + 6],
                                  body_100->data[k + 7],
                                  body_100->data[k + 8],
                                  body_100->data[k + 9],
                                  body_100->data[k + 10],
                                  body_100->data[k + 11],
                                  body_100->data[k + 12],
                                  body_100->data[k + 13],
                                  body_100->data[k + 14],
                                  body_100->data[k + 15]);
                            tree_insert_sub(node_attr_data, txt);
                        }
                    }
                    break;
                }
                default:
                {
                    SP(_T("未知属性:%X %d %d"), head_up->type, i, j);
                    MessageBox(NULL, txt, g_title, MB_OK);
                    break;
                }
            }
        }
    }
}

void tree_ntfs_root(HTREEITEM parent, unsigned __int64 pos, p_ntfs_index_alloc root)
{
    SP(_T("扇区:%08I64X ROOT"), pos);
    HTREEITEM sub, node = tree_insert_sort(parent, txt);

    p_ntfs_mft_attr_index_alloc_head head;
    p_ntfs_mft_attr_index_entry      entry;

    for (int i = 0; i < INDEX_ALLOC_SIZE && root->entry_count > 0; i++, root++)
    {
        head = &(root->head);
        entry = root->entry;

        SP(_T("索引分配%02X"), i);
        sub = tree_insert_sub(node, txt);

        SP(_T("INDX:                   "));
        get_unicode_str(txt, head->name, 4);
        tree_insert_sub(sub, txt);

        SP(_T("更新序列偏移:           %X"), head->update_sn_offset);
        tree_insert_sub(sub, txt);

        SP(_T("更新序列长度:           %X"), head->update_sn_size);
        tree_insert_sub(sub, txt);

        SP(_T("日志序列号:             %I64X"), head->logfile_sn);
        tree_insert_sub(sub, txt);

        SP(_T("VCN:                    %I64X"), head->vcn);
        tree_insert_sub(sub, txt);

        SP(_T("索引项偏移:             %X"), head->entry_offset);
        tree_insert_sub(sub, txt);

        SP(_T("索引项大小:             %X"), head->entry_size);
        tree_insert_sub(sub, txt);

        SP(_T("索引分配大小:           %X"), head->entry_size_alloc);
        tree_insert_sub(sub, txt);

        SP(_T("有子节点:               %X"), head->flag);
        tree_insert_sub(sub, txt);

        SP(_T("更新序列:               %X"), head->update_sn);
        tree_insert_sub(sub, txt);

        SP(_T("更新序列数组:           %02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X"),
            head->update_array[0],
            head->update_array[1],
            head->update_array[2],
            head->update_array[3],
            head->update_array[4],
            head->update_array[5],
            head->update_array[6],
            head->update_array[7],
            head->update_array[8],
            head->update_array[9],
            head->update_array[10],
            head->update_array[11],
            head->update_array[12],
            head->update_array[13],
            head->update_array[14],
            head->update_array[15]);
        tree_insert_sub(sub, txt);

        for (unsigned int i = 0; i < root->entry_count; i++, entry++)
        {
            tree_ntfs_index_entry(sub, entry);
        }
    }
}

void tree_file_system(p_disk_info disk)
{
    HTREEITEM node_part;
    p_part_table_item item;
    HTREEITEM node_disk = disk->tree_node;
    p_mbr_list_node mbr_list = &(disk->mbr_list);

    do
    {
        SP(_T("扇区:%08I64X %s---------ID:%02X%02X%02X%02X"), mbr_list->pos, (mbr_list->pos == 0) ? _T("MBR") : _T("EBR"),
           mbr_list->mbr.disk_id[0], mbr_list->mbr.disk_id[1], mbr_list->mbr.disk_id[2], mbr_list->mbr.disk_id[3]);

        node_part = tree_insert_sub(node_disk, txt);
        item = mbr_list->mbr.part_table;

        for (int i = 0; i < 4 && item->type != 0; i++, item++)
        {
            SP(_T("分区%d 系统:%02x 偏移:%08X 大小:%08X 类型:%s"), i, item->system, item->offset, item->size, part_type_text[item->type]);

            tree_insert_sub(node_part, txt);

            if (item->type == 0x05 || item->type == 0x0F) // 跳过扩展分区
            {
                continue;
            }

            switch (item->type)
            {
                case 0x06:
                case 0x16: // FAT16
                {
                    p_fat16_info fat16 = &(mbr_list->file_system[i].fat16);
                    tree_fat16_dbr(node_disk,    fat16->pos_dbr,     &(fat16->dbr), item->type);
                    tree_fat16_dir(node_disk,    fat16->pos_dir,     fat16->dir);
                    tree_data_bit16(node_disk,   fat16->pos_fat1,    _T("FAT1"), fat16->fat1, FAT_SIZE);
                    tree_data_bit16(node_disk,   fat16->pos_fat2,    _T("FAT2"), fat16->fat2, FAT_SIZE);
                    tree_data_cluster(node_disk, fat16->pos_cluster, fat16->dbr.cluster_sector_size, fat16->cluster, false);
                    break;
                }
                case 0x0B: // FAT32
                {
                    p_fat32_info fat32 = &(mbr_list->file_system[i].fat32);
                    tree_fat32_dbr(node_disk,    fat32->pos_dbr,      &(fat32->dbr), item->type);
                    tree_fat32_dir(node_disk,    fat32->pos_dir,      fat32->dir);
                    tree_fat32_fsinfo(node_disk, fat32->pos_fsinfo,   &(fat32->fsinfo));
                    tree_data_bit32(node_disk,   fat32->pos_fat1,     _T("FAT1"), fat32->fat1, FAT_SIZE);
                    tree_data_bit32(node_disk,   fat32->pos_fat2,     _T("FAT2"), fat32->fat2, FAT_SIZE);
                    tree_data_cluster(node_disk, fat32->pos_cluster,  fat32->dbr.cluster_sector_size, fat32->cluster, false);
                    break;
                }
                case 0x01: // EXFAT
                {
                    p_exfat_info exfat = &(mbr_list->file_system[i].exfat);
                    tree_exfat_dbr(node_disk,    exfat->pos_dbr,    &(exfat->dbr), item->type);
                    tree_exfat_dir(node_disk,    exfat->pos_dir,     exfat->dir);
                    tree_data_bit32(node_disk,   exfat->pos_fat1,    _T("FAT1"),   exfat->fat1,   FAT_SIZE);
                    tree_data_bit32(node_disk,   exfat->pos_fat2,    _T("FAT2"),   exfat->fat2,   FAT_SIZE);
                    tree_data_bit8(node_disk,    exfat->pos_bitmap,  _T("MAPBIT"), exfat->bitmap, BITMAP_SIZE);
                    tree_data_bit16(node_disk,   exfat->pos_upcase,  _T("UPCASE"), exfat->upcase, UPCASE_SIZE);
                    tree_data_cluster(node_disk, exfat->pos_cluster, (unsigned char)pow(2, exfat->dbr.SectorsPerClusterShift), exfat->cluster, false);
                    break;
                }
                case 0x07: // NTFS
                {
                    p_ntfs_info ntfs = &(mbr_list->file_system[i].ntfs);
                    tree_ntfs_dbr(node_disk,     ntfs->pos_dbr,      &(ntfs->dbr), item->type);
                    tree_ntfs_mft(node_disk,     ntfs->pos_mft,      ntfs->mft, ntfs->mft_count, false);
                    tree_ntfs_mft(node_disk,     ntfs->pos_mirr,     ntfs->mirr, MIRR_SIZE, true);
                    tree_ntfs_root(node_disk,    ntfs->pos_root,     ntfs->root);
                    tree_data_bit8(node_disk,    ntfs->pos_bitmap,   _T("BITMAP"), ntfs->bitmap, BITMAP_SIZE);
                    tree_data_bit16(node_disk,   ntfs->pos_upcase,   _T("UPCASE"), ntfs->upcase, UPCASE_SIZE);
                    tree_data_cluster(node_disk, ntfs->pos_cluster,  ntfs->dbr.cluster_sector_size, ntfs->cluster, true);
                    break;
                }
            }
        }
    }
    while (mbr_list = mbr_list->next);
}

void tree_disk_info()
{
    g_disk_count = get_disk_info(SIZEOF(g_disk), g_disk);

    for (unsigned int i = 0; i < g_disk_count; i++)
    {
        SP(_T("总线:%s 媒介:%s 产品:%s"), bus_type_text[g_disk[i].bus_type], media_type_text[g_disk[i].media_type], g_disk[i].product_id);

        g_disk[i].tree_node = tree_insert_sub(TVI_ROOT, txt);

        tree_file_system(&g_disk[i]);
    }
}

/**
 * \brief   创建消息处理函数
 * \param   [in]  HWND wnd 窗体句柄
 * \return  无
 */
void on_create(HWND wnd)
{
    g_tree = CreateWindow(WC_TREEVIEW,
                          _T("Tree View"),
                          WS_CHILD | WS_VISIBLE | TVS_HASLINES| TVS_HASBUTTONS |
                          TVS_LINESATROOT | WS_EX_ACCEPTFILES,
                          0, 0,
                          100, 100,
                          wnd,
                          NULL,
                          NULL,
                          NULL);

    SendMessage(g_tree, WM_SETFONT, (WPARAM)g_font, (LPARAM)TRUE);

    tree_disk_info();
}

/**
 * \brief   窗体类消息处理回调函数
            当用户点击窗体上的关闭按钮时,
            系统发出WM_CLOSE消息,DefWindowProc内执行DestroyWindow关闭窗口,DestroyWindow内发送WM_DESTROY消息,
            需要自己调用PostQuitMessage关闭应用程序,PostQuitMessage内发出WM_QUIT消息来关闭消息循环
 * \param   [in]  HWND   wnd    窗体句柄
 * \param   [in]  UINT   msg    消息ID
 * \param   [in]  WPARAM w      消息参数
 * \param   [in]  LPARAM l      消息参数
 * \return  LRESULT 消息处理结果，它与发送的消息有关
 */
LRESULT CALLBACK window_proc(HWND wnd, UINT msg, WPARAM w, LPARAM l)
{
    switch(msg)
    {
        case WM_CREATE:     on_create(wnd);                                         break;
        case WM_DESTROY:    PostQuitMessage(0);                                     break;
        case WM_SIZE:       MoveWindow(g_tree, 0, 0, LOWORD(l), HIWORD(l), TRUE);   break;
        //case WM_NOTIFY: LPNMTREEVIEW; break;
    }

    return DefWindowProc(wnd, msg, w, l);
}

/**
 * \brief   窗体类程序主函数
 * \param   [in]  HINSTANCE hInstance       当前实例句柄
 * \param   [in]  HINSTANCE hPrevInstance   先前实例句柄
 * \param   [in]  LPSTR     lpCmdLine       命令行参数
 * \param   [in]  int       nCmdShow        显示状态(最小化,最大化,隐藏)
 * \return  int 程序返回值
 */
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // 窗体大小
    int cx = 800;
    int cy = 600;

    // 树控件字体
    LOGFONT font = { 18, 8, 0, 0, 400, 0, 0, 0, DEFAULT_CHARSET, 0, 0, 0, 0, _T("Courier New") };
    g_font = CreateFontIndirect(&font);

    // 窗体类
    WNDCLASS wc = { CS_HREDRAW | CS_VREDRAW, window_proc, 0, 0, hInstance, 0, 0, 0, 0, g_title };
    RegisterClass(&wc);

    // 创建窗体
    HWND wnd = CreateWindow(wc.lpszClassName,                           // 类名称
                            g_title,                                    // 窗体名称
                            WS_OVERLAPPEDWINDOW,                        // 窗体属性
                            (GetSystemMetrics(SM_CXSCREEN) - cx) / 2,   // 窗体位置
                            (GetSystemMetrics(SM_CYSCREEN) - cy) / 2,   // 窗体居中
                            cx, cy,                                     // 窗体大小
                            NULL,                                       // 父窗句柄
                            NULL,                                       // 菜单句柄
                            hInstance,                                  // 实例句柄
                            NULL);                                      // 参数,给WM_CREATE的lParam的CREATESTRUCT

    // 显示窗体
    ShowWindow(wnd, SW_SHOWNORMAL);

    // 重绘窗体
    UpdateWindow(wnd);

    // 消息体
    MSG msg;

    // 消息循环,从消息队列中取得消息,直到WM_QUIT时退出
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg); // 将WM_KEYDOWN和WM_KEYUP转换为一条WM_CHAR消息
        DispatchMessage(&msg);  // 分派消息到窗口,内部调用窗体消息处理回调函数
    }

    return (int)msg.lParam;
}