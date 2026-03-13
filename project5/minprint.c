#include "minprint.h"

void print_ptable(ptable_entry* ptable){
    printf("PTABLE-------------\n");
    
    printf("bootind\t");
    printf("s_head\t");
    printf("s_sec\t");
    printf("s_cyl\t");
    printf("type\t");
    printf("e_head\t");
    printf("e_sec\t");
    printf("e_cyl\t");
    printf("lFirst\t");
    printf("size\t");
    printf("\n");
    for(int i = 0; i < PMAX; i++){
        printf("%#04X\t", ptable[i].bootind);
        printf("%d\t", ptable[i].start_head);
        printf("%d\t", ptable[i].start_sec);
        printf("%d\t", ptable[i].start_cyl);
        printf("%#04X\t", ptable[i].type);
        printf("%d\t", ptable[i].end_head);
        printf("%d\t", ptable[i].end_sec);
        printf("%d\t", ptable[i].end_cyl);
        printf("%d\t", ptable[i].lFirst);
        printf("%d\t", ptable[i].size);
        printf("\n");
    }
}

void print_sblock(superblock sblock){
    printf("\nSuperblock Contents:\n");
    printf("Stored Fields:\n");

    printf("  %-15s %10d\n", "ninodes", sblock.ninodes);
    printf("  %-15s %10d\n", "i_blocks", sblock.i_blocks);
    printf("  %-15s %10d\n", "z_blocks", sblock.z_blocks);
    printf("  %-15s %10d\n", "firstdata", sblock.firstdata);
    printf("  %-15s %10d"  , "log_zone_size", sblock.log_zone_size);
    printf(" (zone size: %d)\n", sblock.blocksize << sblock.log_zone_size);
    
    printf("  %-15s %10u\n", "max_file", sblock.max_file);
    printf("  %-15s %10x\n", "magic", sblock.magic);
    printf("  %-15s %10d\n", "zones", sblock.zones);
    printf("  %-15s %10d\n", "blocksize", sblock.blocksize);
    printf("  %-15s %10d\n", "subversion", sblock.subversion);
}

void print_inode(inode n){
    char perm[11];
    strncpy(perm, LS_STRING, 11);
    perm[11] = '\0';
    
    if(IS_DIR(n.mode)){
        perm[0] = 'd';
    }else{
        perm[0] = '-';
    }
    
    for(int i = 0; i < 9; i++){
        if((n.mode & (1<<i)) != 1<<i){
            perm[9-i] = '-';
        }
    }
    
    time_t atime_v = (time_t)n.atime;
    time_t mtime_v = (time_t)n.mtime;
    time_t ctime_v = (time_t)n.ctime;
    
    printf("\nFile inode:\n");
    
    printf("  %-15s %10x"  , "uint16_t mode", n.mode);
    printf(" (%s)\n", perm);
    printf("  %-15s %10d\n", "uint16_t links", n.links);
    printf("  %-15s %10d\n", "uint16_t uid", n.uid);
    printf("  %-15s %10d\n", "uint16_t gid", n.gid);
    printf("  %-15s %10u\n", "uint32_t size", n.size);
    
    printf("  %-15s %10u", "uint32_t atime", n.atime);
    printf(" --- %s", ctime(&atime_v));
    printf("  %-15s %10u", "uint32_t mtime", n.mtime);
    printf(" --- %s", ctime(&mtime_v));
    printf("  %-15s %10u", "uint32_t ctime", n.ctime);
    printf(" --- %s", ctime(&ctime_v));
    
    printf("\n Direct zones:\n");
    for(int i = 0; i < DIRECT_ZONES; i++){
        printf("\t\tzone[%d]\t=\t\t%d\n", i, n.zone[i]);
    }
    printf("  uint32_t\tindirect\t\t%d\n", n.indirect);
    printf("  uint32_t\tdouble\t\t\t%d\n", n.two_indirect);
}

void print_dir(dirent *d, int n){
    printf("DIRENT-------------\n");
    printf("inode\tname\n");
    
    char buff[61];
    memset(buff, 0, 61);
    for(int i = 0; i < n; i++){
        strncpy(buff, d[i].name, 60);
        buff[60] = '\0';
        if(buff[0] != 0){
            printf("%d\t%s\n", d[i].inode_num, buff);
        }
    }
    printf("\n");
}