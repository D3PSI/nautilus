#ifndef NAUTILUS_CORE_HPP
#define NAUTILUS_CORE_HPP

#include "NautilusStatus.hpp"
#include "NautilusShell.hpp"

class NautilusCore {
public:

    /**
     * Default constructor
     */
    NautilusCore(void);

    /**
     * Attaches a shell to the core
     * @return Returns a NautilusCore status code
     */
    NautilusStatus attachShell(NautilusShell _shell);

    /**
     * Default destructor
     */ 
    ~NautilusCore(void);

};

#endif      // NAUTILUS_CORE_HPP