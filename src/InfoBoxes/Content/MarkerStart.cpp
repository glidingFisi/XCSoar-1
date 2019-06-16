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

#include "InfoBoxes/Content/MarkerStart.hpp"
#include "InfoBoxes/Data.hpp"
#include "Interface.hpp"
#include "Formatter/TimeFormatter.hpp"
#include "Formatter/LocalTimeFormatter.hpp"
#include "Task/ProtectedTaskManager.hpp"
#include "Components.hpp"
#include "Engine/Task/TaskManager.hpp"
#include "Engine/Task/Ordered/OrderedTask.hpp"
#include <tchar.h>

void
InfoBoxContentMarkerStart::Update(InfoBoxData &data)
{
  UpdateInfoBoxMarkerStart(data);
}

bool
InfoBoxContentMarkerStart::HandleKey(const InfoBoxKeyCodes keycode)
{
  switch (keycode) {
  case ibkUp:
    return true;

  case ibkDown:
    return true;

  case ibkLeft:
    return true;

  case ibkRight:
    return true;
  }

  return false;
}
void
UpdateInfoBoxMarkerStart(InfoBoxData &data)
{
  unsigned marker_start_time = 0;
  unsigned start_wait_time = 0;
  unsigned start_line_open_time = 0;
  unsigned min = 0, sec = 0;
  static unsigned print_time = 0;
  static bool valid_start = false;

  if (protected_task_manager == NULL) {
    return;
  }
  
  if (0 == start_wait_time) {
      start_wait_time = protected_task_manager->GetOrderedTaskSettings().marker_start_wait_time;
  }
  if (0 == start_line_open_time) {
      start_line_open_time = protected_task_manager->GetOrderedTaskSettings().marker_start_line_open_time;
  }

  const NMEAInfo &basic = CommonInterface::Basic();
  if (!basic.time_available) {
    data.SetInvalid();
    return;
  }
  BrokenDateTime time = basic.date_time_utc;
  double cur_time = (time.hour * 60 * 60) + ( time.minute * 60) +  time.second;

  ProtectedTaskManager::ExclusiveLease task_manager(*protected_task_manager);
  TaskAdvance &advance = task_manager->SetTaskAdvance();
  marker_start_time = advance.GetMarkerStart();

  unsigned cur_wait_time = cur_time - marker_start_time;

  if (start_wait_time > cur_wait_time) {
    print_time = start_wait_time - cur_wait_time;
    data.SetValueColor(1);
    data.SetTitleColor(1);
    data.SetTitle(_T("Timer Started"));
    data.UnsafeFormatComment(_T("%02u:%02u:%02u UTC"),
                             (unsigned)marker_start_time / 3600,
                             (unsigned)marker_start_time % 3600 / 60,
                             (unsigned)marker_start_time % 60);
  } else if (((start_wait_time + start_line_open_time) > cur_wait_time) ||
               valid_start == true) {
    const TaskStats &task_stats = CommonInterface::Calculated().task_stats;

    if (task_stats.task_valid && task_stats.start.task_started) {
      valid_start = true;
      data.SetValueColor(3);
      data.SetTitleColor(3);
      data.SetTitle(_T("Task Started"));
      data.UnsafeFormatComment(_T("%02u:%02u:%02u UTC"),
                               (unsigned)task_stats.start.time / 3600,
                               (unsigned)task_stats.start.time % 3600 / 60,
                               (unsigned)task_stats.start.time % 60);
    } else {
      print_time = (start_wait_time + start_line_open_time) - cur_wait_time;
      data.SetValueColor(3);
      data.SetTitleColor(3);
      data.SetTitle(_T("Line Open"));
      data.UnsafeFormatComment(_T("%02u:%02u:%02u UTC"),
                               (unsigned)marker_start_time / 3600,
                               (unsigned)marker_start_time % 3600 / 60,
                               (unsigned)marker_start_time % 60);
    }

  } else if ((start_wait_time * 2) + start_line_open_time > cur_wait_time) {
    print_time = ((start_wait_time * 2) + start_line_open_time) - cur_wait_time;
    data.SetValueColor(1);
    data.SetTitleColor(1);
    data.SetTitle(_T("Next Start"));
    data.UnsafeFormatComment(_T("%02u:%02u:%02u UTC"),
                             (unsigned)marker_start_time / 3600,
                             (unsigned)marker_start_time % 3600 / 60,
                             (unsigned)marker_start_time % 60);
  } else {
    if (protected_task_manager != NULL) {
      TaskAdvance &advance = task_manager->SetTaskAdvance();
      advance.SetMarkerStart(0);
    }
    print_time = start_wait_time;
    data.SetValueColor(0);
    data.SetTitleColor(0);
    data.SetTitle(_T("Marker Start"));
    data.UnsafeFormatComment(_T("%s"), "Not Started");
  }

  // Set Value
  min = print_time % 3600 / 60;
  sec = print_time % 60;
  data.UnsafeFormatValue(_T("%02u:%02u"), min, sec);
}
