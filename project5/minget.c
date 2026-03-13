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
        perror("No valid partition table found in disk\n");
        return -1;
    }
    
    if(verbose){
        print_ptable(ptable);
    }
    
    if(ptable[p].type != PTYPE_MINIX){
        perror("Partition is not a MINIX filesystem\n");
        return -1;
    }
    
    poffset = SECTOR_SIZE * ptable[p].lFirst;
    if(s == -1){
        return poffset;
    }
    
    //read subpartition table
    pread(fd, &ptable, sizeof(ptable), PTABLE_OFFSET + poffset);
    pread(fd, &psig, sizeof(psig), PSIG_OFFSET + poffset);
    if(psig != PSIG){
        //no subpartition table
        perror("No valid partition table found in subpartition\n");
        return -1;
    }
    
    if(verbose){
        print_ptable(ptable);
    }
    
    if(ptable[s].type != PTYPE_MINIX){
        perror("Subpartition is not a MINIX filesystem\n");
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
    if(z_i < DIRECT_ZONES){
        //check direct
        bytes_read = read_zone(buff, n.zone, z_i);
    }else{
        //read indirect
        if(n.indirect != 0){
            uint32_t indirect[blocksize/sizeof(uint32_t)];
            pread(fd, indirect, blocksize, fs_offset + (n.indirect*zonesize));
            
            bytes_read = read_zone(buff, indirect, z_i-DIRECT_ZONES);
        }
    }
    if(bytes_read == 0){
        return 0;
    }else{
        memcpy(dest, buff, size);
        return size;
    }
}

int main(int argc, char** argv){
    //parse command line arguments
    verbose = FALSE;
    int partition = -1;;
    int subpartition = -1;
    
    bool fn_found = FALSE;
    bool src_found = FALSE;
    char filename[MAX_FILENAME];
    filename[0] = -1;
    char src_path[MAX_FILEPATH];
    src_path[0] = -1;
    char dest_path[MAX_FILEPATH];
    dest_path[0] = -1;
    
    int ret;
    ret = getopt(argc, argv, "-vhp:s:");
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
                    if(!src_found){
                        strncpy(src_path, optarg, MAX_FILEPATH);
                        src_found = TRUE;
                    }else{
                        strncpy(dest_path, optarg, MAX_FILEPATH);
                    }
                }
                break;
            default:
                perror("bad argument");
                return 1;
        }
        ret = getopt(argc, argv, "-vp:s:");
    }
    
    //validate arguments
    if(partition > 3){
        perror("Invalid partition number\n");
        return 1;
    } 
    if(subpartition > 3){
        perror("Invalid subpartition number\n");
        return 1;
    }   
    
    //parse src path
    if(src_path[0] == -1){
        perror("Missing src file path\n");
        return 1;
    }
    
    //open dest file
    int dest_fd;
    if(dest_path[0] == -1){
        dest_fd = STDOUT_FILENO;
    }else{
        dest_fd = open(dest_path, O_WRONLY | O_TRUNC | O_CREAT, 
                        S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    }
    
    //open disk file
    fd = open(filename, O_RDONLY);
    if(fd == -1){
        printf("Cannot open file %s: ", filename);
        switch(errno){
        case ENOENT:
            perror("File does not exist\n");
            break;
        case EACCES:
            perror("Permission denied\n");
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
    
    //read file system superblock
    superblock sblock;
    pread(fd, &sblock, sizeof(sblock), fs_offset + SBLOCK_OFFSET);
    if(sblock.magic != SBLOCK_MAGIC){
        perror("Invalid superblock\n");
        close(fd);
        return 1;
    }
    if(verbose){
        print_sblock(sblock);
    }
    
    blocksize = sblock.blocksize;
    zonesize = blocksize << sblock.log_zone_size;
    
    int inode_block_offset = 2 + sblock.i_blocks + sblock.z_blocks;
    int inode_byte_offset = blocksize * inode_block_offset;
    
    //read inode list
    inode inode_list[sblock.ninodes];
    pread(fd, &inode_list, sizeof(inode_list), fs_offset + inode_byte_offset);
    inode cur_inode = inode_list[0];
    
    char path_list[32][32];
    int depth = 0;
    char *arg;
    arg = strtok(src_path, "/");
    if(arg == NULL){
        perror("Missing file path\n");
        close(fd);
        return 1;
    }
    while(arg != NULL){
        strcpy(path_list[depth], arg);
        depth++;
        arg = strtok(NULL, "/");
    }
    
    //traverse directories
    dirent cur_dir_z[64];
    memset(cur_dir_z, 0, sizeof(cur_dir_z));
    
    char cur_path[MAX_FILEPATH];
    memset(cur_path, 0, MAX_FILEPATH);
    cur_path[0] = '\0';
    
    for(int d = 0; d < depth; d++){
        int found = FALSE;
        int z = 0; //specific zone of the directory we are currently scanning
        
        while(!found){
            //check if cur_inode is a directory
            if(IS_FILE(cur_inode.mode)){
                perror("Not a directory\n");
                close(fd);
                return 1;
            }
            
            //scan through all zones of this directory
            int bytes_read;
            bytes_read = inode_read(
                    (uint8_t*)&cur_dir_z, cur_inode, z, sizeof(dirent)*64);
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
                    cur_inode = inode_list[cur_dir_z[i].inode_num - 1];
                }
            }
            z++;
        }
        
        if(!found){
            strcat(cur_path, path_list[d]);
            perror("File not found\n");
            close(fd);
            return 1;
        }
        
        if(d != depth-1){
            strcat(cur_path, "/");
        }
    }
    
    if(verbose){
        print_inode(cur_inode);
    }
    
    if(IS_DIR(cur_inode.mode)){
        perror("Target is not a file\n");
        return 1;
    }
    
    //write out file
    uint8_t w_buff[zonesize];
    //scan through all zones of this directory
    int bytes_read;
    int bytes_written = 0;
    int z = 0;
    
    bytes_read = inode_read(w_buff, cur_inode, z, zonesize);
    while(cur_inode.size - bytes_written > zonesize){
        write(dest_fd, w_buff, zonesize);
        bytes_written += zonesize;
        z++;
        bytes_read = inode_read(w_buff, cur_inode, z, zonesize);
    }
    write(dest_fd, w_buff, cur_inode.size - bytes_written);
    
    return 0;
}