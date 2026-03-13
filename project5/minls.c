#include "minls.h"
#include "minprint.h"

bool verbose;
int fd, fs_offset, blocksize, zonesize;

int find_fs_offset(int p, int s){
    if(p == -1){
        return 0;
    }
    
    ptable_entry ptable[4];
    uint16_t psig;
    int poffset = 0;
    
    //read partition table
    pread(fd, &ptable, sizeof(ptable), PTABLE_OFFSET);
    pread(fd, &psig, sizeof(psig), PSIG_OFFSET);
    if(psig != PSIG){
        //no partition table
        printf("No valid partition table found in disk\n");
        return -1;
    }
    
    if(verbose){
        print_ptable(ptable);
    }
    
    poffset = SECTOR_SIZE * ptable[p].lFirst;
    if(s == -1){
        if(ptable[p].type != PTYPE_MINIX){
            printf("Partition is not a MINIX filesystem\n");
            return -1;
        }else{
            return poffset;
        }
    }
    
    //read subpartition table
    pread(fd, &ptable, sizeof(ptable), PTABLE_OFFSET + poffset);
    pread(fd, &psig, sizeof(psig), PSIG_OFFSET + poffset);
    if(psig != PSIG){
        //no subpartition table
        printf("No valid partition table found in subpartition\n");
        return -1;
    }
    
    if(verbose){
        print_ptable(ptable);
    }
    
    if(ptable[s].type != PTYPE_MINIX){
        printf("Subpartition is not a MINIX filesystem\n");
        return -1;
    }
    return (SECTOR_SIZE * ptable[s].lFirst);
}

int read_zone(uint8_t *dest, uint32_t *zone_list, int z_i){
    if(zone_list[z_i] == 0){
        memset(dest, 0, zonesize);
        return 0;
    }else{
        pread(fd, dest, zonesize, fs_offset + (zone_list[z_i]*zonesize));
        return zonesize;
    }
}

int inode_read(uint8_t* dest, inode n, int z_i, int size){
    //Reads from a zone from inode n
    uint8_t buff[zonesize];
    int bytes_read = 0;
    if(z_i < 8){
        //check direct
        bytes_read = read_zone(buff, n.zone, z_i);
    }else{
        //read indirect
        if(n.indirect != 0){
            uint32_t indirect[blocksize/sizeof(uint32_t)];
            pread(fd, indirect, blocksize, fs_offset + (n.indirect*zonesize));
            
            bytes_read = read_zone(buff, indirect, z_i);
        }
    }
    if(bytes_read == 0){
        return 0;
    }else{
        memcpy(dest, buff, size);
        return size;
    }
}

void inode_ls(inode n, char* path){
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
    printf("%s ", perm);
    printf("%*d ", 9, n.size);
    printf("%s\n", path);
}

void dir_ls(inode n, char* path, inode* inode_list){
    dirent dir_z[64];
    //scan through all zones of this directory
    int bytes_read;
    int z = 0;
    
    bytes_read = inode_read((uint8_t*)&dir_z, n, z, sizeof(dirent)*64);
    while(bytes_read != 0){
        //scan through all entries in this data zone
        for(int i = 0; i < 64; i++){
            char* curent_name = dir_z[i].name;
            int curent_inode = dir_z[i].inode_num;
            if(curent_name[0] != 0 && curent_inode != 0){
                inode_ls(inode_list[curent_inode-1], curent_name);
            }
        }
        z++;
        bytes_read = inode_read((uint8_t*)&dir_z, n, z, sizeof(dirent)*64);
    }
}

int main(int argc, char** argv){
    //parse command line arguments
    verbose = FALSE;
    int partition = -1;;
    int subpartition = -1;
    
    bool fn_found = FALSE;
    char filename[MAX_FILENAME];
    filename[0] = -1;
    char path[MAX_FILEPATH];
    path[0] = -1;
    
    int ret;
    ret = getopt(argc, argv, "-vp:s:");
    while(ret != -1){
        switch(ret){
            case 'v':
                verbose = TRUE;
                break;
            case 'p':
                partition = atoi(optarg);
                break;
            case 's':
                subpartition = atoi(optarg);
                break;
            case 1:
                if(!fn_found){
                    strncpy(filename, optarg, MAX_FILENAME);
                    fn_found = TRUE;
                }else{
                    strncpy(path, optarg, MAX_FILEPATH);
                }
                break;
        }
        ret = getopt(argc, argv, "-vp:s:");
    }
    
    //validate arguments
    if(partition > 3){
        printf("Invalid partition number\n");
        return 1;
    } 
    if(subpartition > 3){
        printf("Invalid subpartition number\n");
        return 1;
    }    
    if(filename[0] == -1){
        printf("usage: minls [ -v ] [ -p num [ -s num ] ] imagefile [ path ]\n"
               "Options:\n"
               "-p part --- select partition for filesystem (default: none)\n"
             "-s sub --- select subpartition for filesystem (default: none)\n"
               "-v verbose --- increase verbosity level\n"
        );
        return 0;
    }
    
    //open file
    fd = open(filename, O_RDONLY);
    if(fd == -1){
        printf("Cannot open file %s: ", filename);
        switch(errno){
        case ENOENT:
            printf("File does not exist\n");
            break;
        case EACCES:
            printf("Permission denied\n");
            break;
        }
        return 1;
    }
    
    //get file system offset
    fs_offset = find_fs_offset(partition, subpartition);
    if(fs_offset == -1){
        close(fd);
        return 1;
    }
    
    if(verbose){
        printf("fs_offset=%d\n", fs_offset);
    }
    
    //read file system superblock
    superblock sblock;
    pread(fd, &sblock, sizeof(sblock), fs_offset + SBLOCK_OFFSET);
    if(sblock.magic != SBLOCK_MAGIC){
        printf("Invalid superblock\n");
        close(fd);
        return 1;
    }
    print_sblock(sblock);
    
    blocksize = sblock.blocksize;
    zonesize = blocksize << sblock.log_zone_size;
    
    int inode_block_offset = 2 + sblock.i_blocks + sblock.z_blocks;
    int inode_byte_offset = blocksize * inode_block_offset;
    
    //read inode list
    inode inode_list[sblock.ninodes];
    pread(fd, &inode_list, sizeof(inode_list), fs_offset + inode_byte_offset);
    inode cur_inode = inode_list[0];
    
    //parse destination path
    if(path[0] == -1){
        print_inode(cur_inode);
        printf("/:\n");
        dir_ls(cur_inode, "", inode_list);
        return 0;
    }
    
    char path_list[32][32];
    int depth = 0;
    char *arg;
    arg = strtok(path, "/");
    if(arg == NULL){
        print_inode(cur_inode);
        printf("/:\n");
        dir_ls(inode_list[0], "", inode_list);
        return 0;
    }
    while(arg != NULL){
        strcpy(path_list[depth], arg);
        depth++;
        arg = strtok(NULL, "/");
    }
    
    //traverse directories
    dirent cur_dir_z[64];
    
    char cur_path[MAX_FILEPATH];
    cur_path[0] = '/';
    
    for(int d = 0; d < depth; d++){
        int found = FALSE;
        int z = 0; //specific zone of the directory we are currently scanning
        
        while(!found){
            //check if cur_inode is a directory
            if(IS_FILE(cur_inode.mode)){
                printf("%s is not a directory\n", cur_path);
                close(fd);
                return 0;
            }
            
            //scan through all zones of this directory
            int bytes_read;
            bytes_read = inode_read((uint8_t*)&cur_dir_z, cur_inode, z, sizeof(dirent)*64);
            if(bytes_read == 0){
                //end of data zones
                break;
            }
         
            for(int i = 0; i < 64; i++){
                //scan through all dirent in this data zone
                if(strcmp(path_list[d], cur_dir_z[i].name) == 0
                    && cur_dir_z[i].inode_num != 0){
                    found = TRUE;
                    strcat(cur_path, cur_dir_z[i].name);
                    cur_inode = inode_list[cur_dir_z[i].inode_num - 1];//1-index
                }
            }
            z++;
        }
        
        if(!found){
            strcat(cur_path, path_list[d]);
            printf("%s not found\n", cur_path);
            close(fd);
            return 0;
        }
        
        if(d != depth-1){
            strcat(cur_path, "/");
        }
    }
    
    if(verbose){
        print_inode(cur_inode);
    }
    
    printf("%s:\n", cur_path);
    if(IS_DIR(cur_inode.mode)){
        dir_ls(cur_inode, cur_path, inode_list);
    }else{
        inode_ls(cur_inode, cur_path);
    }
    return 0;
}