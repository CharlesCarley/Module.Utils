/*
-------------------------------------------------------------------------------
    Copyright (c) Charles Carley.

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------
*/
// TODO:  All this should be extracted into a header only sub-directory
#pragma once
#include <bitset>
#include <cstdint>
#include <iomanip>
#include "Utils/Array.h"
#include "Utils/String.h"
#undef min
#undef max

namespace Rt2
{
    template <typename T>
    struct CallableStream
    {
        friend OStream& operator<<(
            OStream& out,
            const T& set)
        {
            return set(out);
        }
    };

    struct Nl : CallableStream<Nl>
    {
        OStream& operator()(OStream& out) const
        {
            return out << std::endl;
        }
    };

    template <typename T>
    class Hex : CallableStream<Hex<T>>
    {
    private:
        size_t _value;

    public:
        explicit Hex(T v) :
            _value{(size_t)v} {}
        explicit Hex(T* v) :
            _value{(size_t)v} {}

        OStream& operator()(OStream& out) const
        {
            return out
                   << std::setfill('0')
                   << std::setw(sizeof(T) << 1)
                   << std::uppercase
                   << std::hex
                   << _value;
        }
    };

    class HexId : CallableStream<HexId>
    {
    public:
        OStream& operator()(OStream& out) const
        {
            static int nr = 0;

            return out << std::setfill('0')
                       << std::setw(8)
                       << std::uppercase
                       << std::hex
                       << (++nr);
        }
    };

    template <typename T,
              uint8_t W = (sizeof(T) << 1) + 2,
              uint8_t P = sizeof(T) << 1>
    class FPrintT : CallableStream<FPrintT<T, W, P>>
    {
    private:
        T       _value;
        uint8_t _w;
        uint8_t _p;

    public:
        explicit FPrintT(const T&      v,
                         const uint8_t w = W,
                         const uint8_t p = P) :
            _value{v}, _w(w), _p(p) {}

        OStream& operator()(OStream& out) const
        {
            if (_p > 0)
                out << std::setprecision(_p) << std::fixed;
            else
                out << std::defaultfloat;
            return out << _value;
        }

        void from(IStream& in)
        {
            in >>
                std::setfill(' ') >>
                std::setw(_w) >>
                std::setprecision(_p) >>
                _value;
        }

        explicit operator T()
        {
            return _value;
        }
    };

    template <typename T,
              uint8_t Len>
    class Bin : CallableStream<Bin<T, Len>>
    {
    private:
        std::bitset<Len> _value;

    public:
        explicit Bin(const T& v) :
            _value(v) {}

        OStream& operator()(OStream& out) const
        {
            size_t        i   = 0;
            const String& str = _value.to_string();
            while (i < str.size())
            {
                out << str[i];
                if (i + 1 < str.size() && i % 4 == 3)
                    out << "`";
                ++i;
            }
            return out;
        }
    };

    template <uint8_t PadIn  = 1,
              uint8_t PadOut = 1>
    class Equ : CallableStream<Equ<PadIn, PadOut>>
    {
    private:
        uint8_t _pi;
        uint8_t _po;

    public:
        explicit Equ(const uint8_t pi = PadIn,
                     const uint8_t po = PadOut) :
            _pi(pi),
            _po(po)
        {
        }

        OStream& operator()(OStream& out) const
        {
            out << std::setw(_pi) << ' ';
            out << ":=";
            return out << std::setw(_po) << ' ';
        }
    };

    template <typename T>
    bool isEmpty(const T& v)
    {
        return v.empty();
    }

    template <typename T>
    class Attribute : CallableStream<Attribute<T>>
    {
    private:
        using CheckFn = std::function<bool(const T&)>;

        String        _key;
        T             _val;
        const CheckFn _fn;

        bool empty() const
        {
            // conditionally write to
            // the stream if it's not empty
            return _fn ? _fn(_val) : true;
        }

    public:
        explicit Attribute(String key, T val, CheckFn cb = isEmpty<T>) :
            _key(std::move(key)),
            _val(std::move(val)),
            _fn(cb)
        {
        }

        OStream& operator()(OStream& out) const
        {
            if (empty())
                return out;
            return out << _key << '=' << '"' << _val << '"';
        }
    };

    class AttributeMap : CallableStream<AttributeMap>
    {
    private:
        StringMap _values;

    public:
        explicit AttributeMap(StringMap& values) :
            _values(std::move(values))
        {
        }

        OStream& operator()(OStream& out) const
        {
            if (!_values.empty())
            {
                out << "style=" << '"';
                for (const auto& [k, v] : _values)
                    out <<  k << ':' << v << ';';
                out << '"';
            }
            return out;
        }
    };

    struct Tab : CallableStream<Tab>
    {
        mutable uint8_t w{4};

        Tab() = default;

        explicit Tab(const uint8_t nw) :
            w(nw) {}

        OStream& operator()(OStream& out) const
        {
            w = std::max<uint8_t>(w, 1);
            return out
                   << std::setfill(' ')
                   << std::setw(std::streamsize(w) - 1)
                   << ' ';
        }
    };

    template <uint8_t W = 80, int8_t C = '-', int8_t R = ' '>
    class FillT : CallableStream<FillT<W, C, R>>
    {
    private:
        uint8_t _w;

    public:
        explicit FillT(const uint8_t w = W) :
            _w(w) {}

        OStream& operator()(OStream& out) const
        {
            out << std::setw(size_t(std::max((int)_w, 1) - 1));
            out << std::setfill((char)C) << (char)C;
            out << std::setfill((char)R);
            return out;
        }
    };

    class NlTab : CallableStream<NlTab>
    {
    public:
        OStream& operator()(OStream& out) const
        {
            out << std::endl;
            return out << "    ";
        }
    };

    template <typename T,
              uint8_t O,
              uint8_t S,
              uint8_t C,
              uint8_t W,
              uint8_t P>
    class TPrintSet : CallableStream<TPrintSet<T, O, S, C, W, P>>
    {
    public:
        using Set = Array<T>;

    private:
        Set     _data;
        uint8_t _w;
        uint8_t _p;
        bool    _fix{false};

    public:
        explicit TPrintSet(const Set&    data,
                           const uint8_t w   = W,
                           const uint8_t p   = P,
                           const bool    fix = false) :
            _data(data), _w(w), _p(p), _fix(fix)
        {
        }

        OStream& operator()(OStream& out) const
        {
            if (O != '\0')
                out << (int8_t)O;

            out << std::setfill(' ');
            bool f = true;
            for (auto& data : _data)
            {
                if (!f) out << (int8_t)S << ' ';
                out << std::setw(_w);
                out << std::setprecision(_p);
                if (_fix)
                    out << std::fixed;
                out << data;
                f = false;
            }
            if (C != '\0')
                out << (int8_t)C;
            return out;
        }
    };

    using AttrI32    = Attribute<I32>;
    using AttrI16    = Attribute<I16>;
    using AttrI8     = Attribute<I8>;
    using AttrString = Attribute<String>;

    using SetD      = TPrintSet<double, '[', ',', ']', sizeof(double) << 1, 12>;
    using SetF      = TPrintSet<float, '[', ',', ']', 6, 3>;
    using SetI      = TPrintSet<int32_t, '[', ',', ']', 0, 8>;
    using SetL      = TPrintSet<int64_t, '[', ',', ']', 0, 8>;
    using SetU      = TPrintSet<uint32_t, '[', ',', ']', 0, 8>;
    using SetUl     = TPrintSet<uint64_t, '[', ',', ']', 0, 8>;
    using SetS      = TPrintSet<String, '(', ',', ')', 0, 0>;
    using ValueSetF = TPrintSet<float, '\0', ',', '\0', 6, 3>;
    using ValueSetI = TPrintSet<int, '\0', ',', '\0', 0, 8>;
    using ValueSetU = TPrintSet<uint32_t, '\0', ',', '\0', 0, 8>;
    using ValueSetS = TPrintSet<String, '\0', ',', '\0', 0, 0>;
    using Line      = FillT<32, '-', ' '>;
    using Bin8      = Bin<uint8_t, 8>;
    using Bin16     = Bin<uint16_t, 16>;
    using Bin32     = Bin<uint32_t, 32>;
    using Bin64     = Bin<uint64_t, 64>;
    using PrintR32  = FPrintT<float, 5, 3>;
    using PrintR64  = FPrintT<double>;

}  // namespace Rt2
