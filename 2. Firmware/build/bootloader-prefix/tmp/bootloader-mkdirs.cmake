# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/odemki/esp/v5.3.1/esp-idf/components/bootloader/subproject"
  "D:/work/ESP32/Log project/2. ESP32 GPS loger v2/2. Firmware/build/bootloader"
  "D:/work/ESP32/Log project/2. ESP32 GPS loger v2/2. Firmware/build/bootloader-prefix"
  "D:/work/ESP32/Log project/2. ESP32 GPS loger v2/2. Firmware/build/bootloader-prefix/tmp"
  "D:/work/ESP32/Log project/2. ESP32 GPS loger v2/2. Firmware/build/bootloader-prefix/src/bootloader-stamp"
  "D:/work/ESP32/Log project/2. ESP32 GPS loger v2/2. Firmware/build/bootloader-prefix/src"
  "D:/work/ESP32/Log project/2. ESP32 GPS loger v2/2. Firmware/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/work/ESP32/Log project/2. ESP32 GPS loger v2/2. Firmware/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/work/ESP32/Log project/2. ESP32 GPS loger v2/2. Firmware/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
