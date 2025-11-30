#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <type_traits>

template<typename RES, typename... ARGS>
class my_function;

template<typename RES, typename... ARGS>
class my_function<RES(ARGS...)> {
public:
    my_function() : func(nullptr) {}
    ~my_function() { delete func; }
    my_function(const my_function& other) :
        func {other.func ? other.func->clone() : nullptr} {}
    operator bool() const { return func != nullptr; }

    my_function& operator=(const my_function& rhs) {
        if(this != &rhs) {
            delete func;
            func = rhs.func ? rhs.func->clone() : nullptr;
        }
        return *this;
    }
    my_function& operator=(my_function&& rhs) noexcept {
        if(this != &rhs) {
            delete func;
            func = rhs.func;
            rhs.func = nullptr;
        }
        return *this;
    }
    my_function& operator=(std::nullptr_t) {
        delete func;
        func = nullptr;
        return *this;
    }
    template<typename T,
             typename = std::enable_if_t<
                 !std::is_same_v<std::decay_t<T>, std::nullptr_t>>>
    my_function& operator=(T&& f) {
        delete func;
        func = new my_function_helper<std::decay_t<T>>(std::forward<T>(f));
        return *this;
    }

    RES operator()(ARGS... args) {
        if(func == nullptr) {
            throw std::bad_function_call();
        }
        return func->call(args...);
    }

private:
    struct my_function_helper_base {
        my_function_helper_base() = default;
        virtual ~my_function_helper_base() = default;
        virtual my_function_helper_base* clone() const = 0;
        virtual RES call(ARGS... args) = 0;
    };

    template<typename T>
    struct my_function_helper : public my_function_helper_base {
        T func;
        my_function_helper(const T& f) : func(f) {}
        my_function_helper(T&& f) : func(std::move(f)) {}
        my_function_helper* clone() const override {
            return new my_function_helper(func);
        }
        RES call(ARGS... args) override { return func(args...); }
    };
    my_function_helper_base* func;
};

int main() {
    my_function<double(double)> f;

    if(!f) std::cout << "Egyelőre nullptr" << std::endl;

    f = sin;
    std::cout << sin(2.3) << "==" << f(2.3) << std::endl;

    f = [](double x) { return x * x; };
    std::cout << 2.3 * 2.3 << "==" << f(2.3) << std::endl;

    f = std::bind(pow, std::placeholders::_1, 4);
    std::cout << pow(2.3, 4) << "==" << f(2.3) << std::endl;

    auto f2 = f; /* másolható */
    std::cout << pow(2.3, 4) << "==" << f2(2.3) << std::endl;

    f = nullptr;
    try {
        f(2.3);
    } catch(std::bad_function_call& e) {
        std::cout << "Megint nullptr" << std::endl;
    }
}