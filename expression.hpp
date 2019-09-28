#ifndef EXPRESSION_HPP
#define EXPRESSION_HPP

#include <functional>
#include <type_traits>
#include <utility>
#include <tuple>
#include <memory>

namespace expr
{
    template <typename Expr>
    constexpr decltype(auto) expressify(Expr&& expr);

    template <typename T>
    inline constexpr bool is_expression = false;

    template <typename Operation>
    class expression;

    template <typename Operation>
    expression<Operation> make_expression(Operation op)
        noexcept(std::is_nothrow_copy_constructible_v<Operation>);

    template <typename Operation>
    class expression
    {
    public:
        explicit constexpr expression(Operation op)
            noexcept(std::is_nothrow_copy_constructible_v<Operation>)
            : _op(op)
        {}

        template <typename... Args>
            requires std::is_invocable_v<Operation&, Args&...>
        constexpr decltype(auto) operator()(Args&&... args) const
            noexcept(std::is_nothrow_invocable_v<Operation&, Args&...>)
        {
            return _op(args...);
        }

        template <typename Rhs>
        constexpr auto operator=(Rhs&& rhs) const&
        {
            return make_expression([op = _op, rhs = expressify(std::forward<Rhs>(rhs))]
                                   (auto&&... args) -> decltype(auto) {
                return op(args...) = rhs(args...);
            });
        }

        template <typename Rhs>
        constexpr auto operator=(Rhs&& rhs) &&
        {
            return make_expression([op = std::move(_op), rhs = expressify(std::forward<Rhs>(rhs))]
                                   (auto&&... args) -> decltype(auto) {
                return op(args...) = rhs(args...);
            });
        }

        template <typename Index>
        constexpr auto operator[](Index&& index) const&
        {
            return make_expression([op = _op, index = expressify(std::forward<Index>(index))]
                                   (auto&&... args) -> decltype(auto) {
                return op[index(args...)];
            });
        }

        template <typename Index>
        constexpr auto operator[](Index&& index) &&
        {
            return make_expression([op = std::move(_op), index = expressify(std::forward<Index>(index))]
                                   (auto&&... args) -> decltype(auto) {
                return op[index(args...)];
            });
        }

    private:
        mutable Operation _op;
    };

    template <typename Operation>
    inline constexpr bool is_expression<expression<Operation>> = true;

    template <typename Operation>
    expression<Operation> make_expression(Operation op)
        noexcept(std::is_nothrow_copy_constructible_v<Operation>)
    {
        return expression(op);
    }

    template <typename Type>
    class variable
    {
    public:
        explicit constexpr variable(Type& var)
            noexcept
            : _var(std::addressof(var))
        {}

        template <typename... Args>
        constexpr Type& operator()([[maybe_unused]] Args&&... args) const noexcept
        {
            return *_var;
        }

        template <typename Rhs>
        constexpr auto operator=(Rhs&& rhs) const
        {
            return expression([var = _var, rhs = expressify(std::forward<Rhs>(rhs))]
                              (auto&&... args) -> decltype(auto) {
                return *var = rhs(args...);
            });
        }

        template <typename Index>
        constexpr auto operator[](Index&& index) const
        {
            return expression([var = _var, index = expressify(std::forward<Index>(index))]
                              (auto&&... args) -> decltype(auto) {
                return (*var)[index(args...)];
            });
        }

    private:
        Type* _var;
    };

    template <typename Type>
    inline constexpr bool is_expression<variable<Type>> = true;

    template <typename Type>
    class constant
    {
    public:
        explicit constexpr constant(const Type& val)
            noexcept(std::is_nothrow_copy_constructible_v<Type>)
            requires std::is_copy_constructible_v<Type>
            : _val(val)
        {}

        explicit constexpr constant(Type&& val)
            noexcept(std::is_nothrow_move_constructible_v<Type>)
            requires std::is_move_constructible_v<Type>
            : _val(std::move(val))
        {}

        template <typename... Args>
        constexpr const Type& operator()([[maybe_unused]] Args&&... args) const noexcept
        {
            return _val;
        }

        template <typename Index>
        constexpr auto operator[](Index&& index) const&
        {
            return expression([val = _val, index = expressify(std::forward<Index>(index))]
                              (auto&&... args) -> decltype(auto) {
                return val[index(args...)];
            });
        }

        template <typename Index>
        constexpr auto operator[](Index&& index) &&
        {
            return expression([val = std::move(_val), index = expressify(std::forward<Index>(index))]
                              (auto&&... args) -> decltype(auto) {
                return val[index(args...)];
            });
        }

    private:
        Type _val;
    };

    template <typename Type>
    inline constexpr bool is_expression<constant<Type>> = true;

    template <int I>
    struct placeholder
    {
        template <typename... Args>
        constexpr decltype(auto) operator()(Args&&... args) const noexcept
        {
            static_assert(I < sizeof...(args), "Placeholder out of range");
            return std::get<I>(std::tie(args...));
        }

        template <typename Member>
            requires std::is_member_pointer_v<Member>
        constexpr auto operator->*(Member member) const noexcept
        {
            if constexpr (std::is_member_function_pointer_v<Member>)
                return expression([member] (auto&&... params) {
                    return expression([member, ...params = expressify(std::forward<decltype(params)>(params))]
                                      (auto&&... args) -> decltype(auto) {
                        static_assert(I < sizeof...(args), "Placeholder out of range");
                        return std::invoke(member, std::get<I>(std::tie(args...)), params(args...)...);
                    });
                });
            else if constexpr (std::is_member_object_pointer_v<Member>)
                return expression([member] (auto&&... args) -> decltype(auto) {
                    static_assert(I < sizeof...(args), "Placeholder out of range");
                    return std::invoke(member, std::get<I>(std::tie(args...)));
                });
        }

        template <typename Index>
        constexpr auto operator[](Index&& index) const
        {
            return expression([index = expressify(std::forward<Index>(index))]
                              (auto&&... args) -> decltype(auto) {
                static_assert(I < sizeof...(args), "Placeholder out of range");
                return std::get<I>(std::tie(args...))[index(args...)];
            });
        }

        template <typename Rhs>
        constexpr auto operator=(Rhs&& rhs) const
        {
            return expression([rhs = expressify(std::forward<Rhs>(rhs))]
                              (auto&&... args) -> decltype(auto) {
                static_assert(I < sizeof...(args), "Placeholder out of range");
                return std::get<I>(std::tie(args...)) = rhs(args...);
            });
        }
    };

    template <int I>
    inline constexpr bool is_expression<placeholder<I>> = true;

    inline constexpr placeholder<0> _1 = {};
    inline constexpr placeholder<1> _2 = {};
    inline constexpr placeholder<2> _3 = {};
    inline constexpr placeholder<3> _4 = {};
    inline constexpr placeholder<4> _5 = {};
    inline constexpr placeholder<5> _6 = {};
    inline constexpr placeholder<6> _7 = {};

    template <typename Function, typename... Params>
    auto bind(Function&& func, Params&&... params)
    {
        return expression([func = expressify(std::forward<Function>(func)),
                           ...params = expressify(std::forward<Params>(params))]
                          (auto&&... args) -> decltype(auto) {
            return std::invoke(func(args...), params(args...)...);
        });
    }

    template <typename Expr>
    constexpr decltype(auto) expressify(Expr&& expr)
    {
        if constexpr (is_expression<std::decay_t<Expr>>) return std::forward<Expr>(expr);
        else if constexpr (std::is_lvalue_reference_v<Expr>) return variable(std::forward<Expr>(expr));
        else return constant(std::forward<Expr>(expr));
    }

#if (defined EXPR_UN_OP) || (defined EXPR_UN_OP_POST) || (defined EXPR_BIN_OP) || (defined EXPR_COMMA)
#error "Multiple defines!"
#endif

#define EXPR_UN_OP(OP)                                               \
    template <typename Rhs>                                          \
        requires is_expression<std::decay_t<Rhs>>                    \
    constexpr auto operator OP (Rhs&& rhs)                           \
    {                                                                \
        return expression([rhs = expressify(std::forward<Rhs>(rhs))] \
                          (auto&&... args) -> decltype(auto) {       \
            return OP rhs(args...);                                  \
        });                                                          \
    }

#define EXPR_UN_OP_POST(OP)                                          \
    template <typename Lhs>                                          \
        requires is_expression<std::decay_t<Lhs>>                    \
    constexpr auto operator OP (Lhs&& lhs, [[maybe_unused]] int)     \
    {                                                                \
        return expression([lhs = expressify(std::forward<Lhs>(lhs))] \
                          (auto&&... args) -> decltype(auto) {       \
            return lhs(args...) OP;                                  \
        });                                                          \
    }

#define EXPR_BIN_OP(OP)                                              \
    template <typename Lhs, typename Rhs>                            \
        requires is_expression<std::decay_t<Lhs>>                    \
              or is_expression<std::decay_t<Rhs>>                    \
    constexpr auto operator OP (Lhs&& lhs, Rhs&& rhs)                \
    {                                                                \
        return expression([lhs = expressify(std::forward<Lhs>(lhs)), \
                           rhs = expressify(std::forward<Rhs>(rhs))] \
                          (auto&&... args) -> decltype(auto) {       \
            return lhs(args...) OP rhs(args...);                     \
        });                                                          \
    }

#define EXPR_COMMA ,

EXPR_UN_OP(+)
EXPR_UN_OP(-)
EXPR_UN_OP(!)
EXPR_UN_OP(~)
EXPR_UN_OP(*)
EXPR_UN_OP(&)
EXPR_UN_OP(++)
EXPR_UN_OP(--)
EXPR_UN_OP_POST(++)
EXPR_UN_OP_POST(--)
EXPR_BIN_OP(+)
EXPR_BIN_OP(-)
EXPR_BIN_OP(*)
EXPR_BIN_OP(/)
EXPR_BIN_OP(%)
EXPR_BIN_OP(EXPR_COMMA)
EXPR_BIN_OP(&)
EXPR_BIN_OP(|)
EXPR_BIN_OP(^)
EXPR_BIN_OP(<<)
EXPR_BIN_OP(>>)
EXPR_BIN_OP(<)
EXPR_BIN_OP(<=)
EXPR_BIN_OP(>)
EXPR_BIN_OP(>=)
EXPR_BIN_OP(==)
EXPR_BIN_OP(!=)
EXPR_BIN_OP(&&)
EXPR_BIN_OP(||)
EXPR_BIN_OP(+=)
EXPR_BIN_OP(-=)
EXPR_BIN_OP(*=)
EXPR_BIN_OP(/=)
EXPR_BIN_OP(%=)
EXPR_BIN_OP(&=)
EXPR_BIN_OP(|=)
EXPR_BIN_OP(^=)
EXPR_BIN_OP(<<=)
EXPR_BIN_OP(>>=)

#undef EXPR_COMMA
#undef EXPR_BIN_OP
#undef EXPR_UN_OP_POST
#undef EXPR_UN_OP
} // namespace expr

#endif // EXPRESSION_HPP
