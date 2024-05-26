#pragma once

#include <stdexcept>
#include <memory>

class IWorldService;
class IScreenManagerService;
class ILoggerService;

class Services {
    static std::shared_ptr<IWorldService> world_service;
    static std::shared_ptr<IScreenManagerService> screen_manager_service;
    static std::shared_ptr<ILoggerService> logger_service;

public:
    static void initialize();
    static void destruct();

    static void provide(std::shared_ptr<IWorldService> world_service);
    static void provide(std::shared_ptr<IScreenManagerService> screen_manager_service);
    static void provide(std::shared_ptr<ILoggerService> logger_service);

    template <typename T>
    static std::shared_ptr<T> get();
};
