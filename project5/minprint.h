#ifndef MINPRINT_H
#define MINPRINT_H

#include "minls.h"
#include "time.h"

void print_ptable(ptable_entry* ptable);
void print_sblock(superblock sblock);
void print_inode(inode n);
void print_dirent(dirent* d);
void print_dir(dirent* d, int n);

#endif