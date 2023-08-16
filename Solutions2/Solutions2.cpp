#include <iostream>
#include <functional>
#include <string>
#include <algorithm>
#include <type_traits>
#include <array>
#include <cmath>
#include <locale>
#include <limits>
#include <codecvt>
#include <io.h>
#include <fcntl.h>
using namespace std;

template<typename Test, template<typename...> class Ref>
struct is_specialization : std::false_type {};

template<template<typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};



struct solution {
    typedef void (*exec_func)();
    wstring_view name;
    wstring_view description;
    wstring_view input_data;
    wstring_view output_data;
    exec_func executor;
    exec_func start_tests;
    wstring_view acmp_id;
    wstring_view acmp_hard;
    bool virtual_sol;
};
class Error {
public:
    enum err_type {
        VALIDATE,
        UNREACHABLE,
    };
    enum validation_error {
        NONE,
        // ints errors
        NOT_NATURAL,
        OVERFLOW_READ,
        NOT_IN_RANGE,
        NOT_QUAL,

        // strs errors
        MORE_MAX_SIZE,
        INVALID_CHAR,
    };
private:
    err_type et;
    wstring_view why;
    wstring_view where;
    validation_error ve;
public:
    wstring_view get_why() {
        return why;
    }
    void set_where(wstring_view where) {
        if(this->where.empty())
            this->where = where;
    }
    wstring_view get_where() {
        return where;
    }
    err_type get_error_type() {
        return et;
    }
    err_type get_validation_error() {
        return et;
    }

    Error(err_type et, wstring_view where, wstring_view why=L"", validation_error ve=NONE) {
        this->et = et;
        this->why = why;
        this->where = where;
        this->ve = ve;
    }
    Error(validation_error ve, wstring_view why) {
        this->et = VALIDATE;
        this->why = why;
        this->where = L"";
        this->ve = ve;
    }
};
namespace Validators
{
    // TODO: REFACTOR 
    namespace Ints {
        template<typename T> constexpr void check_natural(T i) {
            if (!(i > 0))
                throw Error(Error::NOT_NATURAL, L"Число не натуральное!");
        }
        template<typename T> constexpr void check_not_overflow(T i) {
            if(i == std::numeric_limits<T>::max() || i == std::numeric_limits<T>::min())
                throw Error(Error::OVERFLOW_READ, L"Переполнение");
        }
        template<typename T, int more> constexpr void check_not_more_than(T i) {
            if (i >= more)
                throw Error(Error::NOT_IN_RANGE, L"Выход за пределы допустимого значения!!!");
        }
        template<typename T, int more> constexpr void check_not_more_than_abs(T i) {
            check_not_more_than<T, more>(abs(i));
        }
        template<typename T, int min> constexpr void check_not_min_than(T i) {
            if (i <= min)
                throw Error(Error::NOT_IN_RANGE, L"Выход за пределы допустимого значения!!!");
        }
        template<typename T, int min, int max> constexpr void check_in_range_int(T i) {
            check_not_min_than<T, min>(i);
            check_not_more_than_abs<T, max>(i);
        }
        template<typename T> constexpr void check_for_qual(T i) {
            if constexpr (std::is_same<T, int>::value || std::is_same<T, short>::value) // int не могут не быть целыми
                return;
            if (((int)i) != i)
                throw Error(Error::NOT_QUAL, L"Число не целое!!!");
        }
    }

    namespace Chars {
        template<typename T, int count, const array<T, count> allowed_chars> constexpr void check_char(T c) {
            for (const auto& i : allowed_chars)
                if (i == c)
                    return;
            throw Error(Error::INVALID_CHAR, L"Символ не является разрешённым!");
        }
        template<typename T, int min, int max> constexpr void check_in_range(T i) {
            if (!(min < i < max))
                throw Error(Error::INVALID_CHAR, L"Символ вне диапазона!");
        }
    }

    namespace Strs {
        template<typename T, int max_size> constexpr void check_for_size(const T& str) {
            if (str.size() > max_size)
                throw Error(Error::MORE_MAX_SIZE, L"Строка слишком большая");
        }
        template<typename T, int size, array<typename T::value_type, size> chars> constexpr void check_for_only_chars(const T& str) {
            for (const auto& i : str) {
                bool find = false;
                for (const auto& c : chars) {
                    if (c == i) {
                        find = true;
                        break;
                    }
                }
                if(!find)
                    throw Error(Error::INVALID_CHAR, L"Символ не является разрешённым!");
            }
        }
        template<typename T, T::value_type min, T::value_type max> constexpr void check_for_in_range_chars(const T& str) {
            for (const auto& i : str) {
                Chars::check_in_range<T::value_type, min, max>(i);
            }
        }
        template<typename T> constexpr void check_for_capital_chars(const T& str) {
            for (const auto& i : str) {
                if (!isupper(i))
                    throw Error(Error::INVALID_CHAR, L"Символ в строке в нижнем регистре!");
            }
        }
        template<typename T> constexpr void check_for_noncapital_chars(const T& str) {
            for (const auto& i : str) {
                if (!islower(i))
                    throw Error(Error::INVALID_CHAR, L"Символ в строке в высшем регистре!");
            }
        }
    }
}
using namespace Validators::Ints;
using namespace Validators::Strs;
using namespace Validators::Chars;

namespace IO {
    // read section

    template<typename T> T constexpr read_smth(wstring_view why) {
        if(!why.empty())
            wcout << why << endl;
        T val;
        wcin >> val;
        return val;
    }
    template<int size = 0> wstring constexpr read_str(wstring_view why, array<void(*)(const wstring&), size> validators = {}) {
        wstring res = read_smth<wstring>(why);
        try {
            std::for_each(validators.begin(), validators.end(), [&res](auto a) {a(res); });
        }
        catch (Error& e) {
            if (e.get_error_type() == Error::VALIDATE)
                e.set_where(why);
            throw;
        }
        return res;
    }
    template<int T = 0> int constexpr read_int(wstring_view why, array<void(*)(const int&), T> validators = {}) {
        int val = read_smth<int>(why);
        try {
            std::for_each(validators.begin(), validators.end(), [&cval = as_const(val)](const auto& a) {a(cval); });
        }
        catch (Error& e) {
            if (e.get_error_type() == Error::VALIDATE)
                e.set_where(why);
            throw;
        }
        return val;
    }
    double constexpr read_double(wstring_view why) {
        return read_smth<double>(why);
    }
    template<int count = 0> wchar_t constexpr read_wchart(wstring_view why, array<void (*)(wchar_t), count> validators = {}) {
        wchar_t val = read_smth<wchar_t>(why);
        try {
            std::for_each(validators.begin(), validators.end(), [&val](auto a) {a(val); });
        }
        catch (Error& e) {
            if (e.get_error_type() == Error::VALIDATE)
                e.set_where(why);
            throw;
        }
        return val;
    }
    template<typename T> vector<T> constexpr read_vector(wstring_view why, int count = 0) {
        vector<T> v;
        if (!why.empty())
            wcout << why << endl;
        if (count == 0) {
            wcin >> count;
            check_not_overflow(count);
        }
        v.reserve(count);
        for (int i = 0;i < count;i++) {
            T val = 0;
            wcin >> val;
            check_not_overflow<T>(val);
            v.push_back(val);
        }
        return v;
    }
    template<typename T, int count_v = 0, int count_vc = 0, int count_va = 0> void constexpr read_vector_referred(wstring_view why, vector<T>& v, int count = 0,
        std::array<void(*)(T), count_v> validators_elements = {}, 
        std::array<void(*)(int), count_vc> validators_count = {},
        std::array<void(*)(const vector<T>&), count_va> validators_vector = {}) {
        if (!why.empty())
            wcout << why << endl;
        if (count == 0) {
            wcin >> count;
            check_not_overflow<int>(count);
        }
        try {
            std::for_each(validators_count.begin(), validators_count.end(), [&count](auto a) {a(count); });
        }
        catch (Error& e) {
            if (e.get_error_type() == Error::VALIDATE)
                e.set_where(why);
            throw;
        }
        v.reserve(count);
        for (int i = 0; i < count; i++) {
            T val = 0;
            wcin >> val;
            check_not_overflow<T>(val);
            try {
                std::for_each(validators_elements.begin(), validators_elements.end(), [&val](auto a) {a(val); });
            }
            catch (Error& e) {
                if (e.get_error_type() == Error::VALIDATE)
                    e.set_where(why);
                throw;
            }
            v.push_back(val);
        }
        for (int i = 0; i < count_va; i++) {
            validators_vector[i](v);
        }
    }
    template<typename T> vector<T> constexpr read_vector_custom(wstring_view why, function<T()> getter, int count = 0) {
        vector<T> v;
        if (!why.empty())
            wcout << why << endl;
        if (count == 0) {
            wcin >> count;
            check_not_overflow(count);
        }
        
        v.reserve(count);
        for (int i = 0; i < count; i++) {
            T val = getter();
            check_not_overflow(val);
            v.push_back(val);
        }
        return v;
    }
    // for ACMP.RU 37
    template<typename T, typename T2, int count_vc = 0, int count_vac = 0, int count_ve = 0> void constexpr read_vector_custom_and_read_some_after_count_referred(wstring_view why, vector<T>& v, T2& val, T(*getter)(void), int count = 0,
        array<void(*)(int&), count_vc> validator_count = {},
        array<void(*)(T2&), count_vac> validator_after_count = {},
        array<void(*)(T&), count_ve> validator_elem = {}
        ) {
        if (!why.empty())
            wcout << why << endl;
        if (count == 0) {
            wcin >> count;
            check_not_overflow(count);
            try {
                std::for_each(validator_count.begin(), validator_count.end(), [&count](auto a) {a(count); });
            }
            catch (Error& e) {
                if (e.get_error_type() == Error::VALIDATE)
                    e.set_where(why);
                throw;
            }
            val = read_smth<T2>(L"");
            check_not_overflow(val);
            try {
                std::for_each(validator_after_count.begin(), validator_after_count.end(), [&val](auto a) {a(val); });
            }
            catch (Error& e) {
                if (e.get_error_type() == Error::VALIDATE)
                    e.set_where(why);
                throw;
            }
        }
        v.reserve(count);
        for (int i = 0; i < count; i++) {
            T val = getter();
            check_not_overflow(val);
            try {
                std::for_each(validator_elem.begin(), validator_elem.end(), [&val](auto a) {a(val); });
            }
            catch (Error& e) {
                if (e.get_error_type() == Error::VALIDATE)
                    e.set_where(why);
                throw;
            }
            v.push_back(val);
        }
    }
    template<typename T, typename T2, int T3 = 0, int T4 = 0> tuple<T, T2> constexpr read_two_smth(tuple<wstring_view, wstring_view> why, 
        tuple<array<void(*)(const T&), T3>, array<void(*)(const T2&), T4>>  validators = {}) {
        T val = read_smth<T>(get<0>(why));
        check_not_overflow<T>(val);
        try {
            std::for_each(get<0>(validators).begin(), get<0>(validators).end(), [&cval = as_const(val)](auto a) {a(cval); });
        }
        catch (Error& e) {
            if (e.get_error_type() == Error::VALIDATE)
                e.set_where(get<0>(why));
            throw;
        }
        T2 val2 = read_smth<T2>(get<1>(why));
        check_not_overflow<T2>(val2);
        try {
            std::for_each(get<1>(validators).begin(), get<1>(validators).end(), [&val2 = as_const(val)](auto a) {a(val2); });
        }
        catch (Error& e) {
            if (e.get_error_type() == Error::VALIDATE)
                e.set_where(get<1>(why));
            throw;
        }
        return make_tuple(val, val2);
    }
    template<typename T, int T2 = 0> tuple<T, T> constexpr read_two_smth(wstring_view why, array<void(*)(const T&), T2> validators = {}) {
        if (!why.empty())
            wcout << why << endl;
        return read_two_smth<T, T, T2, T2>(make_tuple(L"", L""), make_tuple(validators, validators));
    }

    // print section

    template<typename T> void constexpr print_array(T val, const bool no_begin = true) {
        if (!no_begin)
            wcout << L"Результат: " << endl;
        for (const auto& i: val) {
            wcout << i << L" ";
        }
        wcout << endl << endl;
    }
    // optimizated version)
    template<typename T> void constexpr print_array_referred(T& val, const bool no_begin = true) {
        if (!no_begin)
            wcout << L"Результат: " << endl;
        for (const auto& i : val) {
            wcout << i << L" ";
        }
        wcout << endl << endl;
    }
    template<typename T, typename T2> void print_pair(pair<T, T2> p, bool no_begin = true) {
        if (!no_begin)
            wcout << L"Результат: " << endl;
        wcout << p.first << L" " << p.second << endl;

    }
    template<typename T> void constexpr print_smth(T val, const bool no_begin = true) {
        if constexpr (!std::is_fundamental<T>::value && !std::is_same<T, wstring_view>::value)
            if constexpr (std::is_reference<T>::value)
                print_array_referred(val, no_begin);
            else
                print_array(val, no_begin);
        else{
            if (!no_begin)
                wcout << L"Результат: " << endl;
            wcout << val << endl << endl;
        }
    }
    template<typename T> void constexpr print_result(T val) {
        if constexpr (std::is_reference<T>::value)
            print_array_referred(val, false);
        else
            print_smth(val, false);
    }
}
using namespace IO;

namespace Progs {
    template<typename T> void constexpr test_smth(wstring_view input, wstring_view output, pair<T, T> in_out) {
        wcout << L"Тестируем выражение: " << endl;
        wcout << L"Входные данные: " << endl;
        print_smth(input);
        wcout << L"Выходные данные: " << endl;
        print_smth(output);
        if (in_out.first != in_out.second) {
            wcout << L"Тест не пройден!" << endl;
            wcout << L"Выходные данные по мнению программы: " << endl;
            print_smth(in_out.second);
        }
        else
            wcout << L"Тест пройден!" << endl;
    }
    class Add {
    public:
        static int constexpr program(int a, int b) {
            return a + b;
        }
        static void constexpr executor() {
            auto two = read_two_smth<int, 1>(
                L"Первое Натуральное число и второе Натуральное число",
                array<void(*)(const int&), 1>{
                check_natural<const int&>,
            });

            print_result(program(get<0>(two), get<1>(two)));
        }
        static void constexpr test() {
            test_smth(
                L"2 3",
                L"5",
                make_pair(program(2,3), 5)
            );
        }
    };

    class Summ {
    public:
        static int constexpr program(int N) {
            int counter = 0;
            if (N > 1) {
                for (int i = 0; i < (N + 1); i++) {
                    counter += i;
                }
            }
            else {
                for (int i = N; i <= 1; i++) {
                    counter += i;
                }
            }
            return counter;
        }
        static void constexpr executor() {
            print_result(program(read_int<2>(L"единственное целое число N, не превышающее по абсолютной величине 10^4",
                array<void(*)(const int&), 2>{
                check_for_qual<const int&>,
                check_not_more_than<const int&, 10000>,
            })));
        }
    };

    class SortPodstetom {
    public:
        static void constexpr program(vector<short>& arr) {
            std::stable_sort(arr.begin(), arr.end());
            
        }
        static void constexpr executor() {
            vector<short> arr;
            read_vector_referred<short, 2, 1>(L"Количество и целые значения температур этих участков, не превосходящие 100 по абсолютной величине.", arr,
                0, array<void(*)(short), 2>{
                    check_for_qual<short>,
                    check_not_more_than_abs<short, 100>,
            },
                array<void(*)(int), 1>{
                check_not_more_than<int, 1000000>,
            });
            program(arr);
            print_result<const vector<short>&>(arr);
        }
        static void constexpr test() {
            vector<short> arr = { 9, -20, 14 };
            program(arr);
            test_smth(
                L"\n3\n9 -20 14",
                L"-20 9 14",
                make_pair(arr, vector<short> {-20, 9, 14})
            );
        }
    };

    class Dragons {
    public:
        static int constexpr program(int n) {
            int m = pow(3, (n / 3));
            if(n % 3 == 2){
                m *= 2;
            }
            else if (n % 3 == 1) {
                m = m * 4 / 3;
            }
            return m;
        }
        static void constexpr executor() {
            print_result(program(read_int<2>(L"натуральное число N (0 < N < 100) – количество голов драконьей стаи.",
                array<void(*)(const int&), 2>{
                    check_natural<const int&>,
                    check_not_more_than<const int&, 100>
            }
            )));
        }
    };

    class Strelki {
    public:
        static int constexpr program(wstring_view str) {
            int k = 0;
            int pos = str.find(L">>-->");
            while (pos + 1) {
                k++;
                pos = str.find(L">>-->", pos + 1);
            }
            int pos1 = str.find(L"<--<<");
            while (pos1 + 1) {
                k++;
                pos1 = str.find(L"<--<<", pos1 + 1);
            }
            return k;
        }
        static void constexpr executor() {
            print_result(program(
                read_str<2>(L"строка, состоящая из символов ‘>’, ‘<’ и ‘-‘ (без пробелов). Строка состоит не более, чем из 250 символов.",
                    array<void(*)(const wstring&), 2>{
                        check_for_size<wstring, 250>,
                        check_for_only_chars < wstring, 3, array<wchar_t, 3>{L'>',L'<',L'-'}>,
                    }
                    )));
        }
    };

    class ShimaushiOperator {
    public:
        static void custom_validator_elem(pair<pair<double, double>, pair<double, double>>& p) {
            check_not_more_than_abs<double, 1000>(p.first.first);
            check_not_more_than_abs<double, 1000>(p.first.second);
            check_not_more_than_abs<double, 1000>(p.second.first);
            check_not_more_than_abs<double, 1000>(p.second.second);
        }
        static pair<pair<double, double>, pair<double, double>> custom_reader() {
            pair<pair<double, double>, pair<double, double>> p;
            wcin >> p.first.first >> p.first.second >> p.second.first >> p.second.second;
            return p;
        }
        static double distance(const pair<double, double>& a, const pair<double, double>& b) {
            double dx = a.first - b.first, dy = a.second - b.second;
            return sqrt(dx * dx + dy * dy);
        }

        static std::wstring_view shimaushi_operator(double q, const vector<pair<pair<double, double>, pair<double, double>>>& arr) {
            const pair<double,double> center(0, 0);
            bool isCompressed = true;
            for (const auto& i : arr) {
                auto d_a = distance(i.first, center) * 1000;
                auto d_b = distance(i.second, center) * 1000;

                if (d_a * q < d_b) {
                    isCompressed = false;
                    break;
                }
            }
            return isCompressed ? L"Yes" : L"No";
        }
        static wstring_view constexpr program(double q, const vector<pair<pair<double, double>, pair<double, double>>>& v) {
            return shimaushi_operator(q, v);
        }
        static void constexpr executor() {
            double q = 0;
            std::vector<pair<pair<double, double>, pair<double, double>>> arr_q;
            read_vector_custom_and_read_some_after_count_referred<pair<pair<double, double>, pair<double, double>>, double, 2, 2, 1>(
                L"количество точек n (1 ≤ n ≤ 100) и число q Следующие n строк содержат по 4 целых числа, по модулю не превосходящих 1000, разделенные пробелами – координаты точки множества X и сопоставленной ей точки из множества Y.",
                arr_q, q,
                custom_reader,
                0,
                array<void(*)(int&), 2>{
                    check_natural,
                    check_not_more_than<int&, 100>,
                },
                array<void(*)(double&), 2>{
                check_natural,
                check_not_more_than<double&, 1>,
                },
                array<void(*)(pair<pair<double, double>, pair<double, double>>&), 1>{
                    custom_validator_elem,
                }
                );
            print_result(program(q, arr_q));
        }
    };

    class NotHardEval {
    public:
        static int constexpr program(int a, int k) {
            char buf[128];
            _itoa_s(a, buf, 128, k);
            int res = 1;
            int pluses = 0;
            for (char* s = buf; *s != '\0'; s++) {
                res *= (*s - '0');
            }
            for (char* s = buf; *s != '\0'; s++) {
                pluses += (*s - '0');
            }
            res -= pluses;
            return res;
        }
        static void constexpr executor() {
            print_smth<wstring_view>(L" n и k (1 ≤ n ≤ 109, 2 ≤ k ≤ 10). Оба этих числа заданы в десятичной системе счисления.");
            auto nk = read_two_smth<int, int, 2, 2>(make_tuple(L"", L""),
                make_tuple(array<void (*)(const int&), 2>{
                    check_not_min_than<const int&, 1>,
                    check_not_more_than<const int&, 1000000000>
                },
                array<void (*)(const int&), 2>{
                    check_not_min_than<const int&, 2>,
                    check_not_more_than<const int&, 10>
                })
                );
            print_result(program(get<0>(nk),get<1>(nk)));
        }
    };

    class Keyboard {
    public:
        static wchar_t constexpr program(wchar_t a) {
            constexpr const wstring_view s = L"qwertyuiopasdfghjklzxcvbnm";
            return s[(s.find(a) + 1) % 26];
        }
        static void constexpr executor() {
            print_result(program(read_wchart<1>(L"один символ — маленькая буква английского алфавита.",
                array<void (*)(wchar_t), 1>{
                check_in_range<wchar_t, L'a', L'm'>
                }
                )));
        }
    };

    class Numberman {
    public:
        static void constexpr program(wstring& a, int& i) {
            int m = 0;
            while (a.size() > 1) {
                for (auto i : a) {
                    m += (i - '0');
                }
                a = to_wstring(m);
                m = 0;
                i++;
            }
        }
        static void constexpr executor() {
            // TODO: convert
            wstring a = read_str<1>(L"число N – время жизни человека в секундах (1 ≤ N ≤ 10^1000).",
                array<void (*)(const wstring&), 1>{
                    // строка может быть бесконечно большой
                    // check_for_size<wstring, 10^1000> 
                    [](const wstring&) {} // placeholder
                }
            );
            int i = 0;
            program(a, i);
            print_pair(make_pair((wstring_view)a, i), false);
        }
    };

    class Arbus {
    public:
        static pair<int, int> constexpr program(const vector<int>& a) {
            return make_pair(*std::min_element(a.begin(), a.end()), *std::max_element(a.begin(), a.end()));
        }
        static void constexpr executor() {
            std::vector<int> v;
            read_vector_referred<int, 2, 2>(L"число N – количество арбузов. Вторая строка содержит N чисел, записанных через пробел. Здесь каждое число – это масса соответствующего арбуза. Все числа натуральные и не превышают 30000.", v,
                0,
                array<void (*)(int), 2>{
                    check_natural<int>,
                    check_not_more_than<int, 30000>,
                },
                array<void (*)(int), 2>{
                check_natural<int>,
                check_not_more_than<int, 30000>,
                }
                );
            print_pair(program(v), false);
        }
    };

    class Anagrammes {
    public:
        static wstring_view constexpr program(wstring_view s, wstring_view s1) {
            if (s.size() != s1.size()) {
                return L"NO";
            }
            std::vector<wchar_t> a = std::vector<wchar_t>(s.size());
            std::vector<wchar_t> b = std::vector<wchar_t>(s1.size());

            std::copy(s.begin(), s.end(), a.begin());
            std::copy(s1.begin(), s1.end(), b.begin());


            std::sort(a.begin(), a.end());
            std::sort(b.begin(), b.end());

            return ((a == b) ? L"YES" : L"NO");
        }
        static void constexpr executor() {
            tuple<wstring, wstring> a = read_two_smth<wstring, 2>(L"строку S1, вторая - S2. Обе строки состоят только из прописных букв английского алфавита. Строки не пусты и имеют длину не больше 100000 символов.",
                array<void (*)(const wstring&), 2>{
                check_for_capital_chars<wstring>,
                check_for_size<wstring, 100000>,
            }
            );
            print_result(program(get<0>(a), get<1>(a)));
        }
        static void constexpr test() {
            test_smth(
                L"ABAA\nABBA",
                L"NO",
                make_pair(program(L"ABAA", L"ABBA"), (wstring_view)L"NO")
            );
        }
    };

    class NumberE {
    public:
        static wstring_view constexpr program(int n) {
            switch (n) {
            case 0:  return L"3"; break;
            case 1:  return L"2.7"; break;
            case 2:  return L"2.72"; break;
            case 3:  return L"2.718"; break;
            case 4:  return L"2.7183"; break;
            case 5:  return L"2.71828"; break;
            case 6:  return L"2.718282"; break;
            case 7:  return L"2.7182818"; break;
            case 8:  return L"2.71828183"; break;
            case 9:  return L"2.718281828"; break;
            case 10: return L"2.7182818285"; break;
            case 11: return L"2.71828182846"; break;
            case 12: return L"2.718281828459"; break;
            case 13: return L"2.7182818284590"; break;
            case 14: return L"2.71828182845905"; break;
            case 15: return L"2.718281828459045"; break;
            case 16: return L"2.7182818284590452"; break;
            case 17: return L"2.71828182845904524"; break;
            case 18: return L"2.718281828459045235"; break;
            case 19: return L"2.7182818284590452354"; break;
            case 20: return L"2.71828182845904523536"; break;
            case 21: return L"2.718281828459045235360"; break;
            case 22: return L"2.7182818284590452353603"; break;
            case 23: return L"2.71828182845904523536029"; break;
            case 24: return L"2.718281828459045235360288"; break;
            case 25: return L"2.7182818284590452353602875"; break;
            default: throw Error(Error::err_type::UNREACHABLE, L"Аргумент n > 25", L"NumberE");
            }
        }
        static void constexpr executor() {
            print_result(program(read_int<2>(L"целое число n (0 ≤ n ≤ 25)",
                array<void(*)(const int&), 2>{
                    check_for_qual<const int&>,
                    check_in_range_int<const int&, -1, 27>,
                }
                )));
        }
    };
}

using namespace Progs;

const constexpr solution sols[] = {
	{L"A+B",
    L"Требуется сложить два целых числа А и В.", 
    L"В единственной строке входного файла INPUT.TXT записаны два натуральных числа через пробел. Значения чисел не превышают 109.",
    L"В единственную строку выходного файла OUTPUT.TXT нужно вывести одно целое число — сумму чисел А и В.",
    Add::executor,
    Add::test,
    L"0001",
    L"2%",
    },
    {L"Сумма",
    L"Требуется посчитать сумму целых чисел, расположенных между числами 1 и N включительно.",
    L"В единственной строке входного файла INPUT.TXT записано единственное целое число N, не превышающее по абсолютной величине 104.",
    L"В единственную строку выходного файла OUTPUT.TXT нужно вывести одно целое число — сумму чисел, расположенных между 1 и N включительно.",
    Summ::executor,
    nullptr,
    L"0002",
    L"19%",
    },
    {L"Сортировка подсчетом",
    L"На планете «Аурон» атмосфера практически отсутствует, поэтому она известна своими перепадами температур в различных точках. Известно, что эти перепады колеблются от -100 до 100 градусов. Нашим специалистам удалось выяснить значения температур в N точках этой планеты. К сожалению, эти значения вычислены с большими погрешностями, поэтому их решили округлить до целых чисел. Хотелось бы наглядно видеть участки с повышенной и пониженной температурой. Вам требуется помочь. Вы должны упорядочить температуры участков по неубыванию.",
    L"В первой строке входного файла INPUT.TXT задано натуральное число N - количество участков (N ≤ 106). Во второй строке через пробел записаны целые значения температур этих участков, не превосходящие 100 по абсолютной величине.",
    L"В единственную строку выходного файла OUTPUT.TXT нужно вывести разделенные пробелом значения температур всех известных участков, которые должны следовать друг за другом в порядке неубывания.",
    SortPodstetom::executor,
    SortPodstetom::test,
    L"0041",
    L"29%",
    },
    {L"Драконы",
    L"Известно, что у дракона может быть несколько голов и его сила определяется числом голов. Но как определить силу драконьей стаи, в которой несколько драконов и у каждого из них определенное число голов? Вероятно, вы считаете, что это значение вычисляется как сумма всех голов? Это далеко не так, иначе было бы слишком просто вычислить силу драконьей стаи. Оказывается, что искомое значение равно произведению значений числа голов каждого из драконов. Например, если в стае 3 дракона, у которых 3, 4 и 5 голов соответственно, то сила равна 3*4*5 = 60. Предположим, что нам известно суммарное количество голов драконьей стаи, как нам вычислить максимально возможное значение силы этого логова драконов? Именно эту задачу Вам и предстоит решить.",
    L"В единственной строке входного файла INPUT.TXT записано натуральное число N (0 < N < 100) – количество голов драконьей стаи.",
    L"В единственную строку выходного файла OUTPUT.TXT нужно вывести максимально возможное значение силы, которая может быть у стаи драконов из N голов.",
    Dragons::executor,
    nullptr,
    L"0041",
    L"29%",
    },
    {L"Стрелки",
    L"Задана последовательность, состоящая только из символов ‘>’, ‘<’ и ‘-‘. Требуется найти количество стрел, которые спрятаны в этой последовательности. Стрелы – это подстроки вида ‘>>-->’ и ‘<--<<’.",
    L"В первой строке входного файла INPUT.TXT записана строка, состоящая из символов ‘>’, ‘<’ и ‘-‘ (без пробелов). Строка состоит не более, чем из 250 символов.",
    L"В единственную строку выходного файла OUTPUT.TXT нужно вывести искомое количество стрелок.",
    Strelki::executor,
    nullptr,
    L"0044",
    L"20%",
    },
    {L"Сжимающий оператор",
    L"Оператором А, действующим из множества Х в множество Y (или просто оператором из X в Y) называется правило, согласно которому каждому элементу x множества X сопоставляется элемент y=Ax из множества Y. Пусть X и Y – множества точек на плоскости. Оператор A из X в Y называется сжимающим с коэффициентом q, где q – вещественное число из полуинтервала [0, 1), если для любого x из X выполнено ||Ax|| ≤ q*||x|| (здесь ||x|| - норма точки x – расстояние от x до начала координат). Проще говоря, оператор называется сжимающим с коэффициентом q если он сопоставляет каждой точке точку, которая не менее, чем в q раз ближе к началу координат.\nДля заданного оператора А требуется проверить является ли он сжимающим с коэффициентом q.",
    L"Первая строка входного файла INPUT.TXT содержит количество точек n (1 ≤ n ≤ 100) и число q (0 ≤ q < 1), заданное не более чем с 3 знаками после десятичной точки. Следующие n строк содержат по 4 целых числа, по модулю не превосходящих 1000, разделенные пробелами – координаты точки множества X и сопоставленной ей точки из множества Y.",
    L"В выходной файл OUTPUT.TXT выведите одно слово: “Yes” если оператор является сжимающим с коэффициентом q и “No” в противном случае.",
    ShimaushiOperator::executor,
    nullptr,
    L"0037",
    L"34%",
    },
    {L"Несложное вычисление",
    L"Задано натуральное число n. Необходимо перевести его в k-ичную систему счисления и найти разность между произведением и суммой его цифр в этой системе счисления.\nНапример, пусть n = 239, k = 8. Тогда представление числа n в восьмеричной системе счисления — 357, а ответ на задачу равен 3 × 5 × 7 − (3 + 5 + 7) = 90.",
    L"Входной файл INPUT.TXT содержит два натуральных числа: n и k (1 ≤ n ≤ 10^9, 2 ≤ k ≤ 10). Оба этих числа заданы в десятичной системе счисления.",
    L"В выходной файл OUTPUT.TXT выведите ответ на задачу (в десятичной системе счисления).",
    NotHardEval::executor,
    nullptr,
    L"0059",
    L"25%",
    },
    {L"Клавиатура",
    L"Для данной буквы английского алфавита нужно вывести справа стоящую букву на стандартной клавиатуре. При этом клавиатура замкнута, т.е. справа от буквы «p» стоит буква «a», от буквы «l» стоит буква «z», а от буквы «m» — буква «q».",
    L"Первая строка входного файла INPUT.TXT содержит один символ — маленькую букву английского алфавита.",
    L"В выходной файл OUTPUT.TXT следует вывести букву стоящую справа от заданной буквы, с учетом замкнутости клавиатуры.",
    Keyboard::executor,
    nullptr,
    L"0066",
    L"11%",
    },
    {L"Нумеролог",
    L"Чтобы предсказать судьбу человека, нумеролог берет время жизни человека в секундах, затем складывает все цифры этого числа. Если полученное число состоит более чем из одной цифры, операция повторяется, пока в числе не останется одна цифра. Затем по полученной цифре и числу операций, необходимых для преобразования числа в цифру нумеролог предсказывает судьбу человека. Нумеролог плохо умеет считать, а числа, с которыми он работает, могут быть очень большими. Напишите программу, которая бы делала все расчеты за него.",
    L"Входной файл INPUT.TXT содержит число N – время жизни человека в секундах (1 ≤ N ≤ 101000).",
    L"В выходной файл OUTPUT.TXT выведите два числа через пробел: полученную цифру из числа N и число преобразований.",
    Numberman::executor,
    nullptr,
    L"0095",
    L"24%",
    },
    {L"Арбузы",
    L"Иван Васильевич пришел на рынок и решил купить два арбуза: один для себя, а другой для тещи. Понятно, что для себя нужно выбрать арбуз потяжелей, а для тещи полегче. Но вот незадача: арбузов слишком много и он не знает как же выбрать самый легкий и самый тяжелый арбуз? Помогите ему!",
    L"В первой строке входного файла INPUT.TXT задано одно число N – количество арбузов. Вторая строка содержит N чисел, записанных через пробел. Здесь каждое число – это масса соответствующего арбуза. Все числа натуральные и не превышают 30000.",
    L"В выходной файл OUTPUT.TXT нужно вывести два числа через пробел: массу арбуза, который Иван Васильевич купит теще и массу арбуза, который он купит себе.",
    Arbus::executor,
    nullptr,
    L"0081",
    L"14%",
    },
    {L"Анаграммы",
    L"Cтрока S1 называется анаграммой строки S2, если она получается из S2 перестановкой символов. Даны строки S1 и S2. Напишите программу, которая проверяет, является ли S1 анаграммой S2.",
    L"Первая строка входного файла INPUT.TXT содержит строку S1, вторая - S2. Обе строки состоят только из прописных букв английского алфавита. Строки не пусты и имеют длину не больше 100000 символов.",
    L"В выходной файл OUTPUT.TXT выведите YES, если S1 является анаграммой S2, и NO - в противном случае.",
    Anagrammes::executor,
    Anagrammes::test,
    L"0081",
    L"14%",
    },
    {L"Число E",
    L"Выведите в выходной файл округленное до n знаков после десятичной точки число E. В данной задаче будем считать, что число Е в точности равно 2.7182818284590452353602875.",
    L"Входной файл INPUT.TXT содержит целое число n (0 ≤ n ≤ 25).",
    L"В выходной файл OUTPUT.TXT выведите ответ на задачу.",
    NumberE::executor,
    nullptr,
    L"0046",
    L"20%",
    },
    {L"Выход!!!",
    L"",
    L"",
    L"",
    []() {exit(0); },
    nullptr,
    L"",
    L"",
    true
    },

};
void internal_init() {
    // ONLY FOR WINDOWS
    _setmode(_fileno(stdout), _O_U16TEXT);
    _setmode(_fileno(stdin), _O_U16TEXT);
    _setmode(_fileno(stderr), _O_U16TEXT);
}
int main()
{
    internal_init();
    while (true) {
        for (int i = 0; i < sizeof(sols) / sizeof(solution); i++) {
            wcout << i << L". " << sols[i].name << endl;
        }
        int a = 0;
        wcin >> a;
        if (a > (sizeof(sols) / sizeof(solution)))
            continue;
        if (sols[a].virtual_sol) {
            sols[a].executor();
            continue;
        }
        wcout << L"t - тест" << endl;
        wcout << L"i - информация" << endl;
        wcout << L"r - запуск" << endl;
        wcout << L"m - меню" << endl;
        wchar_t c = '\0';
        wcin >> c;
        switch (c) {
        case 'i':
            wcout << sols[a].name << endl;
            wcout << sols[a].description << endl;
            wcout << sols[a].input_data << endl;
            wcout << sols[a].output_data << endl;
            if (!sols[a].acmp_id.empty()) {
                wcout << L"Номер задания на сайте acmp.ru: " << sols[a].acmp_id << endl;
                wcout << L"Сложность задания на сайте acmp.ru: " << sols[a].acmp_hard << endl;
            }
            break;
        case 'r':
            try {
                sols[a].executor();
            }
            catch (Error e) {
                switch (e.get_error_type()) {
                case Error::VALIDATE:
                    wcout << L"Неправильный аргумент:\n\n" << e.get_where() << L"\n\nу решения. " << L" Причина : " << e.get_why() << endl;
                    if (!wcin) {
                        wcin.clear();
                        wcin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    }
                    break;
                case Error::UNREACHABLE:
                    wcout << L"Внутренняя ошибка! Недосягаемый код : " << e.get_where() << L" Причина: " << e.get_why() << endl;
                    break;
                }
            }
            break;
        case 'm':
            break;
        case 't':
            if (sols[a].start_tests)
                sols[a].start_tests();
            else
                wcout << L"Тесты отсуствуют!" << endl;
            break;
        }
    }
}