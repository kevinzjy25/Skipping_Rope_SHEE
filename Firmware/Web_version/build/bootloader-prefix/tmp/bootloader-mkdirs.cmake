# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/kevinzjy/ESP_container/v5.4.2/esp-idf/components/bootloader/subproject"
  "/home/kevinzjy/Project2025/Skipping_Rope_SHEE/Firmware/Web_version/build/bootloader"
  "/home/kevinzjy/Project2025/Skipping_Rope_SHEE/Firmware/Web_version/build/bootloader-prefix"
  "/home/kevinzjy/Project2025/Skipping_Rope_SHEE/Firmware/Web_version/build/bootloader-prefix/tmp"
  "/home/kevinzjy/Project2025/Skipping_Rope_SHEE/Firmware/Web_version/build/bootloader-prefix/src/bootloader-stamp"
  "/home/kevinzjy/Project2025/Skipping_Rope_SHEE/Firmware/Web_version/build/bootloader-prefix/src"
  "/home/kevinzjy/Project2025/Skipping_Rope_SHEE/Firmware/Web_version/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/kevinzjy/Project2025/Skipping_Rope_SHEE/Firmware/Web_version/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/kevinzjy/Project2025/Skipping_Rope_SHEE/Firmware/Web_version/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
