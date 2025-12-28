set_project("EmilyTrader")
add_rules("mode.debug", "mode.release")

-- 添加依赖
add_requires("quill", "nlohmann_json")

-- 公共配置
set_languages("c++17")
add_includedirs("include")
if is_plat("linux") then
    add_syslinks("rt", "pthread")
end

-- 应用 1: Simulator
target("simulator")
    set_kind("binary")
    add_files("apps/simulator/main.cpp")
    add_packages("quill", "nlohmann_json")

-- 策略动态库
target("simple_strategy")
    set_kind("shared")
    add_files("src/strategy/simple_strategy.cpp")
    add_packages("quill", "nlohmann_json")

-- 策略加载器
target("strategy_loader")
    set_kind("binary")
    add_files("apps/strategy_loader/main.cpp")
    add_packages("quill", "nlohmann_json")

-- 应用: System Engine (Journal Manager)
target("engine")
    set_kind("binary")
    add_files("apps/engine/main.cpp")
    add_packages("quill", "nlohmann_json")

-- 应用: Mock Exchange
target("exchange")
    set_kind("binary")
    add_files("apps/exchange/main.cpp")
    add_packages("quill", "nlohmann_json")

-- 应用 3: Event Logger
target("event_logger")
    set_kind("binary")
    add_files("apps/event_logger/main.cpp")
    add_packages("quill", "nlohmann_json")
