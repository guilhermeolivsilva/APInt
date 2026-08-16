#include <algorithm>
#include <iostream>

#include "APInt.h"


APInt::APInt(std::size_t bitWidth, std::uint64_t value)
    : bitWidth_(bitWidth), words_(numWordsForBits(bitWidth), 0) {
    if (bitWidth == 0) {
        throw std::invalid_argument("bitWidth must be > 0");
    }

    std::size_t maxBits = std::min<std::size_t>(bitWidth_, 64);
    std::uint64_t mask = (maxBits == 64) ? ~std::uint64_t(0)
                                         : (std::uint64_t(1) << maxBits) - 1;
    words_[0] = value & mask;

    canonicalize();
}

void APInt::canonicalize() {
    // Trim leading zero words, but keep at least one word.
    while (words_.size() > 1 && words_.back() == 0) {
        words_.pop_back();
    }

    // Ensure we don't have more words than needed for bitWidth_.
    std::size_t needed = numWordsForBits(bitWidth_);
    if (words_.size() > needed) {
        words_.resize(needed);
    }

    // Mask the top word to respect exact bitWidth_.
    // Use signed arithmetic to detect "negative extraBits".
    long long extraBitsSigned =
        static_cast<long long>(words_.size()) * static_cast<long long>(bitsPerWord)
        - static_cast<long long>(bitWidth_);

    if (extraBitsSigned <= 0) {
        return;
    }

    std::size_t extraBits = static_cast<std::size_t>(extraBitsSigned);
    if (extraBits >= bitsPerWord) {
        words_.clear();
        words_.push_back(0);
        return;
    }

    std::uint64_t mask = (~std::uint64_t(0)) >> extraBits;
    words_.back() &= mask;
}

std::uint64_t APInt::getZExtValue() const {
    if (words_.empty()) return 0;
    std::uint64_t result = words_[0];
    if (bitWidth_ < 64) {
        std::uint64_t mask = (std::uint64_t(1) << bitWidth_) - 1;
        result &= mask;
    }
    return result;
}

// Print from most significant bit (bitWidth_-1) down to 0.
void APInt::printBinary(std::ostream& os) const {
    for (std::size_t i = bitWidth_; i-- > 0; ) {
        os << (getBit(i) ? '1' : '0');
    }
}

bool APInt::getBit(std::size_t idx) const {
    if (idx >= bitWidth_) {
        throw std::out_of_range("bit index out of range");
    }
    std::size_t wordIdx = idx / bitsPerWord;
    std::size_t bitIdx = idx % bitsPerWord;
    return (words_[wordIdx] >> bitIdx) & 1u;
}

void APInt::setBit(std::size_t idx, bool val) {
    if (idx >= bitWidth_) {
        throw std::out_of_range("bit index out of range");
    }
    std::size_t wordIdx = idx / bitsPerWord;
    std::size_t bitIdx = idx % bitsPerWord;
    std::uint64_t mask = std::uint64_t(1) << bitIdx;
    if (val) {
        words_[wordIdx] |= mask;
    } else {
        words_[wordIdx] &= ~mask;
    }
    canonicalize();
}

APInt APInt::operator+(const APInt& other) const {
    if (bitWidth_ != other.getBitWidth()) {
        throw std::invalid_argument("operand bit widths must match for addition");
    }

    std::size_t nWords = numWordsForBits(bitWidth_);
    APInt result(bitWidth_, 0);
    if (result.words_.size() < nWords) {
        result.words_.resize(nWords, 0);
    }

    std::uint64_t carry = 0;
    for (std::size_t i = 0; i < nWords; ++i) {
        std::uint64_t a = (i < words_.size()) ? words_[i] : 0;
        std::uint64_t b = (i < other.words_.size()) ? other.words_[i] : 0;

        unsigned __int128 wide =
            static_cast<unsigned __int128>(a)
          + static_cast<unsigned __int128>(b)
          + static_cast<unsigned __int128>(carry);

        result.words_[i] = static_cast<std::uint64_t>(wide);
        carry = static_cast<std::uint64_t>(wide >> 64);
    }

    result.canonicalize();
    return result;
}

APInt APInt::operator-(const APInt& other) const {
    if (bitWidth_ != other.getBitWidth()) {
        throw std::invalid_argument("operand bit widths must match for subtraction");
    }

    std::size_t nWords = numWordsForBits(bitWidth_);
    APInt result(bitWidth_, 0);
    if (result.words_.size() < nWords) {
        result.words_.resize(nWords, 0);
    }

    std::uint64_t borrow = 0;
    for (std::size_t i = 0; i < nWords; ++i) {
        std::uint64_t a = (i < words_.size()) ? words_[i] : 0;
        std::uint64_t b = (i < other.words_.size()) ? other.words_[i] : 0;

        unsigned __int128 diff =
            static_cast<unsigned __int128>(a)
          - static_cast<unsigned __int128>(b)
          - static_cast<unsigned __int128>(borrow);

        borrow = (diff >> 64) ? 1 : 0;
        result.words_[i] = static_cast<std::uint64_t>(diff);
    }

    result.canonicalize();
    return result;
}

APInt APInt::operator*(const APInt& other) const {
    if (bitWidth_ != other.getBitWidth()) {
        throw std::invalid_argument("operand bit widths must match for multiplication");
    }

    std::size_t nWords = numWordsForBits(bitWidth_);

    // Create result with the correct number of words, bypassing canonicalization for now.
    APInt result(bitWidth_, 0);
    // Ensure result has exactly nWords, even if they are all zero.
    if (result.words_.size() < nWords) {
        result.words_.resize(nWords, 0);
    }

    // Schoolbook multiplication.
    for (std::size_t i = 0; i < words_.size(); ++i) {
        if (words_[i] == 0) continue;
        std::uint64_t carry = 0;
        for (std::size_t j = 0; j < other.words_.size() && (i + j) < nWords; ++j) {
            unsigned __int128 prod =
                static_cast<unsigned __int128>(words_[i])
              * static_cast<unsigned __int128>(other.words_[j])
              + static_cast<unsigned __int128>(result.words_[i + j])
              + static_cast<unsigned __int128>(carry);

            result.words_[i + j] = static_cast<std::uint64_t>(prod);
            carry = static_cast<std::uint64_t>(prod >> 64);
        }

        // Propagate remaining carry.
        std::size_t k = i + other.words_.size();
        while (carry != 0 && k < nWords) {
            unsigned __int128 sum =
                static_cast<unsigned __int128>(result.words_[k])
              + static_cast<unsigned __int128>(carry);

            result.words_[k] = static_cast<std::uint64_t>(sum);
            carry = static_cast<std::uint64_t>(sum >> 64);
            ++k;
        }
    }

    result.canonicalize();
    return result;
}
