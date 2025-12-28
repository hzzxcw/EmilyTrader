#include "strategy/simple_strategy.h"

extern "C" {
    quote::strategy::BaseStrategy* create_strategy() {
        return new quote::strategy::SimpleStrategy();
    }
}
