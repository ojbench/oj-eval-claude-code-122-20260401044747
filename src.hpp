#ifndef MEMO_HPP
#define MEMO_HPP

#include "event.h"
#include <iostream>
#include <map>
#include <set>
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
  int deadline;
  int notify_time;  // For NotifyBefore
  int frequency;    // For NotifyLate

  EventInfo(const Event* e, EventType t, int d, int nt = 0, int f = 0)
    : event(e), type(t), notify_count(0), deadline(d), notify_time(nt), frequency(f) {}
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
    int deadline = event->GetDeadline();
    int notify_time = 0;
    int frequency = 0;

    // Determine event type once at add time and cache relevant info
    const NotifyBeforeEvent* notify_before = dynamic_cast<const NotifyBeforeEvent*>(event);
    const NotifyLateEvent* notify_late = dynamic_cast<const NotifyLateEvent*>(event);

    if (notify_before != nullptr) {
      type = NOTIFY_BEFORE_EVENT;
      notify_time = notify_before->GetNotifyTime();
      // Add to both notify_time and deadline time slots
      time_map_[notify_time].emplace_back(event, type, deadline, notify_time, 0);
      if (deadline != notify_time) {
        time_map_[deadline].emplace_back(event, type, deadline, notify_time, 0);
      }
      return;
    } else if (notify_late != nullptr) {
      type = NOTIFY_LATE_EVENT;
      frequency = notify_late->GetFrequency();
    }

    // For normal and notify_late events, add to deadline
    time_map_[deadline].emplace_back(event, type, deadline, notify_time, frequency);
  }

  void Tick() {
    current_time_++;

    if (current_time_ > duration_) {
      return;
    }

    // Only process events that are relevant to current time
    auto it = time_map_.find(current_time_);
    if (it == time_map_.end()) {
      return;  // No events at this time
    }

    std::vector<EventInfo>& events_at_time = it->second;
    std::vector<EventInfo> events_to_reschedule;

    for (EventInfo& info : events_at_time) {
      const Event* event = info.event;

      if (event->IsComplete()) {
        continue;
      }

      if (info.type == NOTIFY_BEFORE_EVENT) {
        // Check if this is notify_time or deadline
        if (current_time_ == info.notify_time) {
          // At notify_time, always output first notification
          std::cout << event->GetNotification(0) << std::endl;
          notified_before_events_.insert(event);
        } else if (current_time_ == info.deadline) {
          // At deadline, output second notification (only if we notified before)
          if (notified_before_events_.find(event) != notified_before_events_.end()) {
            std::cout << event->GetNotification(1) << std::endl;
          }
        }
      } else if (info.type == NOTIFY_LATE_EVENT) {
        // NotifyLate event
        std::cout << event->GetNotification(info.notify_count) << std::endl;
        info.notify_count++;

        // Schedule next notification
        if (!event->IsComplete()) {
          int next_time = current_time_ + info.frequency;
          if (next_time <= duration_) {
            events_to_reschedule.push_back(info);
            events_to_reschedule.back().deadline = next_time;  // Update when to check next
          }
        }
      } else {
        // Normal event
        std::cout << event->GetNotification(0) << std::endl;
      }
    }

    // Add rescheduled events to their new time slots
    for (const EventInfo& info : events_to_reschedule) {
      int next_time = info.deadline;
      time_map_[next_time].push_back(info);
    }
  }

 private:
  int duration_;
  int current_time_;
  std::map<int, std::vector<EventInfo>> time_map_;  // Map from time to events
  std::set<const Event*> notified_before_events_;   // Track which NotifyBefore events have been notified
};

#endif
