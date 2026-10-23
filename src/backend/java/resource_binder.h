#ifndef TIECODEC_BACKEND_JAVA_RESOURCE_BINDER_H
#define TIECODEC_BACKEND_JAVA_RESOURCE_BINDER_H

#include <string>

#include "symbols.h"

namespace tiecodec {

/// Bridges resource-typed fields to the target platform (DIP: the Java emitter
/// only asks for a field type and initializer expression, it never knows how
/// Android resources are laid out).
class ResourceBinder {
public:
  virtual ~ResourceBinder() = default;

  /// True when `member` is a compile-time resource constant of the platform.
  virtual bool IsResourceField(const ClassInfo& owner, const Member& member) const = 0;

  /// Output type of such a field (e.g. "int").
  virtual std::string FieldType(const ClassInfo& owner, const Member& member) const = 0;

  /// Output initializer expression (e.g. "R.drawable.tu_biao"); registering the
  /// resource as a side effect.
  virtual std::string BindField(const ClassInfo& owner, const Member& member) = 0;

  /// Import needed when a class binds resources (e.g. "结绳.中文.R").
  virtual std::string ResourceImport(const ClassInfo& owner) const = 0;
};

}  // namespace tiecodec

#endif  // TIECODEC_BACKEND_JAVA_RESOURCE_BINDER_H
