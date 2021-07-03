#ifndef INC_GDCL_VALUE_H
#define INC_GDCL_VALUE_H

#include <string>
#include <memory>

enum ValueType
{
    VALUE_TYPE_INT = 0,
    VALUE_TYPE_FLOAT = 1,
    VALUE_TYPE_BOOL = 2,
    VALUE_TYPE_STRING = 3,
};

class Value
{
    public:
        Value(int val)   : _type(VALUE_TYPE_INT),   _i(val) {}
        Value(float val) : _type(VALUE_TYPE_FLOAT), _f(val) {}
        Value(bool val)  : _type(VALUE_TYPE_BOOL),  _b(val) {}
        Value(const char* val) : _type(VALUE_TYPE_STRING), _s(nullptr) { CopyStringData(val); }
        Value(const Value& val);

        ~Value() { DestroyStringData(); }

        static std::unique_ptr<Value> Parse(const std::string& s);

        ValueType GetType() const { return _type; }

        int         ToInt()    const { return _i; }
        float       ToFloat()  const { return _f; }
        bool        ToBool()   const { return _b; }
        const char* ToString() const { return _s; }

    private:
        void DestroyStringData();
        void CopyStringData(const char* val);

        const ValueType _type;

        union
        {
            int   _i;
            float _f;
            bool  _b;
            char* _s;
        };
};

#endif//INC_GDCL_VALUE_H