#include "ir/global.hpp"

#include "ir/ConstantValue.hpp"
namespace ir {
/*
 * @brief: GlobalVariable::print
 * @example:
 *      scalar: @a = global i32 0
 *      array:  @a = global [4 x [4 x i32]] zeroinitializer
 */
void GlobalVariable::print(std::ostream& os) const {
  os << "@" << mName;
  if (mIsConst)
    os << " = constant ";
  else
    os << " = global ";
  os << *(baseType()) << " ";

  if (isArray()) {
    size_t idx = 0, dimensions = dims_cnt();
    if (isInit()) print_ArrayInit(os, dimensions, 0, &idx);
    else os << "zeroinitializer";
  } else {
    os << *scalarValue();
  }

  os << "\n";
}

void GlobalVariable::print_ArrayInit(std::ostream& os,
                                     const size_t dimension,
                                     const size_t begin,
                                     size_t* idx) const {
  auto atype = dyn_cast<ArrayType>(baseType());
  auto baseType = atype->baseType();
  bool is_zero = true;
  if (begin + 1 == dimension) {
    size_t num = atype->dim(begin);

    is_zero = true;
    for (size_t i = 0; i < num; i++) {
      // auto cinit = init(*idx + i)->dynCast<ConstantInteger>();
      auto cinit = init(*idx + i)->dynCast<ConstantValue>();
      if (!cinit->isZero()) {
        is_zero = false;
        break;
      }
    }

    if (is_zero)
      os << "zeroinitializer";
    else {
      os << "[";
      for (size_t i = 0; i < num - 1; i++) {
        os << *(baseType) << " " << *init(*idx + i) << ", ";
      }
      os << *(baseType) << " " << *init(*idx + num - 1);
      os << "]";
    }
    *idx = *idx + num;
  } else if (dimension == 2 + begin) {
    os << "[";

    size_t num1 = atype->dim(begin), num2 = atype->dim(begin + 1);
    for (size_t i = 0; i < num1 - 1; i++) {
      os << "[" << num2 << " x " << *(baseType) << "] ";

      is_zero = true;
      for (size_t j = 0; j < num2; j++) {
        auto cinit = init(*idx + i * num2 + j)->dynCast<ConstantValue>();
        if (!cinit->isZero()) {
          is_zero = false;
          break;
        }
      }
      if (is_zero)
        os << "zeroinitializer, ";
      else {
        os << "[";
        for (size_t j = 0; j < num2 - 1; j++) {
          os << *(baseType) << " " << *init(*idx + i * num2 + j) << ", ";
        }
        os << *(baseType) << " " << *init(*idx + i * num2 + num2 - 1);
        os << "], ";
      }
    }
    os << "[" << num2 << " x " << *(baseType) << "] ";

    is_zero = true;
    for (size_t j = 0; j < num2; j++) {
      auto cinit = init(*idx + (num1 - 1) * num2 + j)->dynCast<ConstantValue>();
      if (!cinit->isZero()) {
        is_zero = false;
        break;
      }
    }
    if (is_zero)
      os << "zeroinitializer";
    else {
      os << "[";
      for (size_t j = 0; j < num2 - 1; j++) {
        os << *(baseType) << " " << *init(*idx + (num1 - 1) * num2 + j) << ", ";
      }
      os << *(baseType) << " " << *init(*idx + (num1 - 1) * num2 + num2 - 1);
      os << "]";
    }

    *idx = *idx + (num1 - 1) * num2 + num2;
    os << "]";
  } else {
    os << "[";

    size_t num = atype->dim(begin), num1 = atype->dim(begin + 1),
        num2 = atype->dim(begin + 2);
    for (size_t i = 1; i < num; i++) {
      for (size_t cur = begin + 1; cur < dimension; cur++) {
        auto value = atype->dim(cur);
        os << "[" << value << " x ";
      }
      os << *(baseType);
      for (size_t cur = begin + 1; cur < dimension; cur++)
        os << "]";
      os << " ";
      print_ArrayInit(os, dimension, begin + 1, idx);
      os << ", ";
    }
    for (size_t cur = begin + 1; cur < dimension; cur++) {
      auto value = atype->dim(cur);
      os << "[" << value << " x ";
    }
    os << *(baseType);
    for (size_t cur = begin + 1; cur < dimension; cur++)
      os << "]";
    os << " ";
    print_ArrayInit(os, dimension, begin + 1, idx);

    os << "]";
  }
}
}  // namespace ir