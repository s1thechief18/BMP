#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCKSIZE 10000

int main() {
    FILE* file = fopen("Front.jpeg","r");
    FILE* file_out = fopen("Back.jpeg", "w");
    // FILE* file = fopen("file.txt","rb");

    if(file != NULL){
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        long cursor = 0;
        fseek(file, 0, SEEK_SET); 

        printf("Size: %ld\n", file_size);

        char* buffer = (char*)malloc(BLOCKSIZE);
        char* buffer2 = (char*)malloc(BLOCKSIZE);

        // fseek(file,1,SEEK_CUR);

        if (buffer != NULL) {
            while(ftell(file)<file_size){
                if(BLOCKSIZE < 0){
                    printf("Block Size not valid!\n");
                    return -1;
                }
                int bytes_read = fread(buffer, sizeof(char), BLOCKSIZE, file);
                buffer[bytes_read] = '\0';
                fwrite(buffer, sizeof(char), bytes_read, file_out);
                printf("String in the buffer: %s\n", buffer);
            }

            // int bytes_read = fread(buffer, sizeof(char), 100000, file);
            // buffer[bytes_read] = '\0';
            // bytes_read = fread(buffer2, sizeof(char), 100000, file);
            // buffer2[bytes_read] = '\0';

            // memcpy(buffer+100000, buffer2, bytes_read);

            // fwrite(buffer, sizeof(char), 100000 + 100000, file_out);
        } else {
            printf("Memory allocation failed\n");
        }
    }else{
        printf("Cannot open file!\n");
        return -1;
    }
}