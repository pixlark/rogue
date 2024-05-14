#include "services.h"

//
// null services
//

class NullWorldService : public IWorldService {
};

//
// Services
//

std::shared_ptr<IWorldService> Services::worldService;

void Services::initialize() {
    worldService = std::make_shared<NullWorldService>();
}

void Services::provide(std::shared_ptr<IWorldService> worldService) {
    Services::worldService = std::move(worldService);
}
