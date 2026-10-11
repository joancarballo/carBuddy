#include "services/calendar/EventCollector.h"

#include <algorithm>

#include "services/calendar/Recurrence.h"

using CivilTime::Minutes;

namespace {

// Resumen corto del UID (FNV-1a) para comparar sin guardar el texto entero.
uint32_t hashUid(const std::string& uid) {
  uint32_t h = 2166136261u;
  for (size_t i = 0; i < uid.size(); i++) {
    h ^= (uint8_t)uid[i];
    h *= 16777619u;
  }
  return h;
}

bool overlaps(Minutes start, Minutes end, Minutes from, Minutes to) {
  if (end <= start) {
    return start >= from && start < to;  // evento sin duración
  }
  return start < to && end > from;
}

}  // namespace

EventCollector::EventCollector(int64_t firstDay, int dayCount) : firstDay_(firstDay), dayCount_(dayCount) {}

void EventCollector::addCallback(const IcsEvent& ev, void* collector) {
  static_cast<EventCollector*>(collector)->add(ev);
}

void EventCollector::add(const IcsEvent& ev) {
  const Minutes from = firstDay_ * CivilTime::MINUTES_PER_DAY;
  const Minutes to = (firstDay_ + dayCount_) * CivilTime::MINUTES_PER_DAY;

  // Repetición modificada de otro evento: anula la original, se muestre o no.
  if (ev.recurrenceId >= 0) {
    overrides_.push_back(std::make_pair(hashUid(ev.uid), ev.recurrenceId));
    if (!ev.cancelled && overlaps(ev.start, ev.end, from, to)) {
      addOccurrence(ev, ev.start, false, std::string());
    }
    return;
  }

  if (ev.cancelled) {
    return;
  }

  Recurrence::Rule rule;
  if (!ev.rrule.empty() && Recurrence::parse(ev.rrule, rule)) {
    std::vector<Minutes> starts;
    Recurrence::occurrences(ev, rule, from, to, starts);
    const std::string repeat = starts.empty() ? std::string() : Recurrence::describe(rule);
    for (size_t i = 0; i < starts.size(); i++) {
      addOccurrence(ev, starts[i], true, repeat);
    }
    return;
  }
  if (!ev.rrule.empty() && unsupportedRules_.size() < 5) {
    unsupportedRules_.push_back(ev.rrule);
  }

  if (overlaps(ev.start, ev.end, from, to)) {
    addOccurrence(ev, ev.start, false, std::string());
  }
}

void EventCollector::addOccurrence(const IcsEvent& ev, Minutes start, bool fromRule, const std::string& repeat) {
  if (occurrences_.size() >= MAX_EVENTS) {
    return;
  }
  Occurrence occ;
  occ.uidHash = hashUid(ev.uid);
  occ.originalStart = start;
  occ.fromRule = fromRule;
  occ.event.title = ev.summary.empty() ? "(Sin título)" : ev.summary;
  occ.event.location = ev.location;
  occ.event.notes = ev.description;
  occ.event.repeat = repeat;
  occ.event.uidHash = occ.uidHash;
  occ.event.allDay = ev.allDay;
  occ.event.start = start;
  occ.event.end = start + (ev.end - ev.start);
  occurrences_.push_back(occ);
}

std::vector<CalendarEvent> EventCollector::finish() {
  std::vector<CalendarEvent> list;

  for (size_t i = 0; i < occurrences_.size(); i++) {
    const Occurrence& occ = occurrences_[i];

    if (occ.fromRule) {
      bool overridden = false;
      for (size_t j = 0; j < overrides_.size() && !overridden; j++) {
        overridden = overrides_[j].first == occ.uidHash && overrides_[j].second == occ.originalStart;
      }
      if (overridden) {
        continue;
      }
    }

    // Un evento aparece en la lista de cada día que toca.
    for (int d = 0; d < dayCount_; d++) {
      const int64_t day = firstDay_ + d;
      const Minutes dayStart = day * CivilTime::MINUTES_PER_DAY;
      if (overlaps(occ.event.start, occ.event.end, dayStart, dayStart + CivilTime::MINUTES_PER_DAY) &&
          list.size() < MAX_EVENTS) {
        list.push_back(occ.event);
        list.back().day = day;
      }
    }
  }

  std::sort(list.begin(), list.end(), eventListOrder);
  full_ = occurrences_.size() >= MAX_EVENTS;
  occurrences_.clear();
  overrides_.clear();
  return list;
}
