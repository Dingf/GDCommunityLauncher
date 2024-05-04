#ifndef INC_GDCL_FILE_BUFFER_H
#define INC_GDCL_FILE_BUFFER_H

class FileBuffer
{
    public:
        virtual ~FileBuffer()
        {
            if (_buffer != nullptr)
            {
                delete[] _buffer;
                _buffer = nullptr;
            }
            _bufferPos = 0;
            _bufferSize = 0;
        }

        int64_t GetPosition() const { return _bufferPos; }
        int64_t GetBufferSize() const { return _bufferSize; }

        void SetPosition(int64_t position) { _bufferPos = position; }

        const uint8_t* GetBuffer() const { return _buffer; }
              uint8_t* GetBuffer()       { return _buffer; }

    protected:
        uint8_t* _buffer;
        int64_t _bufferSize;
        int64_t _bufferPos;
};

#endif//INC_GDCL_FILE_BUFFER_H