#pragma once

#include "../ecow/ecow.hpp"
#include "../traits/build.hpp"

inline auto missingno() {
  using namespace ecow;
  auto m = unit::create<mod>("missingno");
  m->add_wsdep("traits", traits());
  m->add_part("value");
  m->add_part("opt");
  m->add_part("req");
  m->add_part("operators");
  return m;
}
