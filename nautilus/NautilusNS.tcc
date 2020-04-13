#ifndef NAUTILUS_NS_TCC
#define NAUTILUS_NS_TCC

#include <vector>
#include <stdint.h>

namespace nautilus {

    template< template< typename, typename > typename _Titer, typename _T >
    std::pair< bool, int32_t > getIndexOfElement(const _Titer< _T* , std::allocator< _T* > >& _container, _T* _element) {
        std::pair< bool, int32_t > result;
        auto it = std::find(_container.begin(), _container.end(), _element);
        if(it != _container.end()) {
            result.first = true;
            result.second = std::distance(_container.begin(), it);
        } else {
            result.first = false;
            result.second = -1;
        }
        return result;
    }

}

#endif      // NAUTILUS_NS_TCC