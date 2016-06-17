find_package(PkgConfig)
pkg_check_modules(PC_OPENCC REQUIRED opencc)
set(OPENCC_DEFINITIONS ${PC_LIBXML_CFLAGS_OTHER})

find_path(OPENCC_INCLUDE_DIR opencc
          HINTS ${PC_OPENCC_INCLUDEDIR} ${PC_OPENCC_INCLUDE_DIRS}
          PATH_SUFFIXES opencc)

find_library(OPENCC_LIBRARY NAMES opencc libopencc
             HINTS ${PC_OPENCC_LIBDIR} ${PC_OPENCC_LIBRARY_DIRS} )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(opencc  DEFAULT_MSG
                                  OPENCC_LIBRARY OPENCC_INCLUDE_DIR)

mark_as_advanced(OPENCC_INCLUDE_DIR OPENCC_LIBRARY)
set(OPENCC_LIBRARIES ${OPENCC_LIBRARY})
set(OPENCC_INCLUDE_DIRS ${OPENCC_INCLUDE_DIR})
