#include <vector>
#include <cstdint>
#include <cstddef>
#include <stdexcept>

class APInt {
public:
    // Construct an APInt with a given bit width and initial value.
    explicit APInt(std::size_t bitWidth = 64, std::uint64_t value = 0);

    // Bit width and storage access.
    std::size_t getBitWidth() const { return bitWidth_; }
    std::size_t getNumWords() const { return words_.size(); }

    // Get the value as a zero-extended 64-bit integer.
    std::uint64_t getZExtValue() const;

    // Bit access.
    bool getBit(std::size_t idx) const;
    void setBit(std::size_t idx, bool val);

    // Basic arithmetic operators.
    APInt operator+(const APInt& other) const;
    APInt operator-(const APInt& other) const;
    APInt operator*(const APInt& other) const;

    static constexpr std::size_t bitsPerWord = 64;

private:
    std::size_t bitWidth_;                 // total bits
    std::vector<std::uint64_t> words_;     // little-endian words

    void canonicalize();                      // trim leading zero words, enforce bitWidth_


    std::size_t numWordsForBits(std::size_t bits) const {
        return (bits + bitsPerWord - 1) / bitsPerWord;
    }

    // Helper: compare absolute values (both treated as unsigned).
    int compareAbs(const APInt& other) const;
};
