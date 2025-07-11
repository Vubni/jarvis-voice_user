#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H

#include <string>
#include <vector>
#include <any>
#include <map>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <functional>
#include <regex>
#include <sstream>
#include <algorithm>
#include "core.h"
#include <iostream>

class ScriptEngine {
public:
    ScriptEngine();
    ~ScriptEngine();

    template <typename Func>
    void RegisterFunction(const std::string& name, Func func, const std::vector<std::any>& defaults = {});

    void Execute(const std::string& script);

private:
    class FunctionBase {
    public:
        virtual ~FunctionBase() = default;
        virtual void Call(const std::vector<std::any>& args) = 0;
        virtual size_t GetTotalParams() const = 0;
        virtual size_t GetDefaultParamsCount() const = 0;
        virtual const std::vector<std::any>& GetDefaults() const = 0;
    };

    template <typename ArgsTuple>
    class FunctionWrapper;

    template <typename... Args>
    class FunctionWrapper<std::tuple<Args...>> : public FunctionBase {
    public:
        using FuncType = void(*)(Args...);
        FunctionWrapper(FuncType func, const std::vector<std::any>& defaults)
            : m_func(func), m_defaults(defaults) {
            if (defaults.size() > sizeof...(Args)) {
                throw std::invalid_argument("Defaults count exceeds parameter count");
            }
        }

        void Call(const std::vector<std::any>& args) override {
            std::vector<std::any> all_args = args;
            size_t total_params = sizeof...(Args);
            size_t provided = args.size();
            size_t defaults_needed = total_params - provided;
            if (defaults_needed > m_defaults.size()) {
                throw std::runtime_error("Not enough default values for function");
            }
            for (size_t i = 0; i < defaults_needed; ++i) {
                all_args.push_back(m_defaults[i]);
            }
            CallImpl(all_args, std::index_sequence_for<Args...>());
        }

        size_t GetTotalParams() const override {
            return sizeof...(Args);
        }

        size_t GetDefaultParamsCount() const override {
            return m_defaults.size();
        }

        const std::vector<std::any>& GetDefaults() const override {
            return m_defaults;
        }

    private:
        template <size_t... I>
        void CallImpl(const std::vector<std::any>& args, std::index_sequence<I...>) {
            m_func(std::any_cast<Args>(args[I])...);
        }

        FuncType m_func;
        std::vector<std::any> m_defaults;
    };

    template <typename Func>
    struct FunctionTraits;

    template <typename... Args>
    struct FunctionTraits<void(*)(Args...)> {
        using ArgsTuple = std::tuple<Args...>;
        static constexpr size_t arity = sizeof...(Args);
    };

    std::map<std::string, std::unique_ptr<FunctionBase>> m_functions;

    std::vector<std::string> SplitCommands(const std::string& script);
    std::pair<std::string, std::vector<std::any>> ParseCommand(const std::string& command);
    std::vector<std::string> SplitArgs(const std::string& args_str);
    std::any ConvertArg(const std::string& arg_str);
    std::string Trim(const std::string& s);
};

template <typename Func>
void ScriptEngine::RegisterFunction(const std::string& name, Func func, const std::vector<std::any>& defaults) {
    using Traits = ScriptEngine::FunctionTraits<Func>;
    using ArgsTuple = typename Traits::ArgsTuple;
    m_functions[name] = std::make_unique<FunctionWrapper<ArgsTuple>>(func, defaults);
}

#endif // COMMAND_PROCESSOR_H