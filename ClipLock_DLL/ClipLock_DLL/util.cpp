#include "util.h"


// ���̳ʸ� ���Ϸ� �����ϴ� �Լ�
void saveToBinaryFile(const char* filename, const char* data, size_t length) {
    // ���� ���� (���̳ʸ� ���� ����)
    std::ofstream file(filename, std::ios::out | std::ios::binary);
    if (!file) {
        std::cerr << "���� ���� ����: " << filename << std::endl;
        return;
    }

    // ������ ����
    file.write(data, length);

    // ���� �ݱ�
    file.close();

    if (file.fail()) {
        std::cerr << "���� �ݱ� ����: " << filename << std::endl;
    }
    else {
        std::cout << "���� ���� ����: " << filename << std::endl;
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