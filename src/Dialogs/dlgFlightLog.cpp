/*
 Copyright_License {

 XCSoar Glide Computer - http://www.xcsoar.org/
 Copyright (C) 2000-2016 The XCSoar Project
 A detailed list of copyright holders can be found in the file "AUTHORS".

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 }
 */

#include "Dialogs/Dialogs.h"
#include "Dialogs/WidgetDialog.hpp"
#include "Widget/CreateWindowWidget.hpp"
#include "Widget/ArrowPagerWidget.hpp"
#include "Look/FontDescription.hpp"
#include "Look/DialogLook.hpp"
#include "Look/Colors.hpp"
#include "Screen/Canvas.hpp"
#include "Screen/Layout.hpp"
#include "Screen/Font.hpp"
#include "Util/ConvertString.hpp"
#include "UIGlobals.hpp"
#include "Language/Language.hpp"
#include "Renderer/FlightListRenderer.hpp"
#include "FlightInfo.hpp"
#include "Logger/FlightParser.hpp"
#include "IO/FileLineReader.hpp"
#include "LocalPath.hpp"
#include "LogFile.hpp"


#define ENTRIES_PER_PAGE 20
#define MAX_PAGES 32-1 // Limitation of the ArrowPagerWidget

static int page_count = 0;
static int page_current = 0;
static AllocatedArray<FlightInfo> flight_infos;

class FlightLogWindow final : public PaintWindow
{
protected:
  /** from class PaintWindow */
  virtual void
  OnPaint(Canvas &canvas) override;

private:
  int page = 0;

public:
  void SetPageNumber(int);
};

void
FlightLogWindow::OnPaint(Canvas &canvas){

  const DialogLook &look = UIGlobals::GetDialogLook();
  PixelRect rc = canvas.GetRect();
  canvas.ClearWhite();

  TCHAR tempPage[128];
  StringFormat(tempPage, sizeof(tempPage), _T("Page: %i/%i "),page+1,page_count+1);

  int x = rc.left + Layout::FastScale(10);
  int y = rc.top + Layout::FastScale(10);

  Font font;
  font.Load(FontDescription(Layout::FontScale(10)));
  canvas.Select(font);
  canvas.SetBackgroundTransparent();
  canvas.SetTextColor(COLOR_BLACK);
  canvas.DrawText(x, y, tempPage);

  rc.Grow(-16);
  FlightListRenderer renderer(look.bold_font, look.small_font);
  int i = page*ENTRIES_PER_PAGE;
  while (i<(page*ENTRIES_PER_PAGE+ENTRIES_PER_PAGE)) {
    renderer.AddFlight(flight_infos[i]);
    i++;
  }
  renderer.Draw(canvas, rc);
}

void FlightLogWindow::SetPageNumber(int _page){
  page = _page;
}

static void InitFlightInfo(void)
try
{
  FileLineReaderA file(LocalPath(_T("flights.log")));
  FlightParser parserTemp(file);
  FlightInfo flight;
  int i=0;

  flight_infos.GrowPreserve(ENTRIES_PER_PAGE, 0);
  while (parserTemp.Read(flight)) {
    i++;
    if (i >= (page_count+1)*ENTRIES_PER_PAGE) {
      page_count++;
      if (page_count > MAX_PAGES) {
        page_count--;
        break;
      }
      flight_infos.GrowPreserve((page_count+1)*ENTRIES_PER_PAGE, (page_count)*ENTRIES_PER_PAGE);
    }
    flight_infos[i] = flight;
  }
  LogFormat(_T("Number of flights: %i"), i);
}
catch (const std::runtime_error &e) {
  LogFormat("Flight Log Read Error");
}
static Window *
CreateFlightLogPage(ContainerWindow &parent, const PixelRect &rc,
                    WindowStyle style)
{
  FlightLogWindow *window = new FlightLogWindow();
  window->Create(parent, rc, style);
  window->SetPageNumber(page_current);
  page_current++;
  return window;
}

void
dlgFlightLogShowModal(SingleWindow &parent)
{
  const DialogLook &look = UIGlobals::GetDialogLook();
  WidgetDialog dialog(look);

  //Reset Data
  flight_infos.SetSize(0);
  page_count = 0;
  page_current = 0;

  InitFlightInfo();

  ArrowPagerWidget pager(dialog, look.button);
  for (int i=0; i <= page_count; i++) {
    pager.Add(new CreateWindowWidget(CreateFlightLogPage));
  }

  dialog.CreateFull(UIGlobals::GetMainWindow(), _("Flight Log"), &pager);
  dialog.ShowModal();
  dialog.StealWidget();
}
