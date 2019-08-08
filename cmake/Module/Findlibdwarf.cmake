include(FindPackageHandleStandardArgs)

set(libdwarf_ROOT_DIR "" CACHE PATH "Folder contains libdwarf")

find_path(libdwarf_INCLUDE_DIR libdwarf.h
        PATHS ${libdwarf_ROOT_DIR}
        PATH_SUFFIXES include)

find_library(libdwarf_LIBRARY dwarf
        PATHS ${libdwarf_ROOT_DIR}
        PATH_SUFFIXES lib lib64)

find_package_handle_standard_args(libdwarf DEFAULT_MSG libdwarf_INCLUDE_DIR libdwarf_LIBRARY)

if(libdwarf_FOUND)
    set(libdwarf_INCLUDE_DIRS ${libdwarf_INCLUDE_DIR})
    set(libdwarf_LIBRARIES ${libdwarf_LIBRARY})
    message(STATUS "Found libdwarf (include: ${libdwarf_INCLUDE_DIR}, library: ${libdwarf_LIBRARY})")
    mark_as_advanced(libdwarf_ROOT_DIR
            libdwarf_LIBRARY libdwarf_INCLUDE_DIR)
endif()
