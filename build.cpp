#include "build.hpp"

using namespace ecow;

int main(int argc, char **argv) {
  auto m = missingno();

  auto all = unit::create<seq>("all");
  all->add_ref(m);
  all->add_unit<>("example");
  return run_main(all, argc, argv);
}
