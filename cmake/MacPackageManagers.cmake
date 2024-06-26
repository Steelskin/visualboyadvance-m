# This module checks for Mac Homebrew, MacPorts, or Fink, in that order, on Mac
# OS X.
#
# It will prepend the active package manager's paths to:
#
# CMAKE_FRAMEWORK_PATH
# CMAKE_INCLUDE_PATH
# CMAKE_LIBRARY_PATH
# CMAKE_PROGRAM_PATH
#
# In addition, the following commands are called with the package manager's
# paths:
#
# include_directories()
# link_directories()
#
# The paths of package managers not currently in $ENV{PATH} are added to
# CMAKE_IGNORE_PATH .
#
# Copyright (c) 2016, Rafael Kitover
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
# 
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
# 
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

if(NOT APPLE)
    return()
endif()

if(NOT "$ENV{IN_NIX_SHELL}" STREQUAL "")
    message(STATUS "Configuring for Nix")

    set(NIX ON)

    set(CMAKE_IGNORE_PATH /opt/local /opt/local/bin /opt/local/include /opt/local/Library/Frameworks /opt/local/lib ${CMAKE_IGNORE_PATH})
    set(CMAKE_IGNORE_PATH /sw /sw/bin /sw/include /sw/Library/Frameworks /sw/lib ${CMAKE_IGNORE_PATH})
elseif(EXISTS /usr/local/bin/brew AND $ENV{PATH} MATCHES "(^|:)/usr/local/bin/?(:|$)")
    message(STATUS "Configuring for Mac Homebrew")

    set(MAC_HOMEBREW ON)

    set(CMAKE_IGNORE_PATH /opt/local /opt/local/bin /opt/local/include /opt/local/Library/Frameworks /opt/local/lib ${CMAKE_IGNORE_PATH})
    set(CMAKE_IGNORE_PATH /sw /sw/bin /sw/include /sw/Library/Frameworks /sw/lib ${CMAKE_IGNORE_PATH})

    set(CMAKE_FRAMEWORK_PATH /usr/local/Frameworks ${CMAKE_FRAMEWORK_PATH})

    set(CMAKE_INCLUDE_PATH /usr/local/include ${CMAKE_INCLUDE_PATH})
    include_directories("/usr/local/include")

    set(CMAKE_LIBRARY_PATH /usr/local/lib ${CMAKE_LIBRARY_PATH})
    link_directories("/usr/local/lib")

    set(CMAKE_PROGRAM_PATH /usr/local/bin ${CMAKE_PROGRAM_PATH})

    set(ZLIB_ROOT /usr/local/opt/zlib)
elseif(EXISTS /opt/local/bin/port AND $ENV{PATH} MATCHES "(^|:)/opt/local/bin/?(:|$)")
    message(STATUS "Configuring for MacPorts")

    set(MACPORTS ON)

    set(CMAKE_IGNORE_PATH /sw /sw/bin /sw/include /sw/Library/Frameworks /sw/lib ${CMAKE_IGNORE_PATH})

    set(CMAKE_FRAMEWORK_PATH /opt/local/Library/Frameworks ${CMAKE_FRAMEWORK_PATH})

    set(CMAKE_INCLUDE_PATH /opt/local/include ${CMAKE_INCLUDE_PATH})
    include_directories("/opt/local/include")

    set(CMAKE_LIBRARY_PATH /opt/local/lib ${CMAKE_LIBRARY_PATH})
    link_directories("/opt/local/lib")

    set(CMAKE_PROGRAM_PATH /opt/local/bin ${CMAKE_PROGRAM_PATH})
elseif(EXISTS /sw/bin/fink AND $ENV{PATH} MATCHES "(^|:)/sw/bin/?(:|$)")
    message(STATUS "Configuring for Fink")

    set(FINK ON)

    set(CMAKE_IGNORE_PATH /opt/local /opt/local/bin /opt/local/include /opt/local/Library/Frameworks /opt/local/lib ${CMAKE_IGNORE_PATH})

    set(CMAKE_FRAMEWORK_PATH /sw/Library/Frameworks ${CMAKE_FRAMEWORK_PATH})

    set(CMAKE_INCLUDE_PATH /sw/include ${CMAKE_INCLUDE_PATH})
    include_directories("/sw/include")

    set(CMAKE_LIBRARY_PATH /sw/lib ${CMAKE_LIBRARY_PATH})
    link_directories("/sw/lib")

    set(CMAKE_PROGRAM_PATH /sw/bin ${CMAKE_PROGRAM_PATH})
else()
    # no package manager found or active, do nothing
    return()
endif()

# only ignore /usr/local if brew is installed and not in the PATH
# in other cases, it is the user's personal installations
if(NOT MAC_HOMEBREW AND EXISTS /usr/local/bin/brew)
    set(CMAKE_IGNORE_PATH /usr/local /usr/local/bin /usr/local/include /usr/local/Library/Frameworks /usr/local/lib /usr/local/opt/gettext/bin /usr/local/opt/gettext/lib ${CMAKE_IGNORE_PATH})
endif()
