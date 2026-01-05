#pragma once

#include <print>
#include <vector>
#include <zemita/utils.hpp>

template <typename T>
class RingBuffer
{
   public:
    RingBuffer(int capacity) : capacity_(capacity), rbuffer_(capacity, T{}) {}
    RingBuffer(int capacity, std::initializer_list<T> init) : rbuffer_(init), capacity_(capacity) { aheadEnd_++; }

    std::vector<T>& getBuffer() { return rbuffer_; }

    int getWindowFrontIndex() const { return windowFront_; }

    int getWindowEndIndex() const { return windowEnd_; }

    void setWindowFront(int index)
    {
        index = index % capacity_;
        windowFront_ = index;
    }

    void setWindowEnd(int index)
    {
        index = index % capacity_;
        windowEnd_ = index;
        if (windowSize() > SEARCH_WINDOW_SIZE)  // Is this triggering?
        {
            // Add this
            windowFront_ += windowSize() - SEARCH_WINDOW_SIZE;
            windowFront_ = windowFront_ % capacity_;
        }
    }

    int getAheadFrontIndex() const { return aheadFront_; }

    int getAheadEndIndex() const { return aheadEnd_; }

    const T* getAheadFrontChar() const { return &rbuffer_[aheadFront_]; }

    const T* getAheadEndChar() const { return &rbuffer_[aheadEnd_]; }

    void setAheadFront(int index)
    {
        index = index % capacity_;
        aheadFront_ = index;
    }

    void setAheadEnd(int index)
    {
        index = index % capacity_;
        aheadEnd_ = index;
    }

    auto lookAheadEmpty() const { return aheadFront_ == aheadEnd_; };

    std::vector<T> getAheadRange(std::span<const char>& data, int start, int offset, int length) const
    {
        std::vector<T> result;
        auto lookaheadsize = lookaheadSize();
        auto possible = std::min(lookaheadsize, length);
        result.reserve(possible);
        for (int i = 0; i < possible; i++)
        {
            result.push_back(rbuffer_[(start + i) % rbuffer_.size()]);
        }
        return result;
    }

    std::vector<T> getWindowRange(int start, int length) const
    {
        std::vector<T> result;
        result.reserve(length);
        for (int i = 0; i < length; i++)
        {
            result.push_back(rbuffer_[(start + i) % rbuffer_.size()]);
        }
        return result;
    }

    auto refillLookahead(int& blockDataOffset, std::span<const char>& data, size_t refillSize)
    {
        size_t refiller = std::min(data.size() - blockDataOffset, refillSize);
        for (size_t i{refiller}; i > 0; i--)
        {
            rbuffer_[aheadEnd_] = data[blockDataOffset];
            aheadEnd_ = (aheadEnd_ + 1) % capacity_;
            blockDataOffset++;
        }
        if (windowSize() + refiller > SEARCH_WINDOW_SIZE)
        {
            windowFront_ = aheadEnd_;
        }
    }

    auto windowSize()
    {
        if (windowFront_ > windowEnd_)
        {
            return (capacity_ - windowFront_) + windowEnd_;
        }
        return windowEnd_ - windowFront_;
    }

    auto lookaheadSize() const
    {
        if (aheadFront_ > aheadEnd_)
        {
            return (capacity_ - aheadFront_) + aheadEnd_;
        }
        return aheadEnd_ - aheadFront_;
    }

    bool isInWindow(int index) { return (index - windowFront_ + capacity_) % capacity_ < windowSize(); }
    bool isRangeInWindow(int index, int length)
    {
        // Check if start is in window
        if (!isInWindow(index))
        {
            return false;
        }

        // Check if end is in window
        int endIndex = (index + length - 1) % capacity_;
        if (!isInWindow(endIndex))
        {
            return false;
        }

        // Check length doesn't exceed window size
        return length <= windowSize();
    }
    void pushIndex(T value)
    {
        if (rbuffer_.size() < 8)
        {
            rbuffer_.push_back(value);
            aheadEnd_ = (aheadEnd_ + 1) % capacity_;
        }
        else
        {
            rbuffer_[aheadEnd_] = value;
            aheadEnd_ = (aheadEnd_ + 1) % capacity_;
            // If buffer is full, move front forward (overwrite oldest)
            if (aheadEnd_ == aheadFront_)
            {
                aheadFront_ = (aheadFront_ + 1) % capacity_;
            }
        }
    }

   private:
    std::vector<T> rbuffer_;
    int capacity_;
    int windowFront_ = 0;
    int windowEnd_ = 0;
    int aheadFront_ = 0;
    int aheadEnd_ = 0;
};