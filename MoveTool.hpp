#pragma once

#include "ToolHost.h"
#include "types.hpp"

class QPainter;

struct MouseEvent {
    int x;
    int y;
};

class IToolHandler {
  public:
    virtual ~IToolHandler() = default;
    virtual void render(QPainter &p, bool debug) = 0;
    virtual void activated() = 0;
    virtual void deactivated() = 0;
    virtual void mouse_move(MouseEvent) = 0;
    virtual void mouse_press(MouseEvent) = 0;
    virtual void mouse_release(MouseEvent) = 0;
};

class MoveTool : public IToolHandler {
  public:
    explicit MoveTool(IToolHost &host, Model &m) : m_host(host), m_model(m) {}
    virtual void render(QPainter &p, bool debug) override;
    virtual void activated() override;
    virtual void deactivated() override;
    virtual void mouse_move(MouseEvent) override;
    virtual void mouse_press(MouseEvent) override;
    virtual void mouse_release(MouseEvent) override;

  protected:
    void update() { m_host.ToolHost__update(); }
    void setMoouseTracking(bool v) { m_host.ToolHost__enable_mouse_tracking(v); }

  private:
    IToolHost &m_host;
    Model &m_model;
};
