Args->num_of_blocks_last); i++){
        bytes_read = fread(buffer, sizeof(char), BLOCKSIZE, file);