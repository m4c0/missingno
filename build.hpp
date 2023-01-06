#include "../ecow/ecow.hpp"

inline auto missingno() {
  using namespace ecow;
  auto m = unit::create<mod>("missingno");
  m->add_part("value");
  m->add_part("req");
  return m;
}
