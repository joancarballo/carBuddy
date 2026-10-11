#pragma once

#include <vector>

#include "appmanager/App.h"
#include "services/calendar/CalendarEvent.h"

// Calendario: eventos de hoy y mañana descargados de Google (ver CalendarService).
//
// Pantalla: cabecera con día, fecha y hora, y una lista de 5 filas con flechas
// cuando hay más eventos arriba o abajo. Los eventos que ya han terminado salen
// tachados.
//
// Segundo nivel: el primer giro del dial selecciona un evento (fondo encendido),
// los siguientes se mueven por la lista; pulsar el dial abre el detalle del
// evento y BACK (o 30 s sin tocar nada) vuelve a la lista.
//
// El código está repartido en dos archivos:
//   CalendarApp.cpp     lista y controles
//   CalendarDetail.cpp  detalle de un evento
// Su sección de la web ("Calendario") está en web/CalendarSection.cpp.
class CalendarApp : public App {
 public:
  const char* name() const override { return "Calendario"; }
  void draw() override;
  bool hasInteraction() const override { return true; }
  void onEnter() override;
  void onExit() override;
  bool handleInput(InputEvent ev) override;

 private:
  // Tras 30 s sin tocar nada, el detalle se cierra y la selección se quita.
  static const uint32_t IDLE_TIMEOUT_MS = 30000;

  // Event: un evento. Empty: "Hoy no hay eventos" o "Sin eventos" (se puede
  // seleccionar). Separator: la línea "Mañana" (no se puede seleccionar).
  enum class RowType { Event, Empty, Separator };

  struct Row {
    RowType type;
    int eventIndex;  // posición en events_ (solo para RowType::Event)
    String text;     // texto del separador o de la fila "sin eventos"
  };

  // Lista
  void refreshData(int64_t today);
  void buildRows(int64_t today);
  int firstUpcomingRow(CivilTime::Minutes now) const;
  bool isSelectable(int row) const;
  void moveSelection(int step);
  void keepSelectionVisible();
  void drawHeader(const struct tm& t);
  void drawRow(int row, int topY, CivilTime::Minutes now);
  void drawArrows(int topY, int bottomY, bool up, bool down);

  // Detalle (CalendarDetail.cpp)
  void openDetail(const CalendarEvent& ev, int64_t today);
  void drawDetail();
  void scrollDetail(int step);

  std::vector<CalendarEvent> events_;
  uint32_t dataVersion_ = 0xFFFFFFFF;  // versión de la lista que se tiene copiada
  int64_t rowsDay_ = -1;               // día para el que se construyeron las filas
  bool laiaToday_ = false;             // hoy hay un evento "Laia en Joan" (👧 en la cabecera)

  std::vector<Row> rows_;
  int top_ = 0;        // primera fila visible
  int selected_ = -1;  // fila seleccionada, -1 si ninguna
  bool inside_ = false;

  bool detailOpen_ = false;
  std::vector<String> detailLines_;
  int titleLines_ = 0;  // las primeras líneas del detalle son el título
  int detailTop_ = 0;
  uint32_t lastInputMs_ = 0;
};
