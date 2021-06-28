#ifndef INC_GDCL_VALUE_H
#define INC_GDCL_VALUE_H

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
        Value(const char* val) : _type(VALUE_TYPE_STRING)
        {
            _s = nullptr;
            CopyStringData(val);
        }
        Value(const Value& val) : _type(val._type)
        {
            switch (val._type)
            {
            case VALUE_TYPE_INT:
                _i = val._i;
                break;
            case VALUE_TYPE_FLOAT:
                _f = val._f;
                break;
            case VALUE_TYPE_BOOL:
                _b = val._b;
                break;
            case VALUE_TYPE_STRING:
                CopyStringData(val._s);
                break;
            }
        }
        ~Value()
        {
            DestroyStringData();
        }

        ValueType GetType() const { return _type; }

        int         ToInt()    const { return _i; }
        float       ToFloat()  const { return _f; }
        bool        ToBool()   const { return _b; }
        const char* ToString() const { return _s; }

    private:
        void DestroyStringData()
        {
            if ((_type == VALUE_TYPE_STRING) && (_s != nullptr))
            {
                delete[] _s;
                _s = nullptr;
            }
        }

        void CopyStringData(const char* val)
        {
            if (_type == VALUE_TYPE_STRING)
            {
                DestroyStringData();
                if (val != nullptr)
                {
                    _s = new char[256]();
                    for (int i = 0; i < 255; ++i)
                    {
                        _s[i] = val[i];
                        if (val[i] == '\0')
                            break;
                    }
                }
            }
        }

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