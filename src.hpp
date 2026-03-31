#ifndef MEMO_HPP
#define MEMO_HPP

#include "event.h"
#include <iostream>

inline std::string CustomNotifyLateEvent::GetNotification(int n) const {
  return NotifyLateEvent::GetNotification(n) + generator_(n);
}

// Simple linked list node structure
enum EventType {
  NORMAL_EVENT,
  NOTIFY_BEFORE_EVENT,
  NOTIFY_LATE_EVENT
};

struct EventNode {
  const Event* event;
  EventType type;
  int notify_count;  // Track how many times this event has been notified
  EventNode* next;

  EventNode(const Event* e, EventType t) : event(e), type(t), notify_count(0), next(nullptr) {}
};

class Memo {
 public:
  Memo() = delete;

  Memo(int duration) : duration_(duration), current_time_(0) {
    // Create array of linked list heads for each time slot (1-based indexing)
    time_lists_ = new EventNode*[duration_ + 1];
    for (int i = 0; i <= duration_; ++i) {
      time_lists_[i] = nullptr;
    }
  }

  ~Memo() {
    // Clean up all remaining nodes in all lists
    for (int i = 0; i <= duration_; ++i) {
      EventNode* current = time_lists_[i];
      while (current != nullptr) {
        EventNode* temp = current;
        current = current->next;
        delete temp;
      }
    }
    delete[] time_lists_;
  }

  void AddEvent(const Event* event) {
    // Determine which time slot to add this event to and its type
    int target_time = 0;
    EventType event_type = NORMAL_EVENT;

    // Check the actual type of the event
    const NotifyBeforeEvent* notify_before = dynamic_cast<const NotifyBeforeEvent*>(event);
    const NotifyLateEvent* notify_late = dynamic_cast<const NotifyLateEvent*>(event);

    if (notify_before != nullptr) {
      // For NotifyBefore events, add to the notify_time directly
      target_time = notify_before->GetNotifyTime();
      event_type = NOTIFY_BEFORE_EVENT;
    } else if (notify_late != nullptr) {
      // For NotifyLate events (including CustomNotifyLate), add to deadline
      target_time = event->GetDeadline();
      event_type = NOTIFY_LATE_EVENT;
    } else {
      // For Normal events, add to deadline
      target_time = event->GetDeadline();
      event_type = NORMAL_EVENT;
    }

    // Add to the tail of the corresponding list
    EventNode* new_node = new EventNode(event, event_type);

    if (time_lists_[target_time] == nullptr) {
      time_lists_[target_time] = new_node;
    } else {
      EventNode* current = time_lists_[target_time];
      while (current->next != nullptr) {
        current = current->next;
      }
      current->next = new_node;
    }
  }

  void Tick() {
    current_time_++;

    if (current_time_ > duration_) {
      return;
    }

    // Process all events in the current time slot
    EventNode* current = time_lists_[current_time_];
    EventNode* prev = nullptr;

    while (current != nullptr) {
      const Event* event = current->event;

      if (event->IsComplete()) {
        // Event is complete, remove the node
        EventNode* to_delete = current;
        if (prev == nullptr) {
          time_lists_[current_time_] = current->next;
          current = current->next;
        } else {
          prev->next = current->next;
          current = current->next;
        }
        delete to_delete;
        continue;
      }

      // Event is not complete, need to notify
      EventType event_type = current->type;

      if (event_type == NOTIFY_BEFORE_EVENT) {
        const NotifyBeforeEvent* notify_before = static_cast<const NotifyBeforeEvent*>(event);
        // Check if this is the notify time or deadline
        int notify_time = notify_before->GetNotifyTime();
        int deadline = notify_before->GetDeadline();

        if (current_time_ == notify_time) {
          // This is the notify time
          std::cout << notify_before->GetNotification(0) << std::endl;

          // Move this node to the deadline
          EventNode* to_move = current;
          if (prev == nullptr) {
            time_lists_[current_time_] = current->next;
          } else {
            prev->next = current->next;
          }
          current = current->next;

          // Add to the tail of deadline list
          to_move->next = nullptr;
          if (time_lists_[deadline] == nullptr) {
            time_lists_[deadline] = to_move;
          } else {
            EventNode* tail = time_lists_[deadline];
            while (tail->next != nullptr) {
              tail = tail->next;
            }
            tail->next = to_move;
          }
          continue;
        } else if (current_time_ == deadline) {
          // This is the deadline
          std::cout << notify_before->GetNotification(1) << std::endl;

          // Remove the node
          EventNode* to_delete = current;
          if (prev == nullptr) {
            time_lists_[current_time_] = current->next;
            current = current->next;
          } else {
            prev->next = current->next;
            current = current->next;
          }
          delete to_delete;
          continue;
        } else {
          // This shouldn't happen based on the problem guarantee, but handle it anyway
          // Just move forward
          prev = current;
          current = current->next;
          continue;
        }
      } else if (event_type == NOTIFY_LATE_EVENT) {
        const NotifyLateEvent* notify_late = static_cast<const NotifyLateEvent*>(event);
        // NotifyLate or CustomNotifyLate event
        int frequency = notify_late->GetFrequency();

        // Output notification
        std::cout << notify_late->GetNotification(current->notify_count) << std::endl;
        current->notify_count++;

        // Move to next notification time (current_time + frequency)
        int next_time = current_time_ + frequency;

        EventNode* to_move = current;
        if (prev == nullptr) {
          time_lists_[current_time_] = current->next;
        } else {
          prev->next = current->next;
        }
        current = current->next;

        // Add to the tail of next_time list (if within duration)
        to_move->next = nullptr;
        if (next_time <= duration_) {
          if (time_lists_[next_time] == nullptr) {
            time_lists_[next_time] = to_move;
          } else {
            EventNode* tail = time_lists_[next_time];
            while (tail->next != nullptr) {
              tail = tail->next;
            }
            tail->next = to_move;
          }
        } else {
          // Beyond duration, just delete
          delete to_move;
        }
        continue;
      } else {
        // Normal event
        std::cout << event->GetNotification(0) << std::endl;

        // Remove the node
        EventNode* to_delete = current;
        if (prev == nullptr) {
          time_lists_[current_time_] = current->next;
          current = current->next;
        } else {
          prev->next = current->next;
          current = current->next;
        }
        delete to_delete;
        continue;
      }
    }
  }

 private:
  int duration_;
  int current_time_;
  EventNode** time_lists_;  // Array of linked list heads
};

#endif
