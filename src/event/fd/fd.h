#pragma once

#include "../../util/result.h"

#include <iostream>
#include <memory>

class FD {
  protected:
    FD();

  public:
    virtual ~FD();

    Result<int> get() { return m_fd_result; }

    // for debugging purposes only
    static int _OPEN_COUNT;
    static int _CLOSE_COUNT;

  protected:
    Result<int> m_fd_result;
};