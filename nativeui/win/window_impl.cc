// Copyright 2016 Cheng Zhao. All rights reserved.
// Use of this source code is governed by the license that can be found in the
// LICENSE file.

#include "nativeui/win/window_impl.h"

#include <list>

#include "base/bind.h"
#include "base/macros.h"
#include "base/memory/singleton.h"
#include "base/strings/string_number_conversions.h"
#include "base/synchronization/lock.h"
#include "base/win/win_util.h"
#include "base/win/wrapped_window_proc.h"
#include "nativeui/win/screen.h"
#include "ui/gfx/win/hwnd_util.h"

namespace nu {

namespace {

// Several external scripts rely explicitly on this base class name for
// acquiring the window handle and will break if this is modified!
// static
const wchar_t* const kBaseClassName = L"Yue_";

}  // namespace

///////////////////////////////////////////////////////////////////////////////
// WindowImpl class tracking.

// WindowImpl class information used for registering unique windows.
struct ClassInfo {
  UINT style;
  HICON icon;
  HICON small_icon;

  ClassInfo(int style, HICON icon, HICON small_icon)
      : style(style), icon(icon), small_icon(small_icon) {}

  // Compares two ClassInfos. Returns true if all members match.
  bool Equals(const ClassInfo& other) const {
    return (other.style == style && other.icon == icon &&
            other.small_icon == small_icon);
  }
};

// WARNING: this class may be used on multiple threads.
class ClassRegistrar {
 public:
  ~ClassRegistrar();

  static ClassRegistrar* GetInstance();

  void UnregisterClasses();

  // Returns the atom identifying the class matching |class_info|,
  // creating and registering a new class if the class is not yet known.
  ATOM RetrieveClassAtom(const ClassInfo& class_info);

 private:
  // Represents a registered window class.
  struct RegisteredClass {
    RegisteredClass(const ClassInfo& info,
                    const base::string16& name,
                    ATOM atom,
                    HINSTANCE instance);

    // Info used to create the class.
    ClassInfo info;

    // The name given to the window class
    base::string16 name;

    // The atom identifying the window class.
    ATOM atom;

    // The handle of the module containing the window proceedure.
    HMODULE instance;
  };

  ClassRegistrar();
  friend struct base::DefaultSingletonTraits<ClassRegistrar>;

  typedef std::list<RegisteredClass> RegisteredClasses;
  RegisteredClasses registered_classes_;

  // Counter of how many classes have been registered so far.
  int registered_count_;

  base::Lock lock_;

  DISALLOW_COPY_AND_ASSIGN(ClassRegistrar);
};

ClassRegistrar::~ClassRegistrar() {}

// static
ClassRegistrar* ClassRegistrar::GetInstance() {
  return base::Singleton<ClassRegistrar,
                         base::LeakySingletonTraits<ClassRegistrar>>::get();
}

void ClassRegistrar::UnregisterClasses() {
  for (RegisteredClasses::iterator i = registered_classes_.begin();
        i != registered_classes_.end(); ++i) {
    if (UnregisterClass(MAKEINTATOM(i->atom), i->instance)) {
      registered_classes_.erase(i);
    } else {
      LOG(ERROR) << "Failed to unregister class " << i->name
                 << ". Error = " << GetLastError();
    }
  }
}

ATOM ClassRegistrar::RetrieveClassAtom(const ClassInfo& class_info) {
  base::AutoLock auto_lock(lock_);
  for (RegisteredClasses::const_iterator i = registered_classes_.begin();
       i != registered_classes_.end(); ++i) {
    if (class_info.Equals(i->info))
      return i->atom;
  }

  // No class found, need to register one.
  base::string16 name = base::string16(kBaseClassName) +
      base::IntToString16(registered_count_++);

  WNDCLASSEX window_class;
  base::win::InitializeWindowClass(
      name.c_str(), &base::win::WrappedWindowProc<WindowImpl::WndProc>,
      class_info.style, 0, 0, NULL,
      reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), NULL,
      class_info.icon, class_info.small_icon, &window_class);
  HMODULE instance = window_class.hInstance;
  ATOM atom = RegisterClassEx(&window_class);
  CHECK(atom) << GetLastError();

  registered_classes_.push_back(RegisteredClass(
      class_info, name, atom, instance));

  return atom;
}

ClassRegistrar::RegisteredClass::RegisteredClass(const ClassInfo& info,
                                                 const base::string16& name,
                                                 ATOM atom,
                                                 HMODULE instance)
    : info(info),
      name(name),
      atom(atom),
      instance(instance) {}

ClassRegistrar::ClassRegistrar() : registered_count_(0) {}


///////////////////////////////////////////////////////////////////////////////
// WindowImpl, public

// static
const DWORD WindowImpl::kWindowDefaultChildStyle =
    WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
const DWORD WindowImpl::kWindowDefaultStyle =
    WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN;

WindowImpl::WindowImpl(base::StringPiece16 class_name, HWND parent,
                       DWORD window_style, DWORD window_ex_style)
    : BaseView(false), class_style_(CS_DBLCLKS) {
  if (parent == HWND_DESKTOP) {
    // Only non-child windows can have HWND_DESKTOP (0) as their parent.
    CHECK_EQ(static_cast<int>(window_style & WS_CHILD), 0);
    parent = gfx::GetWindowToParentTo(false);
  } else if (parent == ::GetDesktopWindow()) {
    // Any type of window can have the "Desktop Window" as their parent.
    parent = gfx::GetWindowToParentTo(true);
  } else if (parent != HWND_MESSAGE) {
    CHECK(::IsWindow(parent));
  }

  HWND hwnd = CreateWindowEx(
      window_ex_style,
      class_name.empty() ? reinterpret_cast<wchar_t*>(GetWindowClassAtom())
                         : class_name.data(),
      NULL, window_style, -1, -1, 1, 1, parent, NULL, NULL, this);
  // First nccalcszie (during CreateWindow) for captioned windows is
  // deliberately ignored so force a second one here to get the right
  // non-client set up.
  if (hwnd && (window_style & WS_CAPTION)) {
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
                 SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE |
                 SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
  }

  // For custom window we the hwnd_ is assigned in WM_NCCREATE.
  if (class_name.empty()) {
    // The window procedure should have set the data for us.
    gfx::CheckWindowCreated(hwnd_);
    CHECK_EQ(this, gfx::GetWindowUserData(hwnd));
  } else {
    hwnd_ = hwnd;
    gfx::SetWindowUserData(hwnd, this);
  }

  Init(hwnd_, GetScaleFactorForHWND(hwnd_));
}

WindowImpl::~WindowImpl() {
  if (!::IsWindow(hwnd_))  // handle already destroyed.
    return;

  if (::GetParent(hwnd_) == NULL)  // removing a child window.
    ::SetParent(hwnd_, NULL);

  gfx::SetWindowUserData(hwnd_, NULL);
  ::DestroyWindow(hwnd_);
}

void WindowImpl::SetPixelBounds(const gfx::Rect& bounds) {
  bounds_ = bounds;

  // Calculate the bounds relative to parent HWND.
  gfx::Point pos(bounds.origin());
  for (BaseView* p = parent(); p && p->is_virtual(); p = p->parent()) {
    gfx::Point offset = p->GetPixelBounds().origin();
    pos.set_x(pos.x() + offset.x());
    pos.set_y(pos.y() + offset.y());
  }

  SetWindowPos(hwnd_, NULL, pos.x(), pos.y(), bounds.width(), bounds.height(),
               SWP_NOACTIVATE | SWP_NOZORDER);
  RedrawWindow(hwnd_, NULL, NULL, RDW_INVALIDATE | RDW_ALLCHILDREN);
}

gfx::Rect WindowImpl::GetPixelBounds() {
  return bounds_;
}

void WindowImpl::SetParent(BaseView* parent) {
  BaseView::SetParent(parent);
  ::SetParent(hwnd_, parent ? parent->hwnd() : NULL);
}

HICON WindowImpl::GetDefaultWindowIcon() const {
  return nullptr;
}

HICON WindowImpl::GetSmallWindowIcon() const {
  return nullptr;
}

LRESULT WindowImpl::OnWndProc(UINT message, WPARAM w_param, LPARAM l_param) {
  LRESULT result = 0;

  HWND hwnd = hwnd_;
  if (message == WM_NCDESTROY)
    hwnd_ = NULL;

  // Handle the message if it's in our message map; otherwise, let the system
  // handle it.
  if (!ProcessWindowMessage(hwnd, message, w_param, l_param, result))
    result = DefWindowProc(hwnd, message, w_param, l_param);

  return result;
}

BOOL WindowImpl::ProcessWindowMessage(
    HWND, UINT, WPARAM, LPARAM, LRESULT&, DWORD dwMsgMapID) {
  return false;
}

// static
LRESULT CALLBACK WindowImpl::WndProc(HWND hwnd,
                                     UINT message,
                                     WPARAM w_param,
                                     LPARAM l_param) {
  if (message == WM_NCCREATE) {
    CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(l_param);
    WindowImpl* window = reinterpret_cast<WindowImpl*>(cs->lpCreateParams);
    DCHECK(window);
    gfx::SetWindowUserData(hwnd, window);
    window->hwnd_ = hwnd;
    return TRUE;
  }

  WindowImpl* window =
      reinterpret_cast<WindowImpl*>(gfx::GetWindowUserData(hwnd));
  if (!window)
    return 0;

  return window->OnWndProc(message, w_param, l_param);
}

ATOM WindowImpl::GetWindowClassAtom() {
  HICON icon = GetDefaultWindowIcon();
  HICON small_icon = GetSmallWindowIcon();
  ClassInfo class_info(CS_DBLCLKS, icon, small_icon);
  return ClassRegistrar::GetInstance()->RetrieveClassAtom(class_info);
}

}  // namespace nu
