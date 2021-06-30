module;
#include <qwidget.h>
#include <QMouseEvent>
import Input;
export module RenderViewWidget;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

export class RenderViewWidget : public QWidget
{
public:
    RenderViewWidget(QWidget* parent) : QWidget(parent) {}

    bool nativeEvent(const QByteArray& eventType, void* message, long* result)
    {
        MSG* msg = (MSG*)message;
        if (ImGui_ImplWin32_WndProcHandler(msg->hwnd, msg->message, msg->wParam, msg->lParam))
        {
            return true;
        }

        gCoreSystem.HandleWin32MessagePump(msg->message, msg->wParam, msg->lParam);

        return false;
    }

    virtual void mousePressEvent(QMouseEvent* mouseEvent)
    {
        if (mouseEvent->button() == Qt::MouseButton::LeftButton)
        {
            gInputSystem.StoreMouseLeftDownInput();
        }
        else if (mouseEvent->button() == Qt::MouseButton::RightButton)
        {
            gInputSystem.StoreMouseRightDownInput();
        }
    }

    virtual void wheelEvent(QWheelEvent* mouseWheelEvent)
    {
        if (mouseWheelEvent->angleDelta().y() > 0)
        {
            gInputSystem.StoreMouseWheelUp();
        }
        else if (mouseWheelEvent->angleDelta().y() < 0)
        {
            gInputSystem.StoreMouseWheelDown();
        }
    }

};
