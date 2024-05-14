#pragma once

#include <memory>

class IWorldService {
public:
};

class Services {
    static std::shared_ptr<IWorldService> worldService;

public:
    static void initialize();
    static void provide(std::shared_ptr<IWorldService> worldService);
};
