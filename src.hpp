#ifndef MEMO_HPP
#define MEMO_HPP

#include "event.h"
#include <iostream>
#include <vector>

inline std::string CustomNotifyLateEvent::GetNotification(int n) const {
  return NotifyLateEvent::GetNotification(n) + generator_(n);
}

class Memo {
 public:
  Memo() = delete;

  Memo(int duration) : duration_(duration), current_time_(0) {}

  ~Memo() {
    // Events will be deleted by caller, we don't own them
  }

  void AddEvent(const Event* event) {
    events_.push_back(event);
  }

  void Tick() {
    current_time_++;

    if (current_time_ > duration_) {
      return;
    }

    // Check each event to see if it needs to output something at current time
    for (const Event* event : events_) {
      if (event == nullptr || event->IsComplete()) {
        continue;
      }

      // Check the type and process accordingly
      const NotifyBeforeEvent* notify_before = dynamic_cast<const NotifyBeforeEvent*>(event);
      const NotifyLateEvent* notify_late = dynamic_cast<const NotifyLateEvent*>(event);

      if (notify_before != nullptr) {
        // NotifyBefore event
        int notify_time = notify_before->GetNotifyTime();
        int deadline = notify_before->GetDeadline();

        if (current_time_ == notify_time) {
          std::cout << notify_before->GetNotification(0) << std::endl;
        }
        if (current_time_ == deadline) {
          std::cout << notify_before->GetNotification(1) << std::endl;
        }
      } else if (notify_late != nullptr) {
        // NotifyLate event (including CustomNotifyLate)
        int deadline = notify_late->GetDeadline();
        int frequency = notify_late->GetFrequency();

        // Check if we should notify at this time
        if (current_time_ >= deadline && (current_time_ - deadline) % frequency == 0) {
          int notify_index = (current_time_ - deadline) / frequency;
          std::cout << notify_late->GetNotification(notify_index) << std::endl;
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
  std::vector<const Event*> events_;
};

#endif
