#include <iostream>
#include <fstream>
#include <string>
#include <vector>

void splitFile(const std::string & inputFile,
  const std::string & outputPrefix, int chunkSize) {
  // Open the input file in binary mode
  std::ifstream input(inputFile, std::ios::binary);

  if (input.is_open()) {
    // Get the file size
    input.seekg(0, std::ios::end);
    std::streampos fileSize = input.tellg();
    input.seekg(0, std::ios::beg);

    // Calculate the number of chunks
    int numChunks = ((int)fileSize + chunkSize - 1) / chunkSize;

    // Read and write each chunk
    for (int i = 0; i < numChunks; ++i) {
      // Create or overwrite the output file
      std::ofstream output(outputPrefix + std::to_string(i + 1) + ".txt", std::ios::binary);

      if (output.is_open()) {
        std::vector < char > buffer(chunkSize);

        // Read a chunk of data from the input file
        input.read(buffer.data(), chunkSize);

        // Write the chunk to the output file
        output.write(buffer.data(), input.gcount());

        output.close();
      } else {
        std::cout << "Failed to open output file: " << outputPrefix + std::to_string(i + 1) + ".txt" << std::endl;
      }
    }

    input.close();

    std::cout << "File split successfully." << std::endl;
  } else {
    std::cout << "Failed to open the input file." << std::endl;
  }
}

int main() {
  // Input file
  std::string inputFile = "demo.txt";
  // Prefix for output files
  std::string outputPrefix = "part_";
  int chunkSize = 400; // Chunk size in bytes
  splitFile(inputFile, outputPrefix, chunkSize);
  return 0;
}
