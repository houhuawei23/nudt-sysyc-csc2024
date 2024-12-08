// Automatically generated file, do not edit!

#include "mir/MIR.hpp"
#include "mir/instinfo.hpp"
#include "autogen/generic/InstInfoDecl.hpp"

GENERIC_NAMESPACE_BEGIN

class GENERICInstInfoJump final : public InstInfo {
public:
  GENERICInstInfoJump() = default;

  uint32_t operand_num() const override { return 1; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagMetadata;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override {
    return InstFlagNone | InstFlagTerminator | InstFlagBranch | InstFlagNoFallThrough;
  }

  std::string_view name() const override { return "GENERIC.Jump"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Jump" << " " << mir::GENERIC::OperandDumper{inst.operand(0)};
  }
};

class GENERICInstInfoBranch final : public InstInfo {
public:
  GENERICInstInfoBranch() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagUse;
      case 1:
        return OperandFlagMetadata;
      case 2:
        return OperandFlagMetadata;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagTerminator | InstFlagBranch; }

  std::string_view name() const override { return "GENERIC.Branch"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Branch" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoUnreachable final : public InstInfo {
public:
  GENERICInstInfoUnreachable() = default;

  uint32_t operand_num() const override { return 0; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override {
    return InstFlagNone | InstFlagTerminator | InstFlagNoFallThrough;
  }

  std::string_view name() const override { return "GENERIC.Unreachable"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Unreachable";
  }
};

class GENERICInstInfoLoad final : public InstInfo {
public:
  GENERICInstInfoLoad() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagMetadata;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagLoad; }

  std::string_view name() const override { return "GENERIC.Load"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Load" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoStore final : public InstInfo {
public:
  GENERICInstInfoStore() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagUse;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagMetadata;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagStore; }

  std::string_view name() const override { return "GENERIC.Store"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Store" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoAdd final : public InstInfo {
public:
  GENERICInstInfoAdd() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagCommutative; }

  std::string_view name() const override { return "GENERIC.Add"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Add" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoSub final : public InstInfo {
public:
  GENERICInstInfoSub() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.Sub"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Sub" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoMul final : public InstInfo {
public:
  GENERICInstInfoMul() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagCommutative; }

  std::string_view name() const override { return "GENERIC.Mul"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Mul" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoUDiv final : public InstInfo {
public:
  GENERICInstInfoUDiv() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.UDiv"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "UDiv" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoURem final : public InstInfo {
public:
  GENERICInstInfoURem() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.URem"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "URem" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoAnd final : public InstInfo {
public:
  GENERICInstInfoAnd() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagCommutative; }

  std::string_view name() const override { return "GENERIC.And"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "And" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoOr final : public InstInfo {
public:
  GENERICInstInfoOr() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagCommutative; }

  std::string_view name() const override { return "GENERIC.Or"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Or" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoXor final : public InstInfo {
public:
  GENERICInstInfoXor() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagCommutative; }

  std::string_view name() const override { return "GENERIC.Xor"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Xor" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoShl final : public InstInfo {
public:
  GENERICInstInfoShl() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.Shl"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Shl" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoLShr final : public InstInfo {
public:
  GENERICInstInfoLShr() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.LShr"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "LShr" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoAShr final : public InstInfo {
public:
  GENERICInstInfoAShr() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.AShr"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "AShr" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoSDiv final : public InstInfo {
public:
  GENERICInstInfoSDiv() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.SDiv"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "SDiv" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoSRem final : public InstInfo {
public:
  GENERICInstInfoSRem() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.SRem"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "SRem" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoSMin final : public InstInfo {
public:
  GENERICInstInfoSMin() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagCommutative; }

  std::string_view name() const override { return "GENERIC.SMin"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "SMin" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoSMax final : public InstInfo {
public:
  GENERICInstInfoSMax() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagCommutative; }

  std::string_view name() const override { return "GENERIC.SMax"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "SMax" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoNeg final : public InstInfo {
public:
  GENERICInstInfoNeg() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.Neg"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Neg" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoAbs final : public InstInfo {
public:
  GENERICInstInfoAbs() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.Abs"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Abs" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoFAdd final : public InstInfo {
public:
  GENERICInstInfoFAdd() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagCommutative; }

  std::string_view name() const override { return "GENERIC.FAdd"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "FAdd" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoFSub final : public InstInfo {
public:
  GENERICInstInfoFSub() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.FSub"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "FSub" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoFMul final : public InstInfo {
public:
  GENERICInstInfoFMul() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagCommutative; }

  std::string_view name() const override { return "GENERIC.FMul"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "FMul" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoFDiv final : public InstInfo {
public:
  GENERICInstInfoFDiv() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.FDiv"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "FDiv" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfoFNeg final : public InstInfo {
public:
  GENERICInstInfoFNeg() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.FNeg"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "FNeg" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoFAbs final : public InstInfo {
public:
  GENERICInstInfoFAbs() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.FAbs"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "FAbs" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoFFma final : public InstInfo {
public:
  GENERICInstInfoFFma() = default;

  uint32_t operand_num() const override { return 4; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      case 3:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.FFma"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "FFma" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(3)};
  }
};

class GENERICInstInfoICmp final : public InstInfo {
public:
  GENERICInstInfoICmp() = default;

  uint32_t operand_num() const override { return 4; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      case 3:
        return OperandFlagMetadata;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.ICmp"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "ICmp" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(3)};
  }
};

class GENERICInstInfoFCmp final : public InstInfo {
public:
  GENERICInstInfoFCmp() = default;

  uint32_t operand_num() const override { return 4; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      case 3:
        return OperandFlagMetadata;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.FCmp"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "FCmp" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(3)};
  }
};

class GENERICInstInfoSExt final : public InstInfo {
public:
  GENERICInstInfoSExt() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.SExt"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "SExt" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoZExt final : public InstInfo {
public:
  GENERICInstInfoZExt() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.ZExt"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "ZExt" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoTrunc final : public InstInfo {
public:
  GENERICInstInfoTrunc() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.Trunc"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Trunc" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoF2U final : public InstInfo {
public:
  GENERICInstInfoF2U() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.F2U"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "F2U" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoF2S final : public InstInfo {
public:
  GENERICInstInfoF2S() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.F2S"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "F2S" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoU2F final : public InstInfo {
public:
  GENERICInstInfoU2F() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.U2F"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "U2F" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoS2F final : public InstInfo {
public:
  GENERICInstInfoS2F() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.S2F"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "S2F" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoFCast final : public InstInfo {
public:
  GENERICInstInfoFCast() = default;

  uint32_t operand_num() const override { return 1; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.FCast"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "FCast" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoCopy final : public InstInfo {
public:
  GENERICInstInfoCopy() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagRegCopy; }

  std::string_view name() const override { return "GENERIC.Copy"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Copy" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoSelect final : public InstInfo {
public:
  GENERICInstInfoSelect() = default;

  uint32_t operand_num() const override { return 4; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      case 3:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.Select"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "Select" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(3)};
  }
};

class GENERICInstInfoLoadGlobalAddress final : public InstInfo {
public:
  GENERICInstInfoLoadGlobalAddress() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagMetadata;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagLoadConstant; }

  std::string_view name() const override { return "GENERIC.LoadGlobalAddress"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "LoadGlobalAddress" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoLoadImm final : public InstInfo {
public:
  GENERICInstInfoLoadImm() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagMetadata;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.LoadImm"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "LoadImm" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoLoadStackObjectAddr final : public InstInfo {
public:
  GENERICInstInfoLoadStackObjectAddr() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagMetadata;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagLoadConstant; }

  std::string_view name() const override { return "GENERIC.LoadStackObjectAddr"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "LoadStackObjectAddr" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoCopyFromReg final : public InstInfo {
public:
  GENERICInstInfoCopyFromReg() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagRegCopy; }

  std::string_view name() const override { return "GENERIC.CopyFromReg"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "CopyFromReg" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoCopyToReg final : public InstInfo {
public:
  GENERICInstInfoCopyToReg() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagRegCopy | InstFlagRegDef; }

  std::string_view name() const override { return "GENERIC.CopyToReg"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "CopyToReg" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoLoadImmToReg final : public InstInfo {
public:
  GENERICInstInfoLoadImmToReg() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagMetadata;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagRegDef; }

  std::string_view name() const override { return "GENERIC.LoadImmToReg"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "LoadImmToReg" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoLoadRegFromStack final : public InstInfo {
public:
  GENERICInstInfoLoadRegFromStack() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagMetadata;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagLoad; }

  std::string_view name() const override { return "GENERIC.LoadRegFromStack"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "LoadRegFromStack" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoStoreRegToStack final : public InstInfo {
public:
  GENERICInstInfoStoreRegToStack() = default;

  uint32_t operand_num() const override { return 2; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagMetadata;
      case 1:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone | InstFlagStore; }

  std::string_view name() const override { return "GENERIC.StoreRegToStack"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "StoreRegToStack" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)};
  }
};

class GENERICInstInfoReturn final : public InstInfo {
public:
  GENERICInstInfoReturn() = default;

  uint32_t operand_num() const override { return 0; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override { return InstFlagNone; }

  std::string_view name() const override { return "GENERIC.Return"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override { out << "Return"; }
};

class GENERICInstInfoAtomicAdd final : public InstInfo {
public:
  GENERICInstInfoAtomicAdd() = default;

  uint32_t operand_num() const override { return 3; }

  OperandFlag operand_flag(uint32_t idx) const override {
    switch (idx) {
      case 0:
        return OperandFlagDef;
      case 1:
        return OperandFlagUse;
      case 2:
        return OperandFlagUse;
      default:
        return OperandFlagNone;
        assert(false && "Invalid operand index");
    }
  }

  uint32_t inst_flag() const override {
    return InstFlagNone | InstFlagAtomic | InstFlagLoad | InstFlagStore;
  }

  std::string_view name() const override { return "GENERIC.AtomicAdd"; }

  void print(std::ostream& out, MIRInst& inst, bool comment) const override {
    out << "AtomicAdd" << " " << mir::GENERIC::OperandDumper{inst.operand(0)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(1)} << ", "
        << mir::GENERIC::OperandDumper{inst.operand(2)};
  }
};

class GENERICInstInfo final : public TargetInstInfo {
  GENERICInstInfoJump _instinfoJump;
  GENERICInstInfoBranch _instinfoBranch;
  GENERICInstInfoUnreachable _instinfoUnreachable;
  GENERICInstInfoLoad _instinfoLoad;
  GENERICInstInfoStore _instinfoStore;
  GENERICInstInfoAdd _instinfoAdd;
  GENERICInstInfoSub _instinfoSub;
  GENERICInstInfoMul _instinfoMul;
  GENERICInstInfoUDiv _instinfoUDiv;
  GENERICInstInfoURem _instinfoURem;
  GENERICInstInfoAnd _instinfoAnd;
  GENERICInstInfoOr _instinfoOr;
  GENERICInstInfoXor _instinfoXor;
  GENERICInstInfoShl _instinfoShl;
  GENERICInstInfoLShr _instinfoLShr;
  GENERICInstInfoAShr _instinfoAShr;
  GENERICInstInfoSDiv _instinfoSDiv;
  GENERICInstInfoSRem _instinfoSRem;
  GENERICInstInfoSMin _instinfoSMin;
  GENERICInstInfoSMax _instinfoSMax;
  GENERICInstInfoNeg _instinfoNeg;
  GENERICInstInfoAbs _instinfoAbs;
  GENERICInstInfoFAdd _instinfoFAdd;
  GENERICInstInfoFSub _instinfoFSub;
  GENERICInstInfoFMul _instinfoFMul;
  GENERICInstInfoFDiv _instinfoFDiv;
  GENERICInstInfoFNeg _instinfoFNeg;
  GENERICInstInfoFAbs _instinfoFAbs;
  GENERICInstInfoFFma _instinfoFFma;
  GENERICInstInfoICmp _instinfoICmp;
  GENERICInstInfoFCmp _instinfoFCmp;
  GENERICInstInfoSExt _instinfoSExt;
  GENERICInstInfoZExt _instinfoZExt;
  GENERICInstInfoTrunc _instinfoTrunc;
  GENERICInstInfoF2U _instinfoF2U;
  GENERICInstInfoF2S _instinfoF2S;
  GENERICInstInfoU2F _instinfoU2F;
  GENERICInstInfoS2F _instinfoS2F;
  GENERICInstInfoFCast _instinfoFCast;
  GENERICInstInfoCopy _instinfoCopy;
  GENERICInstInfoSelect _instinfoSelect;
  GENERICInstInfoLoadGlobalAddress _instinfoLoadGlobalAddress;
  GENERICInstInfoLoadImm _instinfoLoadImm;
  GENERICInstInfoLoadStackObjectAddr _instinfoLoadStackObjectAddr;
  GENERICInstInfoCopyFromReg _instinfoCopyFromReg;
  GENERICInstInfoCopyToReg _instinfoCopyToReg;
  GENERICInstInfoLoadImmToReg _instinfoLoadImmToReg;
  GENERICInstInfoLoadRegFromStack _instinfoLoadRegFromStack;
  GENERICInstInfoStoreRegToStack _instinfoStoreRegToStack;
  GENERICInstInfoReturn _instinfoReturn;
  GENERICInstInfoAtomicAdd _instinfoAtomicAdd;

public:
  GENERICInstInfo() = default;
  const InstInfo& getInstInfo(uint32_t opcode) const override {
    switch (opcode) {
      case GENERICInst::Jump:
        return _instinfoJump;
      case GENERICInst::Branch:
        return _instinfoBranch;
      case GENERICInst::Unreachable:
        return _instinfoUnreachable;
      case GENERICInst::Load:
        return _instinfoLoad;
      case GENERICInst::Store:
        return _instinfoStore;
      case GENERICInst::Add:
        return _instinfoAdd;
      case GENERICInst::Sub:
        return _instinfoSub;
      case GENERICInst::Mul:
        return _instinfoMul;
      case GENERICInst::UDiv:
        return _instinfoUDiv;
      case GENERICInst::URem:
        return _instinfoURem;
      case GENERICInst::And:
        return _instinfoAnd;
      case GENERICInst::Or:
        return _instinfoOr;
      case GENERICInst::Xor:
        return _instinfoXor;
      case GENERICInst::Shl:
        return _instinfoShl;
      case GENERICInst::LShr:
        return _instinfoLShr;
      case GENERICInst::AShr:
        return _instinfoAShr;
      case GENERICInst::SDiv:
        return _instinfoSDiv;
      case GENERICInst::SRem:
        return _instinfoSRem;
      case GENERICInst::SMin:
        return _instinfoSMin;
      case GENERICInst::SMax:
        return _instinfoSMax;
      case GENERICInst::Neg:
        return _instinfoNeg;
      case GENERICInst::Abs:
        return _instinfoAbs;
      case GENERICInst::FAdd:
        return _instinfoFAdd;
      case GENERICInst::FSub:
        return _instinfoFSub;
      case GENERICInst::FMul:
        return _instinfoFMul;
      case GENERICInst::FDiv:
        return _instinfoFDiv;
      case GENERICInst::FNeg:
        return _instinfoFNeg;
      case GENERICInst::FAbs:
        return _instinfoFAbs;
      case GENERICInst::FFma:
        return _instinfoFFma;
      case GENERICInst::ICmp:
        return _instinfoICmp;
      case GENERICInst::FCmp:
        return _instinfoFCmp;
      case GENERICInst::SExt:
        return _instinfoSExt;
      case GENERICInst::ZExt:
        return _instinfoZExt;
      case GENERICInst::Trunc:
        return _instinfoTrunc;
      case GENERICInst::F2U:
        return _instinfoF2U;
      case GENERICInst::F2S:
        return _instinfoF2S;
      case GENERICInst::U2F:
        return _instinfoU2F;
      case GENERICInst::S2F:
        return _instinfoS2F;
      case GENERICInst::FCast:
        return _instinfoFCast;
      case GENERICInst::Copy:
        return _instinfoCopy;
      case GENERICInst::Select:
        return _instinfoSelect;
      case GENERICInst::LoadGlobalAddress:
        return _instinfoLoadGlobalAddress;
      case GENERICInst::LoadImm:
        return _instinfoLoadImm;
      case GENERICInst::LoadStackObjectAddr:
        return _instinfoLoadStackObjectAddr;
      case GENERICInst::CopyFromReg:
        return _instinfoCopyFromReg;
      case GENERICInst::CopyToReg:
        return _instinfoCopyToReg;
      case GENERICInst::LoadImmToReg:
        return _instinfoLoadImmToReg;
      case GENERICInst::LoadRegFromStack:
        return _instinfoLoadRegFromStack;
      case GENERICInst::StoreRegToStack:
        return _instinfoStoreRegToStack;
      case GENERICInst::Return:
        return _instinfoReturn;
      case GENERICInst::AtomicAdd:
        return _instinfoAtomicAdd;
      case GENERICInst::AtomicSub:
        break; /* not supported */
      default:
        return TargetInstInfo::getInstInfo(opcode);
    }
  }
  bool matchBranch(MIRInst* inst, MIRBlock*& target, double& prob) const override {
    auto& instInfo = getInstInfo(inst->opcode());
    if (requireFlag(instInfo.inst_flag(), InstFlagBranch)) {
      if (inst->opcode() < ISASpecificBegin) {
        return TargetInstInfo::matchBranch(inst, target, prob);
      }
      switch (inst->opcode()) {
        case Jump:
          target = dynamic_cast<MIRBlock*>(inst->operand(0).reloc());
          prob = 1.0;
          break;
        case Branch:
          target = dynamic_cast<MIRBlock*>(inst->operand(1).reloc());
          prob = inst->operand(2).prob();
          break;
        default:
          std::cerr << "Error: unknown branch instruction: " << instInfo.name() << std::endl;
      }
      return true;
    }
    return false;
  }

  void redirectBranch(MIRInst* inst, MIRBlock* target) const override {
    if (inst->opcode() < ISASpecificBegin) {
      return TargetInstInfo::redirectBranch(inst, target);
    }
    assert(requireFlag(getInstInfo(inst->opcode()).inst_flag(), InstFlagBranch));

    switch (inst->opcode()) {
      case Jump:
        inst->set_operand(0, MIROperand::asReloc(target));
        break;
      case Branch:
        inst->set_operand(1, MIROperand::asReloc(target));
        break;
      default:
        std::cerr << "Error: unknown branch instruction: " << getInstInfo(inst->opcode()).name()
                  << std::endl;
        assert(false);
    }
  }
};

TargetInstInfo& getGENERICInstInfo() {
  static GENERICInstInfo instance;
  return instance;
}
GENERIC_NAMESPACE_END