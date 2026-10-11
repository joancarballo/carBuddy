#include "services/calendar/IcsParser.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

namespace {

// Las notas se guardan hasta este tamaño: suficiente para leerlas en la pantalla
// y sin gastar memoria con descripciones enormes.
const size_t MAX_NOTES = 1000;

bool isContinuationByte(char c) {
  return ((uint8_t)c & 0xC0) == 0x80;
}

// Lee un número de n cifras desde s[pos]. Devuelve -1 si hay algo que no es una cifra.
int readDigits(const std::string& s, size_t pos, int n) {
  int v = 0;
  for (int i = 0; i < n; i++) {
    const char c = s[pos + i];
    if (c < '0' || c > '9') {
      return -1;
    }
    v = v * 10 + (c - '0');
  }
  return v;
}

// Duración iCal ("PT1H30M", "P1D", "P1W") en minutos, o -1 si no se entiende.
int64_t parseDuration(const std::string& s) {
  size_t i = 0;
  int sign = 1;
  if (i < s.size() && (s[i] == '+' || s[i] == '-')) {
    sign = s[i] == '-' ? -1 : 1;
    i++;
  }
  if (i >= s.size() || s[i] != 'P') {
    return -1;
  }
  i++;
  int64_t total = 0;
  int64_t number = 0;
  for (; i < s.size(); i++) {
    const char c = s[i];
    if (c >= '0' && c <= '9') {
      number = number * 10 + (c - '0');
      continue;
    }
    switch (c) {
      case 'T': break;
      case 'W': total += number * 7 * 24 * 60; break;
      case 'D': total += number * 24 * 60; break;
      case 'H': total += number * 60; break;
      case 'M': total += number; break;
      case 'S': total += number / 60; break;
      default: return -1;
    }
    number = 0;
  }
  return sign * total;
}

void appendUtf8(std::string& out, uint32_t cp) {
  if (cp < 0x80) {
    out += (char)cp;
  } else if (cp < 0x800) {
    out += (char)(0xC0 | (cp >> 6));
    out += (char)(0x80 | (cp & 0x3F));
  } else if (cp < 0x10000) {
    out += (char)(0xE0 | (cp >> 12));
    out += (char)(0x80 | ((cp >> 6) & 0x3F));
    out += (char)(0x80 | (cp & 0x3F));
  } else {
    out += (char)(0xF0 | (cp >> 18));
    out += (char)(0x80 | ((cp >> 12) & 0x3F));
    out += (char)(0x80 | ((cp >> 6) & 0x3F));
    out += (char)(0x80 | (cp & 0x3F));
  }
}

// Traduce una entidad HTML sin '&' ni ';' ("amp", "nbsp", "#39"...).
// Devuelve false si no la conoce.
bool decodeEntity(const std::string& name, std::string& out) {
  if (name == "amp") {
    out += '&';
  } else if (name == "lt") {
    out += '<';
  } else if (name == "gt") {
    out += '>';
  } else if (name == "quot") {
    out += '"';
  } else if (name == "apos") {
    out += '\'';
  } else if (name == "nbsp") {
    out += ' ';
  } else if (name.size() > 1 && name[0] == '#') {
    const bool hex = name[1] == 'x' || name[1] == 'X';
    const long cp = strtol(name.c_str() + (hex ? 2 : 1), nullptr, hex ? 16 : 10);
    if (cp <= 0 || cp > 0x10FFFF) {
      return false;
    }
    appendUtf8(out, (uint32_t)cp);
  } else {
    return false;
  }
  return true;
}

// Etiquetas que en las notas de Google separan líneas.
bool tagBreaksLine(const std::string& tag) {
  static const char* const BREAKS[] = {"br", "/p", "/div", "/li", "/h1", "/h2", "/h3", "/ul", "/ol"};
  for (size_t i = 0; i < sizeof(BREAKS) / sizeof(BREAKS[0]); i++) {
    if (tag == BREAKS[i]) {
      return true;
    }
  }
  return false;
}

}  // namespace

// ---------------------------------------------------------------------------
// Funciones de texto y fechas
// ---------------------------------------------------------------------------

namespace Ics {

std::string unescapeText(const std::string& s) {
  std::string out;
  out.reserve(s.size());
  for (size_t i = 0; i < s.size(); i++) {
    if (s[i] == '\\' && i + 1 < s.size()) {
      i++;
      out += (s[i] == 'n' || s[i] == 'N') ? '\n' : s[i];
    } else {
      out += s[i];
    }
  }
  return out;
}

std::string htmlToText(const std::string& s) {
  if (s.find('<') == std::string::npos && s.find('&') == std::string::npos) {
    return s;
  }

  std::string out;
  out.reserve(s.size());
  for (size_t i = 0; i < s.size(); i++) {
    const char c = s[i];

    // Etiqueta: solo si parece una de verdad ("<b>", "</p>", "<br/>"), para no
    // comerse un "<" suelto escrito en el texto.
    if (c == '<' && i + 1 < s.size() && (isalpha((uint8_t)s[i + 1]) || s[i + 1] == '/')) {
      const size_t close = s.find('>', i);
      if (close != std::string::npos && close - i < 300) {
        std::string tag;
        for (size_t j = i + 1; j < close && (isalpha((uint8_t)s[j]) || s[j] == '/' || isdigit((uint8_t)s[j])); j++) {
          tag += (char)tolower((uint8_t)s[j]);
        }
        if (tag.size() > 1 && tag.back() == '/') {
          tag.pop_back();  // <br/>
        }
        if (tagBreaksLine(tag)) {
          out += '\n';
        }
        i = close;
        continue;
      }
    }

    if (c == '&') {
      const size_t semi = s.find(';', i);
      if (semi != std::string::npos && semi - i <= 8) {
        if (decodeEntity(s.substr(i + 1, semi - i - 1), out)) {
          i = semi;
          continue;
        }
      }
    }

    out += c;
  }
  return out;
}

std::string truncateUtf8(const std::string& s, size_t maxBytes) {
  size_t cut = s.size() < maxBytes ? s.size() : maxBytes;
  if (cut == 0) {
    return std::string();
  }
  // Busca el principio del último carácter y comprueba que cabe entero.
  size_t start = cut - 1;
  while (start > 0 && isContinuationByte(s[start])) {
    start--;
  }
  const uint8_t lead = (uint8_t)s[start];
  size_t length = 1;
  if ((lead & 0xE0) == 0xC0) {
    length = 2;
  } else if ((lead & 0xF0) == 0xE0) {
    length = 3;
  } else if ((lead & 0xF8) == 0xF0) {
    length = 4;
  }
  if (start + length > cut) {
    cut = start;
  }
  return s.substr(0, cut);
}

CivilTime::Minutes parseDateTime(const std::string& value, bool& isDate) {
  isDate = false;
  if (value.size() < 8) {
    return -1;
  }
  const int y = readDigits(value, 0, 4);
  const int mo = readDigits(value, 4, 2);
  const int d = readDigits(value, 6, 2);
  if (y < 0 || mo < 1 || mo > 12 || d < 1 || d > 31) {
    return -1;
  }
  if (value.size() == 8) {
    isDate = true;
    return CivilTime::make(y, mo, d, 0, 0);
  }
  if (value.size() < 15 || value[8] != 'T') {
    return -1;
  }
  const int h = readDigits(value, 9, 2);
  const int mi = readDigits(value, 11, 2);
  const int sec = readDigits(value, 13, 2);
  if (h < 0 || mi < 0 || sec < 0) {
    return -1;
  }

  // Con 'Z' la hora está en UTC y hay que pasarla a la hora local de la placa.
  // Sin 'Z' (hora con TZID u hora "flotante") se toma como hora local tal cual.
  if (value.size() >= 16 && value[15] == 'Z') {
    const time_t utc = (time_t)(CivilTime::daysFromCivil(y, mo, d) * 86400 + h * 3600 + mi * 60 + sec);
    return CivilTime::fromUtc(utc);
  }
  return CivilTime::make(y, mo, d, h, mi);
}

}  // namespace Ics

// ---------------------------------------------------------------------------
// Lector
// ---------------------------------------------------------------------------

IcsParser::IcsParser(Callback onEvent, void* context) : onEvent_(onEvent), context_(context) {}

void IcsParser::feed(char c) {
  if (lineEnded_) {
    lineEnded_ = false;
    if (c == ' ' || c == '\t') {
      return;  // la línea anterior continúa: se quita el espacio y se sigue añadiendo
    }
    processLine();
    line_.clear();
  }
  if (c == '\n') {
    if (!line_.empty() && line_.back() == '\r') {
      line_.pop_back();
    }
    lineEnded_ = true;
    return;
  }
  if (line_.size() < MAX_LINE) {
    line_ += c;
  }
}

void IcsParser::finish() {
  if (lineEnded_ || !line_.empty()) {
    processLine();
    line_.clear();
    lineEnded_ = false;
  }
}

void IcsParser::processLine() {
  if (line_.size() >= MAX_LINE) {
    line_ = Ics::truncateUtf8(line_, MAX_LINE);  // por si el corte dejó un carácter a medias
  }

  // NOMBRE;PARÁMETROS:VALOR. Los dos puntos dentro de comillas son parte de un parámetro.
  size_t colon = std::string::npos;
  bool quoted = false;
  for (size_t i = 0; i < line_.size(); i++) {
    if (line_[i] == '"') {
      quoted = !quoted;
    } else if (line_[i] == ':' && !quoted) {
      colon = i;
      break;
    }
  }
  if (colon == std::string::npos) {
    return;
  }
  const size_t semicolon = line_.find(';');
  const size_t nameEnd = (semicolon != std::string::npos && semicolon < colon) ? semicolon : colon;
  const std::string name = line_.substr(0, nameEnd);
  const std::string params = nameEnd < colon ? line_.substr(nameEnd + 1, colon - nameEnd - 1) : std::string();
  const std::string value = line_.substr(colon + 1);

  if (name == "BEGIN") {
    if (value == "VEVENT" && !inEvent_) {
      inEvent_ = true;
      nestedDepth_ = 0;
      event_ = IcsEvent();
      durationMinutes_ = -1;
    } else if (inEvent_) {
      nestedDepth_++;  // VALARM y similares: sus campos no son del evento
    }
    return;
  }

  if (!inEvent_) {
    return;
  }

  if (name == "END") {
    if (nestedDepth_ > 0) {
      nestedDepth_--;
    } else if (value == "VEVENT") {
      inEvent_ = false;
      eventsRead_++;
      if (event_.start >= 0) {
        if (event_.end < 0) {
          // Sin DTEND: se usa DURATION, o un día entero (todo el día) o nada (con hora).
          if (durationMinutes_ >= 0) {
            event_.end = event_.start + durationMinutes_;
          } else {
            event_.end = event_.start + (event_.allDay ? CivilTime::MINUTES_PER_DAY : 0);
          }
        }
        onEvent_(event_, context_);
      }
    }
    return;
  }

  if (nestedDepth_ == 0) {
    handleProperty(name, params, value);
  }
}

void IcsParser::handleProperty(const std::string& name, const std::string& params, const std::string& value) {
  (void)params;  // TZID se ignora: las horas con TZID se toman como hora local de la placa
  bool isDate = false;

  if (name == "SUMMARY") {
    event_.summary = Ics::unescapeText(value);
  } else if (name == "LOCATION") {
    event_.location = Ics::unescapeText(value);
  } else if (name == "DESCRIPTION") {
    event_.description = Ics::truncateUtf8(Ics::htmlToText(Ics::unescapeText(value)), MAX_NOTES);
  } else if (name == "UID") {
    event_.uid = value;
  } else if (name == "STATUS") {
    event_.cancelled = (value == "CANCELLED");
  } else if (name == "RRULE") {
    event_.rrule = value;
  } else if (name == "DTSTART") {
    event_.start = Ics::parseDateTime(value, isDate);
    event_.allDay = isDate;
  } else if (name == "DTEND") {
    event_.end = Ics::parseDateTime(value, isDate);
  } else if (name == "DURATION") {
    durationMinutes_ = parseDuration(value);
  } else if (name == "RECURRENCE-ID") {
    event_.recurrenceId = Ics::parseDateTime(value, isDate);
  } else if (name == "EXDATE") {
    // Puede llevar varias fechas separadas por comas.
    size_t start = 0;
    while (start <= value.size()) {
      size_t comma = value.find(',', start);
      if (comma == std::string::npos) {
        comma = value.size();
      }
      const CivilTime::Minutes t = Ics::parseDateTime(value.substr(start, comma - start), isDate);
      if (t >= 0) {
        event_.exdates.push_back(t);
      }
      start = comma + 1;
    }
  }
}
