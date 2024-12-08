#include "ir/value.hpp"
#include "support/arena.hpp"
namespace ir {
//! Use
void Use::print(std::ostream& os) const {
  os << "use(" << mIndex << ", ";
  mUser->dumpAsOpernd(os);
  os << " -> ";
  mValue->dumpAsOpernd(os);
  os << ")";
  os << std::endl;
}

size_t Use::index() const {
  return mIndex;
}

User* Use::user() const {
  return mUser;
}

Value* Use::value() const {
  return mValue;
}

void Use::set_index(size_t index) {
  mIndex = index;
}
void Use::set_user(User* user) {
  mUser = user;
}
void Use::set_value(Value* value) {
  mValue = value;
}

//! Value

void Value::replaceAllUseWith(Value* mValue) {
  for (auto puseIter = mUses.begin(); puseIter != mUses.end();) {
    auto puse = *puseIter;
    puseIter++;
    puse->user()->setOperand(puse->index(), mValue);
  }
  mUses.clear();
}

Value* Value::setComment(const_str_ref comment) {
  if (!mComment.empty()) {
    std::cerr << "re-set basicblock comment!" << std::endl;
  }
  mComment = comment;
  return this;
}
Value* Value::addComment(const_str_ref comment) {
  if (mComment.empty()) {
    mComment = comment;
  } else {
    mComment = mComment + ", " + comment;
  }
  return this;
}
//! User: public Value

/* return as value */
Value* User::operand(size_t index) const {
  assert(index < mOperands.size());
  return mOperands.at(index)->value();
}

void User::addOperand(Value* value) {
  assert(value != nullptr && "value cannot be nullptr");

  auto new_use = utils::make<Use>(mOperands.size(), this, value);

  /* add use to user.mOperands*/
  mOperands.emplace_back(new_use);
  /* add use to value.mUses */
  value->uses().emplace_back(new_use);
}

void User::unuse_allvalue() {
  for (auto& operand : mOperands) {
    operand->value()->uses().remove(operand);
  }
}
void User::delete_operands(size_t index) {
  mOperands.at(index)->value()->uses().remove(mOperands.at(index));
  mOperands.erase(mOperands.begin() + index);
  for (size_t idx = index + 1; idx < mOperands.size(); idx++)
    mOperands.at(idx)->set_index(idx);
  refresh_operand_index();
}
void User::refresh_operand_index() {
  size_t cnt = 0;
  for (auto op : mOperands) {
    op->set_index(cnt);
    cnt++;
  }
}
void User::setOperand(size_t index, Value* value) {
  if (index >= mOperands.size()) {
    std::cerr << "index=" << index << ", but mOperands max size=" << mOperands.size() << std::endl;
    assert(index < mOperands.size());
  }
  auto oldVal = mOperands.at(index)->value();
  oldVal->uses().remove(mOperands.at(index));
  auto newUse = new Use(index, this, value);
  mOperands.at(index) = newUse;
  value->uses().emplace_back(mOperands.at(index));
}

}  // namespace ir