#ifndef TIECODEC_CORE_POSITION_H
#define TIECODEC_CORE_POSITION_H

#include <cstddef>

namespace tiecodec {

struct SourcePosition {
  std::size_t line = 0;
  std::size_t column = 0;
  std::size_t offset = 0;
};

}

#endif
