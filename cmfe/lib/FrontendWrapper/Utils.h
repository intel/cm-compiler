#ifndef LLVM_FRONTEND_WRAPPER_UTILS_H
#define LLVM_FRONTEND_WRAPPER_UTILS_H

#include "llvm/ADT/IntrusiveRefCntPtr.h"

#include <utility>

namespace wrapper {
template <typename Type, typename... Args>
llvm::IntrusiveRefCntPtr<Type> MakeIntrusiveRefCntPtr(Args &&... args) {
  return llvm::IntrusiveRefCntPtr<Type>(new Type(std::forward<Args>(args)...));
}
} // namespace wrapper

#endif // LLVM_FRONTEND_WRAPPER_UTILS_H