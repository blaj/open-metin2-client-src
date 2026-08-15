add_subdirectory(lzo-2.10)
add_subdirectory(mio)
add_subdirectory(libsodium)

## zstd is a bit tricky
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "BUILD SHARED LIBRARIES" FORCE)
add_subdirectory(zstd-1.5.7/build/cmake zstd)
include_directories("zstd/lib")

## FreeType - disable optional dependencies we don't need
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
add_subdirectory(freetype-2.13.3)

# Msg Pack
add_library(msgpack-cxx INTERFACE)
target_include_directories(msgpack-cxx INTERFACE ${CMAKE_SOURCE_DIR}/vendor/msgpack-c-7.0.0/include)
target_compile_definitions(msgpack-cxx INTERFACE
    MSGPACK_NO_BOOST
    MSGPACK_ENDIAN_LITTLE_BYTE=1
    MSGPACK_ENDIAN_BIG_BYTE=0
)

# Curl
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIBS ON CACHE BOOL "" FORCE)
set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CURL_USE_LIBPSL OFF CACHE BOOL "" FORCE)
set(CURL_ZLIB OFF CACHE BOOL "" FORCE)
set(CURL_USE_SCHANNEL ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_LDAP ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_LDAPS ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_FTP ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_TELNET ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_DICT ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_GOPHER ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_SMTP ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_POP3 ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_IMAP ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_RTSP ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_TFTP ON CACHE BOOL "" FORCE)
set(CURL_DISABLE_MQTT ON CACHE BOOL "" FORCE)
add_subdirectory(curl-8.21.0)

if (WIN32)
	set_target_properties(lzo2 PROPERTIES FOLDER vendor)
	set_target_properties(sodium PROPERTIES FOLDER vendor)
	set_target_properties(freetype PROPERTIES FOLDER vendor)

	## zstd stuff
	set_target_properties(zstd PROPERTIES FOLDER vendor/zstd)
	set_target_properties(libzstd_static PROPERTIES FOLDER vendor/zstd)
	set_target_properties(clean-all PROPERTIES FOLDER vendor/zstd)
	set_target_properties(uninstall PROPERTIES FOLDER vendor/zstd)

	set_target_properties(msgpack-cxx PROPERTIES FOLDER vendor)

	set_target_properties(libcurl_static PROPERTIES FOLDER vendor)
endif()
