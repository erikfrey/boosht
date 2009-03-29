PROJECT(boosht)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

SET(CMAKE_VERBOSE_MAKEFILE ON)

# binaries get installed here
SET(CMAKE_INSTALL_PREFIX "/usr/")

#add definitions, compiler switches, etc.
ADD_DEFINITIONS(-Wall -O2 -DNDEBUG -g `getconf LFS_CFLAGS`)

INCLUDE_DIRECTORIES(include
                    /opt/local/include
                    )
          
# ADD_LIBRARY(moost_http STATIC
#            src/http/mime_types.cpp
#            src/http/reply.cpp
#            src/http/filesystem_request_handler.cpp
#            src/http/request_parser.cpp
#            )            

# INSTALL(TARGETS moost_http ARCHIVE DESTINATION lib)

INSTALL(DIRECTORY include/boosht DESTINATION include PATTERN ".git" EXCLUDE)
