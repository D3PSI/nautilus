#ifndef NAUTILUS_SHELL_CPP
#define NAUTILUS_SHELL_CPP

#include "NautilusShell.hpp"
#include "NautilusNS.hpp"

#include <iostream>

NautilusShell::NautilusShell() {
    this->m_camera = new NautilusCamera2D();
}

NautilusShell::~NautilusShell() {
    delete this->m_camera;
}

void NautilusShell::onDetach(GLFWwindow* _window) {
}

void NautilusShell::onResize(GLFWwindow* _window, int _w, int _h) {
}

void NautilusShell::onFocus(GLFWwindow* _window, int _focus) {
}

void NautilusShell::onIconify(GLFWwindow* _window, int _iconify) {
}

void NautilusShell::onCursor(GLFWwindow* _window, double _x, double _y) {
}

void NautilusShell::onCursorIn(GLFWwindow* _window, int _enter) {
}

void NautilusShell::onKey(
    GLFWwindow*     _window, 
    int             _key, 
    int             _scancode, 
    int             _action, 
    int             _mods) {
    if(_key == GLFW_KEY_ESCAPE && _action == GLFW_PRESS) {
        glfwSetWindowShouldClose(this->m_window, GLFW_TRUE);
    }
}

void NautilusShell::onScroll(GLFWwindow* _window, double _dx, double _dy) {
}

void NautilusShell::resize(GLFWwindow* _window, int _w, int _h) {
    this->onResize(_window, _w, _h);
}

void NautilusShell::focus(GLFWwindow* _window, int _focus) {
    this->onFocus(_window, _focus);
}

void NautilusShell::iconify(GLFWwindow* _window, int _iconify) {
    this->onIconify(_window, _iconify);
}

void NautilusShell::cursor(GLFWwindow* _window, double _x, double _y) {
    this->onCursor(_window, _x, _y);
    this->m_camera->cursor(_window, _x, _y);
}

void NautilusShell::cursorIn(GLFWwindow* _window, int _enter) {
    this->onCursorIn(_window, _enter);
}

void NautilusShell::key(
    GLFWwindow*     _window, 
    int             _key, 
    int             _scancode, 
    int             _action, 
    int             _mods) {
    this->onKey(
        _window,
        _key,
        _scancode,
        _action,
        _mods);
    this->m_camera->key(
        _window,
        _key,
        _scancode,
        _action,
        _mods);
}

void NautilusShell::scroll(GLFWwindow* _window, double _dx, double _dy) {
    this->onScroll(_window, _dx, _dy);
    this->m_camera->scroll(_window, _dx, _dy);
}

nautilus::NautilusStatus NautilusShell::setShellContext(nautilus::NautilusShellContext _context) {
    this->m_shellContext = _context;
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusShell::setShellTitle(std::string _title) {
    this->m_title = _title;
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusShell::setShellExtent(uint32_t _width, uint32_t _height) {
    this->m_width = _width;
    this->m_height = _height;
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusShell::setShellIcon(std::string _path) {
    this->m_shellIconPath = _path;
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusShell::attach() {
    this->createWindow();
    this->setCallbacks();
    this->onAttach();
    std::unique_lock< std::mutex > lock(this->m_attachedLock);
    this->m_attached = true;
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusShell::detach() {
    std::scoped_lock< std::mutex > lock(m_attachedLock);
    this->m_attached = false;
    delete this->m_camera;
    this->clean();
    this->onDetach(this->m_window);
    glfwDestroyWindow(this->m_window);
    nautilus::shells.erase(nautilus::shells.begin() 
        + nautilus::getIndexOfElement(nautilus::shells, this).second);
    std::scoped_lock< std::mutex > shellCountLock(nautilus::shellCountLock);
    nautilus::shellCount--;
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusShell::setCallbacks() {
    if(this->m_callbacksSet) return nautilus::NAUTILUS_STATUS_OK;
    glfwSetFramebufferSizeCallback(this->m_window, nautilus::dispatcher::resize);
    glfwSetWindowFocusCallback(this->m_window, nautilus::dispatcher::focus);
    glfwSetWindowIconifyCallback(this->m_window, nautilus::dispatcher::iconify);
    glfwSetCursorPosCallback(this->m_window, nautilus::dispatcher::cursor);
    glfwSetCursorEnterCallback(this->m_window, nautilus::dispatcher::cursorIn);
    glfwSetKeyCallback(this->m_window, nautilus::dispatcher::key);
    this->m_callbacksSet = true;
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusShell::setDefaultWindowHints() {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    if(!this->m_decoration) glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusShell::createWindow() { 
    if(this->m_windowCreated) return nautilus::NAUTILUS_STATUS_OK;
    nautilus::logger::log("Creating GLFWwindow...");
    this->setDefaultWindowHints();
    this->setAPIWindowHints();
    this->m_monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(this->m_monitor);
    if(this->m_shellContext == nautilus::NAUTILUS_SHELL_CONTEXT_WINDOWED) {
        this->m_window = glfwCreateWindow(
            this->m_width,
            this->m_height,
            this->m_title.c_str(),
            nullptr,
            nullptr);
        glfwSetWindowPos(
            this->m_window,
            mode->width / 2 - this->m_width / 2,
            mode->height / 2 - this->m_height / 2);
    }
    else if(this->m_shellContext == nautilus::NAUTILUS_SHELL_CONTEXT_FULLSCREEN) {
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
        this->m_window = glfwCreateWindow(
            mode->width,
            mode->height,
            this->m_title.c_str(),
            this->m_monitor,
            nullptr);
    }
    else if(this->m_shellContext == nautilus::NAUTILUS_SHELL_CONTEXT_BORDERLESS) {
        this->m_window = glfwCreateWindow(
            mode->width,
            mode->height,
            this->m_title.c_str(),
            this->m_monitor,
            nullptr);
    }
    if(this->m_window == nullptr) {
        nautilus::logger::log("Failed to create GLFWwindow", nautilus::NAUTILUS_STATUS_FATAL);
        return nautilus::NAUTILUS_STATUS_FATAL;
    }
    glfwMakeContextCurrent(this->m_window);
    glfwSwapInterval(0);
    GLFWimage windowIcon[1];
    windowIcon[0].pixels = nautilus::loadSTBI(
        this->m_shellIconPath,
        &windowIcon[0].width,
        &windowIcon[0].height,
        0,
        4);
    glfwSetWindowIcon(this->m_window, 1, windowIcon);
    nautilus::freeSTBI(windowIcon[0].pixels);
    glfwSetWindowUserPointer(this->m_window, this);
    nautilus::logger::log("Successfully created GLFWwindow");
    this->initAPI();
    this->m_windowCreated = true;
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusShell::setShellCamera(const nautilus::NautilusCameraMode& _mode) {
    this->m_cameraMode = _mode;
    delete this->m_camera;
    switch(_mode) {
        case nautilus::NAUTILUS_CAMERA_MODE_2D:
            this->m_dim = nautilus::NAUTILUS_DIMENSION_2D;
            this->m_camera = new NautilusCamera2D();
            break;
        case nautilus::NAUTILUS_CAMERA_MODE_FPS:
            this->m_dim = nautilus::NAUTILUS_DIMENSION_3D;
            this->m_camera = new NautilusCameraFPS();
            break;
        case nautilus::NAUTILUS_CAMERA_MODE_FOCUS:
            this->m_dim = nautilus::NAUTILUS_DIMENSION_3D;
            this->m_camera = new NautilusCameraFocus();
            break;
        default:
            nautilus::logger::log("Unknown camera mode", nautilus::NAUTILUS_STATUS_FATAL);
            break;
    }
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusShell::setShellDimension(const nautilus::NautilusDimension& _dim) {
    this->m_dim = _dim;
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusShell::setShellViewport(const nautilus::NautilusViewport& _viewport) {
    this->m_viewport = _viewport;
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusShell::setShellRefreshRate(uint32_t _fps) {
    this->m_fps = _fps;
    return nautilus::NAUTILUS_STATUS_OK;
}

bool NautilusShell::mustRender() {
    static auto start = std::chrono::system_clock::now();
    auto now = std::chrono::system_clock::now();
    std::chrono::duration< double > elapsed = now - start;
    return elapsed.count() >= 1 / this->m_fps;
}

nautilus::NautilusStatus NautilusShell::printStats() {
    double              currentTime         = glfwGetTime();
    double              deltaTime           = currentTime - this->m_pastTime;
    this->m_pastTime = currentTime;
    this->m_nbFrames++;
    float seconds = 10.0f;
    if(currentTime - this->m_lastTime >= 1.0 && this->m_nbFrames > this->m_maxfps)
        this->m_maxfps = this->m_nbFrames;
    if(currentTime - this->m_lastTime >= seconds) {
        std::string fps                = "Average FPS (last " + std::to_string(seconds) + " seconds):    " + std::to_string(double(this->m_nbFrames / seconds));
        std::string frametime          = "Average Frametime (last " + std::to_string(seconds) + " seconds):    " + std::to_string(double((1000.0 * seconds) /this->m_nbFrames)) + " ms";
        std::string maxFPS             = "Max FPS:    " + std::to_string(double(this->m_maxfps / seconds));
        nautilus::logger::log("Stats for '" + this->m_title + "':");
        nautilus::logger::log(fps);
        nautilus::logger::log(frametime);
        nautilus::logger::log(maxFPS);
        this->m_nbFrames = 0;
        this->m_lastTime += seconds;
    }
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusShell::setShellDecoration(bool _decoration) {
    this->m_decoration = _decoration;
    return nautilus::NAUTILUS_STATUS_OK;
}

nautilus::NautilusStatus NautilusShell::attach(NautilusPipeline* _pipe) {
    return nautilus::NAUTILUS_STATUS_OK;
}

#endif      // NAUTILUS_SHELL_CPP