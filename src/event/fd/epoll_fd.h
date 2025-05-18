#pragma once

#include "fd.h"

class EpollFD : public FD {
  public:
    EpollFD();
    ~EpollFD() {}
};