include(GNUInstallDirs)

# batt_VERSION is only set if we are an LTS release being installed, in which
# case it may be into a system directory and so we need to make subdirectories
# for each installed version of Batteries.

if(batt_VERSION)
  set(BATT_SUBDIR "${PROJECT_NAME}_${PROJECT_VERSION}")
  set(BATT_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}/${BATT_SUBDIR}")
  set(BATT_INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${BATT_SUBDIR}")
  set(BATT_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/{BATT_SUBDIR}")
  set(BATT_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/${BATT_SUBDIR}")
else()
  set(BATT_INSTALL_BINDIR "${CMAKE_INSTALL_BINDIR}")
  set(BATT_INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
  set(BATT_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}")
  set(BATT_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}")
endif()