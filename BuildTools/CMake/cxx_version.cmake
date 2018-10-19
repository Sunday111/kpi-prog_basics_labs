cmake_minimum_required(VERSION 3.5.1)

function(require_cxx_version target version)
    set_property(TARGET ${target} PROPERTY CXX_STANDARD ${version})
    set_property(TARGET ${target} PROPERTY CXX_STANDARD_REQUIRED ON)
endfunction()

function(disable_cxx_extensions target)
    set_property(TARGET ${target} PROPERTY CXX_EXTENSIONS OFF)
endfunction()