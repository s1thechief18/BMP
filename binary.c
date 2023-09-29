#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // The string to write to the binary file
    const char* myString = "This is a sample string.";


    // Open the binary file in write-binary mode
    FILE* file = fopen("file.txt", "w");

    if (file != NULL) {
        // Get the length of the string
        size_t len = strlen(myString);

        // Write the string to the file
        fwrite(myString, sizeof(char), len, file);

        // Close the file
        fclose(file);
        printf("String successfully written to file.bin\n");
    } else {
        printf("Error opening the file\n");
    }

    fclose(file);

    file = fopen("file.txt", "r");

    if (file != NULL) {
        // Seek to the end of the file to get its size
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET); // Rewind the file pointer to the beginning

        // Allocate memory to store the contents of the file
        char* buffer = (char*)malloc(file_size + 1); // Add +1 for null-terminator

        if (buffer != NULL) {
            // Read the entire file into the buffer
            size_t bytes_read = fread(buffer, 1, file_size, file);
            
            // Null-terminate the buffer to treat it as a C string
            buffer[bytes_read] = '\0';

            // Close the file
            fclose(file);

            // Print the contents of the file (in binary sequence)
            printf("Binary sequence in the file:\n");
            for (size_t i = 0; i < bytes_read; i++) {
                printf("%02x ", (unsigned char)buffer[i]);
            }
            printf("\n");

            // You can also print it as a regular string if needed
            printf("String in the file: %s\n", buffer);

            // Free the allocated memory
            free(buffer);
        } else {
            printf("Memory allocation failed\n");
        }
    } else {
        printf("Error opening the file\n");
    }

    return 0;
}
