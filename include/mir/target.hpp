
#pragma once
#include "mir/MIR.hpp"
#include "mir/datalayout.hpp"
#include "mir/iselinfo.hpp"
#include "mir/instinfo.hpp"
#include "mir/frameinfo.hpp"
#include "mir/registerinfo.hpp"
#include "mir/ScheduleModel.hpp"
namespace mir {
class TargetFrameInfo;
/*
 * @brief: Target Class (抽象基类)
 * @note:
 *      存储目标架构 (RISC-V OR ARM)相关信息
 */
class Target {
public:
  virtual ~Target() = default;

public:  // get function
  virtual DataLayout& getDataLayout() = 0;
  virtual TargetScheduleModel& getScheduleModel() = 0;
  virtual TargetInstInfo& getTargetInstInfo() = 0;
  virtual TargetISelInfo& getTargetIselInfo() = 0;
  virtual TargetFrameInfo& getTargetFrameInfo() = 0;
  virtual TargetRegisterInfo& getRegisterInfo() = 0;

public:  // assembly
  virtual void postLegalizeFunc(MIRFunction& func, CodeGenContext& ctx) {}
  virtual void emit_assembly(std::ostream& out, MIRModule& module) = 0;

  virtual bool verify(MIRModule& module) = 0;
  virtual bool verify(MIRFunction& func) = 0;
};

/* MIRFlags */
struct MIRFlags final {
  bool endsWithTerminator = true;
  bool inSSAForm = false;
  bool preRA = true;
  bool postSA = false;
  bool dontForward = false;
  bool postLegal = false;
};

/* CodeGenContext */
struct CodeGenContext final {
  Target& target;
  DataLayout& dataLayout;
  TargetInstInfo& instInfo;
  TargetFrameInfo& frameInfo;

  MIRFlags flags;

  TargetISelInfo* iselInfo;
  TargetRegisterInfo* registerInfo;

  TargetScheduleModel* scheduleModel;

  uint32_t idx = 0;
  auto nextId() { return ++idx; }

  uint32_t label_idx = 0;
  auto nextLabelId() { return label_idx++; }
};

// using TargetBuilder = std::pair<std::string_view, std::function<Targe*()> >;
// class TargetRegistry {
//     std::vector<TargetBuilder> _targets;
// public:
//     void add_target( TargetBuilder& target_builder);
//     Target* select_target()
// };

}  // namespace mir
