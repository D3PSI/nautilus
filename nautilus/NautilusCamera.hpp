#ifndef NAUTILUS_CAMERA_HPP
#define NAUTILUS_CAMERA_HPP

#include "NautilusDefaults.hpp"
#include "NautilusStatus.hpp"
#include "NautilusCameraMovementDirection.hpp"
#include "NautilusCameraCoordinateSystemAxis.hpp"
#include "NautilusLogger.hpp"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>

namespace nautilus {

    class NautilusCamera {
    public:

        struct NautilusCameraVectors {
            glm::vec3 m_pos             = defaults::WORLD_ORIGIN;
            glm::vec3 m_frt             = glm::vec3(1.0f, 0.0f, 0.0f);
            glm::vec3 m_wup             = defaults::WORLD_UP;
            glm::vec3 m_rgt             = glm::vec3(0.0f, 0.0f, 1.0f);
            glm::vec3 m_cup             = defaults::WORLD_UP;
        } m_cameraVectors;

        struct NautilusCameraEulerAngles {
            float m_yaw                 = 0.0f;
            float m_pitch               = 0.0f;
            float m_roll                = 0.0f;
        } m_angles;

        /**
         * Default constructor
         */ 
        NautilusCamera(void);

        /**
         * Constructor with position and alignment arguments
         * @param _vec The camera's initial vectors
         * @param _ang The camera's initial angles
         * @param _speed The camera's inital speed
         * @param _sens The camera's mouse response sensitivity
         * @param _fov The camera's inital field of view
         */ 
        NautilusCamera(
            NautilusCameraVectors       _vec,
            NautilusCameraEulerAngles   _ang,
            float                       _speed  = defaults::CAMERA_SPEED,
            float                       _sens   = defaults::CAMERA_SENS,
            float                       _fov    = defaults::CAMERA_FOV);

        /**
         * Default destructor
         */ 
        ~NautilusCamera(void);

        /**
         * Handles keyboard input
         * Must be implemented by derived camera class
         * @param _window A pointer to the GLFWwindow
         * @param _key The GLFW keycode
         * @param _scancode  The system scancode of the key
         * @param _action The action (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT)
         * @param _mods Bitfield describing width modifier keys were held down
         */ 
        virtual void key(
            GLFWwindow*     _window, 
            int             _key, 
            int             _scancode, 
            int             _action, 
            int             _mods) = 0;

        /**
         * Handles mouse input
         * Must be implemented by derived camera class
         * @param _window A pointer to the GLFWwindow
         * @param _x The new x-coordinate relative to the left edge of the window
         * @param _y The new y-coordinate relative to the top edge of the window
         */ 
        virtual void cursor(GLFWwindow* _window, double _x, double _y) = 0;

        /**
         * Handles scroll input
         * Must be implemented by derived camera class
         * @param _window A pointer to the GLFWwindow
         * @param _dx The x-offset of the scroll wheel
         * @param _dy The y-offset of the scroll wheel
         */ 
        virtual void scroll(GLFWwindow* _window, double _dx, double _dy) = 0;

        /**
         * Moves the camera relative to its current position
         * @param _dir The movement direction of the camera
         * @return Returns a NautilusStatus status code
         */ 
        virtual NautilusStatus move(NautilusCameraMovementDirection _dir);

        /**
         * Moves the camera relative to its current position
         * @param _dir The movement direction of the camera
         * @return Returns a NautilusStatus status code
         */ 
        virtual NautilusStatus move(glm::vec3 _dir);

        /**
         * Moves the camera to an absolute world space position
         * @param _pos The position of the camera
         * @return Returns a NautilusStatus status code
         */ 
        virtual NautilusStatus moveTo(glm::vec3 _pos);

        /**
         * Rotates the camera
         * @param _rot The view space rotation axis
         *      ==> X: points left/right on the screen
         *      ==> Y: points up/down on the screen
         *      ==> Z: points into the screen
         * @param _amt The relative amount to move
         * @return Returns a NautilusStatus status code
         */ 
        virtual NautilusStatus rotate(NautilusCameraCoordinateSystemAxis _rot, float _amt = 1.0f);

        /**
         * Rotates the camera
         * @param _rot The arbitrary rotation axis
         * @param _amt The relative amount to move
         * @return Returns a NautilusStatus status code
         */ 
        virtual NautilusStatus rotate(glm::vec3 _rot, float _amt = 1.0f);

        /**
         * Rotates the camera around an arbitrary rotation axis to an absolute value
         * @param _rot The view space rotation axis
         *      ==> X: points left/right on the screen
         *      ==> Y: points up/down on the screen
         *      ==> Z: points into the screen
         * @param _amt The absolute amount to rotate to
         * @return Returns a NautilusStatus status code
         */ 
        virtual NautilusStatus rotateTo(NautilusCameraCoordinateSystemAxis _rot, float _amt = 1.0f);

        /**
         * Rotates the camera around an arbitrary rotation axis to an absolute value
         * @param _rot The arbitrary rotation axis
         * @param _amt The absolute amount to rotate to
         * @return Returns a NautilusStatus status code
         */ 
        virtual NautilusStatus rotateTo(glm::vec3 _rot, float _amt);

        /**
         * Returns the view matrix according to the current camera vectors
         * @return Returns the view matrix
         */ 
        virtual glm::mat4 view(void);

        /**
         * Updates the camera vectors according to movement and orientation
         */ 
        virtual void update(void);

    protected:

        float       m_speed             = defaults::CAMERA_SPEED;
        float       m_sens              = defaults::CAMERA_SENS;
        float       m_fov               = defaults::CAMERA_FOV;
        bool        m_inputEnabled      = true;
        bool        m_firstMouse        = true;

    };

}

#endif      // NAUTILUS_CAMERA_HPP