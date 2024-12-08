
#include "mir/MIR.hpp"
#include "mir/utils.hpp"
#include "target/riscv/RISCVTarget.hpp"
#include "autogen/riscv/InstInfoDecl.hpp"
#include "autogen/riscv/ISelInfoDecl.hpp"
#include "support/StaticReflection.hpp"

namespace mir {
/**
 * RISCVTarget::postLegalizeFunc: fix pcrel addressing
 */
void RISCVTarget::postLegalizeFunc(MIRFunction& func, CodeGenContext& ctx) {
  constexpr bool Debug = false;

  auto dumpInst = [&](MIRInst* inst) {
    auto& instInfo = ctx.instInfo.getInstInfo(inst);
    instInfo.print(std::cerr << "rvPostLegalizeFunc: ", *inst, false);
    std::cerr << std::endl;
  };
  if (Debug)
    func.print(std::cerr, ctx);
  /* fix pcrel addressing */
  for (auto blockIter = func.blocks().begin(); blockIter != func.blocks().end();) {
    if (Debug)
      std::cerr << "block: " << blockIter->get()->name() << std::endl;
    auto nextIter = std::next(blockIter);

    /* origin reloc -> (dst -> block)*/
    std::unordered_map<MIRRelocable*, std::unordered_map<MIROperand, MIRBlock*, MIROperandHasher>>
      auipcMap;
    while (true) {
      auto& insts = blockIter->get()->insts();
      if (insts.empty()) {
        break;
      }
      bool isNewBlock = false;
      for (auto instIter = insts.begin(); instIter != insts.end(); instIter++) {
        auto& inst = *instIter;
        if (Debug) {
          dumpInst(inst);
        }
        if (inst->opcode() == RISCV::AUIPC) {
          /* AUIPC dst, imm */
          if (instIter == insts.begin() && blockIter != func.blocks().begin()) {
            if (Debug)
              std::cerr << "first in block" << std::endl;
            assert(inst->operand(1).type() == OperandType::HighBits);
            /** first inst in block, block label lowBits is just
             * inst's dst lowBits */
            // auipcMap[inst->operand(1)][inst->operand(0)] =
            //     getLowBits(MIROperand::as_reloc(blockIter->get()));
            // auto t = blockIter->get();
            auipcMap[inst->operand(1).reloc()][inst->operand(0)] = blockIter->get();
          } else {
            if (Debug)
              std::cerr << "not first in block" << std::endl;
            /** other insts */
            auto newBlock =
              std::make_unique<MIRBlock>(&func, "pcrel" + std::to_string(ctx.nextLabelId()));
            auto& newInsts = newBlock->insts();
            newInsts.splice(newInsts.begin(), insts, instIter, insts.end());
            blockIter = func.blocks().insert(nextIter, std::move(newBlock));
            isNewBlock = true;
            break; /* break instIter for insts */
          }
        } else {
          /* not AUIPC */
          auto& instInfo = ctx.instInfo.getInstInfo(inst);

          // instInfo.print(std::cerr << "!!", *inst,  false);
          for (uint32_t idx = 0; idx < instInfo.operand_num(); idx++) {
            auto operand = inst->operand(idx);

            auto getBase = [&] {
              switch (inst->opcode()) {
                case RISCV::ADDI: {
                  /* ADDI dst, src, imm */
                  return inst->operand(1);
                }
                default: {
                  /* must be load or store */
                  assert(requireOneFlag(instInfo.inst_flag(), InstFlagLoad | InstFlagStore));
                  /* load dst, imm(src)
                   * store src2, imm(src1) */
                  return inst->operand(2);
                }
              }
            };

            if (operand.isReloc() && operand.type() == OperandType::LowBits) {
              auto pcrelBlock = auipcMap.at(operand.reloc()).at(getBase());
              auto op = getLowBits(MIROperand::asReloc(pcrelBlock));
              inst->set_operand(idx, op);
              if (Debug) {
                instInfo.print(std::cerr << "fix pcrel: ", *inst, false);
              }
            }
          }
        }
      }
      if (not isNewBlock)
        break;
    }  // while

    blockIter = nextIter;
  }  // blockIter
  if (Debug)
    func.print(std::cerr, ctx);
}  // postLegalizeFunc

void RISCVTarget::emit_assembly(std::ostream& out, MIRModule& module) {
  auto& target = *this;
  // out
  //   << R"(.attribute arch, "rv64i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0_zifencei2p0_zba1p0_zbb1p0")"
  //   << '\n';
  out << SysYRuntime << std::endl;
  CodeGenContext codegen_ctx{target, target.getDataLayout(), target.getTargetInstInfo(),
                             target.getTargetFrameInfo(), MIRFlags{false, false}};
  dumpAssembly(out, module, codegen_ctx);
}

bool RISCVTarget::verify(MIRModule& module) {
  for (auto& func : module.functions()) {
    if (not verify(*func)) {
      return false;
    }
  }
  return true;
}

bool RISCVTarget::verify(MIRFunction& func) {
  // std::cerr << "verify function: " << func.name() << std::endl;
  for (auto& block : func.blocks()) {
    // std::cerr << "block: " << block->name() << std::endl;
    for (auto& inst : block->insts()) {
      const auto opcode = inst->opcode();
      // std::cerr << "opcode: " << opcode << std::endl;
      // std::cerr << "verify: "
      //           << utils::enumName(static_cast<RISCV::RISCVInst>(opcode))
      //           << std::endl;
      if (not(opcode >= RISCV::RISCVInst::RISCVInstBegin and
              opcode <= RISCV::RISCVInst::RISCVInstEnd)) {
        std::cerr << "unknown riscv instruction: "
                  << utils::enumName(static_cast<MIRGenericInst>(opcode)) << std::endl;
        // assert(false);
        return false;
      }
    }
  }
  return true;
}

}  // namespace mir