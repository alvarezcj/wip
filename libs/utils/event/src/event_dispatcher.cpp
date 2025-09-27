#include "event_dispatcher.h"

namespace wip::utils::event {

bool EventDispatcher::unsubscribe(SubscriptionHandle handle) {
    if (!handle.is_valid()) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    
    for (auto& [type_index, handler_list] : handlers_) {
        auto it = std::find_if(handler_list.begin(), handler_list.end(),
                              [handle](const HandlerInfo& info) {
                                  return info.id == handle.id();
                              });
        
        if (it != handler_list.end()) {
            handler_list.erase(it);
            return true;
        }
    }
    
    return false;
}

size_t EventDispatcher::total_subscription_count() const {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    
    size_t total = 0;
    for (const auto& [type_index, handler_list] : handlers_) {
        total += handler_list.size();
    }
    
    return total;
}

void EventDispatcher::clear_all_subscriptions() {
    std::lock_guard<std::mutex> lock(handlers_mutex_);
    handlers_.clear();
}

EventDispatcher& global_dispatcher() {
    static EventDispatcher instance;
    return instance;
}

} // namespace wip::utils::event