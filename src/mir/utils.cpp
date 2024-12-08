#include "mir/utils.hpp"

#include <string_view>

namespace mir {
enum class DataSection {
  Data,
  RoData,
  Bss,
};

static const std::unordered_map<DataSection, std::string_view>
  DataSectionNames = {
    {DataSection::Data, ".data"},
    {DataSection::RoData, ".rodata"},
    {DataSection::Bss, ".bss"},
};

inline int32_t ilog2(size_t x) {
  return __builtin_ctzll(x);
}

void dumpAssembly(std::ostream& os, MIRModule& module, CodeGenContext& ctx) {
  /* 1: data section */
  auto selectDataSection = [](const MIRRelocable* reloc) {
    if (auto data = reloc->dynCast<MIRDataStorage>()) {
      return data->is_readonly() ? DataSection::RoData : DataSection::Data;
    } else if (auto zero = reloc->dynCast<MIRZeroStorage>()) {
      return DataSection::Bss;
    } else {
      assert(false && "Unsupported data section");
    }
  };

  std::unordered_map<DataSection, std::vector<MIRGlobalObject*>> dataSections;

  for (auto& gobj : module.global_objs()) {
    dataSections[selectDataSection(gobj->reloc.get())].emplace_back(gobj.get());
  }

  for (auto ds : {DataSection::Data, DataSection::RoData, DataSection::Bss}) {
    if (dataSections[ds].empty()) continue;
    os << ".section " << DataSectionNames.at(ds) << "\n";
    for (auto gobj : dataSections[ds]) {
      os << ".globl " << gobj->reloc->name() << "\n";
      os << ".p2align " << ilog2(gobj->align) << std::endl;
      os << gobj->reloc->name() << ":\n";
      gobj->reloc->print(os, ctx);
      os << "\n";
    }
  }

  /* 2: text section */
  os << ".section .text\n";
  for (auto& func : module.functions()) {
    if (func->blocks().empty()) continue;
    os << ".globl " << func->name() << "\n";
    for (auto& bb : func->blocks()) {
      if (bb == func->blocks().front()) {
        os << func->name() << ":\n";
        /* dump stack usage comment */
        size_t argument = 0, calleeArgument = 0, loacl = 0, reSpill = 0, calleeSaved = 0;
        for (auto& [operand, stackobj] : func->stackObjs()) {
          switch (stackobj.usage) {
            case StackObjectUsage::Argument:
              argument += stackobj.size;
              break;
            case StackObjectUsage::CalleeArgument:
              calleeArgument += stackobj.size;
              break;
            case StackObjectUsage::Local:
              loacl += stackobj.size;
              break;
            case StackObjectUsage::RegSpill:
              reSpill += stackobj.size;
              break;
            case StackObjectUsage::CalleeSaved:
              calleeSaved += stackobj.size;
              break;
          }
        }
        os << "\t" << "# stack usage: \n";
        os << "\t# Argument=" << argument << ", \n";
        os << "\t# CalleeArgument=" << calleeArgument << ", ";
        os << "Local=" << loacl << ", \n";
        os << "\t# RegSpill=" << reSpill << ", ";
        os << "CalleeSaved=" << calleeSaved << "\n";
      } else {
        os << bb->name() << ":\n";
      }
      for (auto& inst : bb->insts()) {
        auto& info = ctx.instInfo.getInstInfo(inst);
        info.print(os << "\t", *inst, false);
        os << std::endl;
      }
    }
  }
}

void forEachDefOperand(MIRBlock& block,
                       CodeGenContext& ctx,
                       const std::function<void(MIROperand op)>& functor) {
  for (auto& inst : block.insts()) {
    auto& inst_info = ctx.instInfo.getInstInfo(inst);
    for (uint32_t idx = 0; idx < inst_info.operand_num(); idx++) {
      if (inst_info.operand_flag(idx) & OperandFlagDef) {
        functor(inst->operand(idx));
      }
    }
  }
}

void forEachDefOperand(MIRFunction& func,
                       CodeGenContext& ctx,
                       const std::function<void(MIROperand op)>& functor) {
  for (auto& block : func.blocks()) {
    forEachDefOperand(*block, ctx, functor);
  }
}
}  // namespace mir