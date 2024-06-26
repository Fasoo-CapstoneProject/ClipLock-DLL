#include "util.h"


// 바이너리 파일로 저장하는 함수
void saveToBinaryFile(const char* filename, const char* data, size_t length) {
    // 파일 열기 (바이너리 모드로 쓰기)
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file) {
        std::cerr << "파일 열기 오류: " << filename << std::endl;
        return;
    }

    // 데이터 쓰기
    file.write(data, length);

    // 파일 닫기
    file.close();

    if (file.fail()) {
        std::cerr << "파일 닫기 오류: " << filename << std::endl;
    }
    else {
        std::cout << "파일 저장 성공: " << filename << std::endl;
    }
}

void write_vector_to_binary_file(const std::string& filename, const std::vector<unsigned char>& data) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file for writing");
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
    if (!file.good()) {
        throw std::runtime_error("Failed to write data to file");
    }
}

// Function to find two factors of n that are as close to each other as possible
std::pair<int, int> find_factors(int n) {
    return std::make_pair((int)std::sqrt(n) + 1, (int)std::sqrt(n) + 1);
}