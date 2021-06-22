#ifndef INC_GDCL_GAME_OBJECT_H
#define INC_GDCL_GAME_OBJECT_H

#include <stdint.h>

class Object
{
    public:
        Object() : _state(0) {}

        uint32_t GetState() const { return _state; }

        bool IsValid() const { return _state != 0; }

        void SetState(uint32_t state) { _state = state; }

        //TODO: Implement a pure virtual function that will "package" the data into a form that is usable elsewhere
        //      Need to figure out what is the best format/method of doing this... will depend on the API format as well

    protected:
        uint32_t _state;
};

#endif//INC_GDCL_GAME_OBJECT_H