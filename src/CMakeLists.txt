set(CMAKE_CXX_STANDARD 20)


if(NOT MSVC)
    add_compile_options(
        -std=c++2a
        -W
        -Wall 
        -Werror 
        -Wextra 
        -Wwrite-strings 
        -Wmissing-braces 
        -Wparentheses 
        -pipe 
        -pedantic-errors 
        -Wconversion
        -Wdeprecated
        -Winit-self
        -Woverloaded-virtual
        -Wuninitialized
        -Wmissing-declarations
        --extra-warnings
        -pedantic
    )
endif()

add_library( orderbook OrderBook.cpp )
