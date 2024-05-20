#include "services.h"

//
// Services
//

std::shared_ptr<IWorldService> Services::world_service;
std::shared_ptr<IScreenManagerService> Services::screen_manager_service;
std::shared_ptr<ILoggerService> Services::logger_service;

void Services::initialize() {
    Services::world_service = nullptr;
    Services::screen_manager_service = nullptr;
    Services::logger_service = nullptr;
}

void Services::destruct() {
    Services::world_service = nullptr;
    Services::screen_manager_service = nullptr;
    Services::logger_service = nullptr;
}

void Services::provide(std::shared_ptr<IWorldService> world_service) {
    Services::world_service = std::move(world_service);
}

void Services::provide(std::shared_ptr<IScreenManagerService> screen_manager_service) {
    Services::screen_manager_service = std::move(screen_manager_service);
}

void Services::provide(std::shared_ptr<ILoggerService> logger_service) {
    Services::logger_service = std::move(logger_service);
}

template<>
std::shared_ptr<IWorldService> Services::get<IWorldService>() {
    if (Services::world_service == nullptr) {
        throw std::runtime_error("IWorldService was never provided!");
    }

    return Services::world_service;
}

template<>
std::shared_ptr<IScreenManagerService> Services::get<IScreenManagerService>() {
    if (Services::screen_manager_service == nullptr) {
        throw std::runtime_error("IScreenManagerService was never provided!");
    }

    return Services::screen_manager_service;
}

template<>
std::shared_ptr<ILoggerService> Services::get<ILoggerService>() {
    if (Services::logger_service == nullptr) {
        throw std::runtime_error("ILoggerService was never provided!");
    }

    return Services::logger_service;
}
