#pragma once

class IToolHost {
  public:
    virtual ~IToolHost() = default;
    virtual void ToolHost__update() = 0;
    virtual void ToolHost__enable_mouse_tracking(bool v) = 0;
};
