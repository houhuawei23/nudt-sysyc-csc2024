#pragma once
#include "ir/ir.hpp"

namespace mir {

// big endian OR little endian
enum Endian { Big, Little };

/*
 * @brief: DataLayout Class (抽象基类)
 * @note:
 *      Data Layout (virtual class, define the api)
 */
class DataLayout {
public:
  virtual ~DataLayout() = default;

public:  // get function
  virtual Endian edian() const = 0;
  virtual size_t pointerSize() const = 0;

  virtual size_t typeAlign(ir::Type* type) const = 0;
  virtual size_t codeAlign() const = 0;
  virtual size_t memAlign() const = 0;
};
}  // namespace mir
