#include "../ecow/ecow.hpp"

using namespace ecow;

int main(int argc, char **argv) {
  auto m = unit::create<mod>("missingno");

  auto all = unit::create<seq>("all");
  all->add_ref(m);
  all->add_unit<>("example");
  return run_main(all, argc, argv);
}
