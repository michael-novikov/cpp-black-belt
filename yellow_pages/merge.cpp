#include "yellow_pages.h"

#include <algorithm>
#include <tuple>

namespace YellowPages {

  Company Merge(const Signals& signals, const Providers&) {
    // Наивная неправильная реализация: берём все данные из самого первого сигнала, никак не учитываем приоритеты
    return signals.empty() ? Company() : signals[0].company();
  }

}
