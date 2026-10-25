#ifndef TIECODEC_EXTENSIONS_EVENT_CHANNEL_H
#define TIECODEC_EXTENSIONS_EVENT_CHANNEL_H

#include <functional>
#include <utility>
#include <vector>

namespace tiecodec {

template <typename Event>
class EventChannel {
public:
  using Handler = std::function<void(Event&)>;

  void Subscribe(Handler handler) {
    handlers_.push_back(std::move(handler));
  }

  void Publish(Event& event) const {
    for (const Handler& handler : handlers_)
      handler(event);
  }

  std::size_t size() const {
    return handlers_.size();
  }

private:
  std::vector<Handler> handlers_;
};

}

#endif
