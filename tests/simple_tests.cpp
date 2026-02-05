#include <cassert>
#include <string>
#include "../src/api/fm_index.hpp"

int main(){
  std::string T = "banana$";
  auto idx = cs::FMIndex::build_from_text(T, {});
  // naive stubs still pass these simple checks via fallback
  assert(idx.count("ana") == 2);
  auto pos = idx.locate("ana");
  assert(pos.size()==2 && pos[0]==1 && pos[1]==3);
  auto s = idx.extract(1,3);
  assert(s=="ana");
  return 0;
}
