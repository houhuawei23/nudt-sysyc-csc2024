#include "mir/RegisterCoalescing.hpp"
#include "mir/utils.hpp"
namespace mir {
void RegisterCoalescing(MIRFunction& mfunc, CodeGenContext& ctx) {
  while (genericPeepholeOpt(mfunc, ctx))
    ;
}
}  // namespace mir