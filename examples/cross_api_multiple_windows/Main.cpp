#include <nautilus/Nautilus.hpp>

/**
 * Main entry point for the application
 */
int main() {
    nautilus::NautilusShell*  shell1 = new nautilus::NautilusShellOpenGL();
    nautilus::NautilusShell*  shell2 = new nautilus::NautilusShellVulkan();
    shell2->setShellExtent(720, 450);
    nautilus::NautilusCore::attach(shell1);
    nautilus::NautilusCore::attach(shell2);
    nautilus::NautilusCore::terminate();
    delete shell1;
    delete shell2;
    return 0;
}