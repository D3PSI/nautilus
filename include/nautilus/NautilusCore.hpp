#ifndef NAUTILUS_CORE_HPP
#define NAUTILUS_CORE_HPP

#include "NautilusStatus.hpp"
#include "NautilusShell.hpp"
#include "NautilusLogger.hpp"

#include <GLFW/glfw3.h>

#include <thread>
#include <mutex>

class NautilusCore {
public:
    
    std::thread* m_t0;

    static NautilusCore& get(void);

    /**
     * Attaches a shell to the core
     * @param _shell A pointer to a derived shell object
     * @return Returns a nautilus::NautilusStatus status code
     */
    static nautilus::NautilusStatus attachShell(NautilusShell* _shell);

    /**
     * Contains the main loop of the application
     * @return Returns a nautilus::NautilusStatus status code
     */
    static nautilus::NautilusStatus loop(void);

    /**
     * Exits the application and ends all processes owned by it
     * @return Returns a nautilus::NautilusStatus status code
     */ 
    static nautilus::NautilusStatus exit(void);

    /**
     * Terminates the application, must be called manually by user, 
     * otherwise program will exit before even starting the main loop.
     * @return Returns a nautilus::NautilusStatus status code
     */ 
    static nautilus::NautilusStatus terminate(void);

    /**
     * Enables Vulkan validation layers if available
     * Only set in debug/development, do not set in release
     * or deployment/production as these are very performance-heavy
     * Sets validation layers for all NautilusVulkanShells
     * @return Returns a nautilus::NautilusStatus status code
     */ 
    static nautilus::NautilusStatus setEnableVulkanValidationLayers(void);

private:

    /**
     * Attaches a shell to the core
     * @param _shell A pointer to a derived shell object
     * @return Returns a nautilus::NautilusStatus status code
     */
    nautilus::NautilusStatus attachShellInternal(NautilusShell* _shell);

    /**
     * Contains the main loop of the application
     * @return Returns a nautilus::NautilusStatus status code
     */
    nautilus::NautilusStatus loopInternal(void);

    /**
     * Exits the application and ends all processes owned by it
     * @return Returns a nautilus::NautilusStatus status code
     */ 
    nautilus::NautilusStatus exitInternal(void);

    /**
     * Terminates the application, must be called manually by user, 
     * otherwise program will exit before even starting the main loop.
     * @return Returns a nautilus::NautilusStatus status code
     */ 
    nautilus::NautilusStatus terminateInternal(void);

    /**
     * Enables Vulkan validation layers if available
     * Only set in debug/development, do not set in release
     * or deployment/production as these are very performance-heavy
     * Sets validation layers for all NautilusVulkanShells
     * @return Returns a nautilus::NautilusStatus status code
     */ 
    nautilus::NautilusStatus setEnableVulkanValidationLayersInternal(void);

    /**
     * Default constructor
     */
    NautilusCore(void);

    /**
     * Default destructor
     */ 
    ~NautilusCore(void);

};

#endif      // NAUTILUS_CORE_HPP