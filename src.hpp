#ifndef MEMO_HPP
#define MEMO_HPP

#include "event.h"
#include <iostream>
#include <vector>

inline std::string CustomNotifyLateEvent::GetNotification(int n) const {
  return NotifyLateEvent::GetNotification(n) + generator_(n);
}

enum EventType {
  NORMAL_EVENT,
  NOTIFY_BEFORE_EVENT,
  NOTIFY_LATE_EVENT
};

struct EventInfo {
  const Event* event;
  EventType type;
  int notify_count;

  EventInfo(const Event* e, EventType t) : event(e), type(t), notify_count(0) {}
};

class Memo {
 public:
  Memo() = delete;

  Memo(int duration) : duration_(duration), current_time_(0) {}

  ~Memo() {
    // Events will be deleted by caller, we don't own them
  }

  void AddEvent(const Event* event) {
    EventType type = NORMAL_EVENT;

    // Determine event type once at add time
    const NotifyBeforeEvent* notify_before = dynamic_cast<const NotifyBeforeEvent*>(event);
    const NotifyLateEvent* notify_late = dynamic_cast<const NotifyLateEvent*>(event);

    if (notify_before != nullptr) {
      type = NOTIFY_BEFORE_EVENT;
    } else if (notify_late != nullptr) {
      type = NOTIFY_LATE_EVENT;
    }

    events_.emplace_back(event, type);
  }

  void Tick() {
    current_time_++;

    if (current_time_ > duration_) {
      return;
    }

    // Check each event to see if it needs to output something at current time
    for (EventInfo& info : events_) {
      const Event* event = info.event;

      if (event == nullptr || event->IsComplete()) {
        continue;
      }

      if (info.type == NOTIFY_BEFORE_EVENT) {
        // NotifyBefore event
        const NotifyBeforeEvent* notify_before = static_cast<const NotifyBeforeEvent*>(event);
        int notify_time = notify_before->GetNotifyTime();
        int deadline = notify_before->GetDeadline();

        if (current_time_ == notify_time) {
          std::cout << notify_before->GetNotification(0) << std::endl;
        }
        if (current_time_ == deadline) {
          std::cout << notify_before->GetNotification(1) << std::endl;
        }
      } else if (info.type == NOTIFY_LATE_EVENT) {
        // NotifyLate event (including CustomNotifyLate)
        const NotifyLateEvent* notify_late = static_cast<const NotifyLateEvent*>(event);
        int deadline = notify_late->GetDeadline();
        int frequency = notify_late->GetFrequency();

        // Check if we should notify at this time
        if (current_time_ >= deadline && (current_time_ - deadline) % frequency == 0) {
          std::cout << notify_late->GetNotification(info.notify_count) << std::endl;
          info.notify_count++;
        }
      } else {
        // Normal event
        int deadline = event->GetDeadline();
        if (current_time_ == deadline) {
          std::cout << event->GetNotification(0) << std::endl;
        }
      }
    }
  }

 private:
  int duration_;
  int current_time_;
  std::vector<EventInfo> events_;
};

#endif
