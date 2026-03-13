#ifndef MINLS_H
#define MINLS_H

#define _XOPEN_SOURCE 500

typedef enum{
    TRUE = 1,
    FALSE = 0
}bool;

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <math.h>

#include <getopt.h>
extern char *optarg;
extern int optind, opterr, optopt;

#define MAX_FILENAME 64
#define MAX_FILEPATH 256

#define SECTOR_SIZE 512

//-PARTITION_TABLE-//
#define PTABLE_OFFSET 0x1BE
#define PSIG_OFFSET 0x1FE
#define PSIG 0xAA55
#define PMAX 4
#define PTYPE_MINIX 0x81

typedef struct __attribute__((packed)) ptable_entry{
    uint8_t bootind;
    uint8_t start_head;
    uint8_t start_sec;
    uint8_t start_cyl;
    uint8_t type;
    uint8_t end_head;
    uint8_t end_sec;
    uint8_t end_cyl;
    uint32_t lFirst;
    uint32_t size;
}ptable_entry;

//-SUPERBLOCK-//
#define SBLOCK_OFFSET 1024
#define SBLOCK_MAGIC 0x4D5A

typedef struct __attribute__((packed)) superblock{
    uint32_t ninodes;
    uint16_t pad1;
    int16_t  i_blocks;
    int16_t  z_blocks;
    uint16_t firstdata;
    int16_t  log_zone_size;
    int16_t  pad2;
    uint32_t max_file;
    uint32_t zones;
    int16_t  magic;
    int16_t  pad3;
    uint16_t blocksize;
    uint8_t  subversion;
}superblock;

//-INODE-//
#define DIRECT_ZONES 7
#define INODE_SIZE 64

typedef struct __attribute__((packed)) inode{
    uint16_t mode; 
    uint16_t links;
    uint16_t uid;
    uint16_t gid;
    uint32_t size;
    int32_t  atime;
    int32_t  mtime;
    int32_t  ctime;
    uint32_t zone[DIRECT_ZONES];
    uint32_t indirect;
    uint32_t two_indirect;
    uint32_t unused;
}inode;

#define FTYPE_MASK 0xF000
#define FTYPE_DIR  0x4000
#define FTYPE_FILE 0x8000

#define IS_DIR(mode) ((mode & FTYPE_MASK) == FTYPE_DIR)
#define IS_FILE(mode) ((mode & FTYPE_MASK) == FTYPE_FILE)

#define LS_STRING "-rwxrwxrwx"

//-DIRECTORY_ENTRY-//
#define NAME_SIZE 60

typedef struct __attribute__((packed)) dirent{
    uint32_t inode_num;
    char name[NAME_SIZE];
}dirent;


#endif