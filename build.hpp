#include "../ecow/ecow.hpp"

inline auto missingno() {
  using namespace ecow;
  return unit::create<mod>("missingno");
}
