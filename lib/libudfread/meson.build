# Copyright (C) 2025 Rob Hall
# SPDX-License-Identifier: MIT

project('libudfread', 'c',
	  version: '1.2.0',
	  default_options: ['buildtype=debugoptimized',
                      'c_std=c99',
                      'default_library=both',
                      'optimization=3',
                      'warning_level=3'],
	  meson_version: '>= 0.60.0')

cc = meson.get_compiler('c')

# Configuration data for config.h
cdata = configuration_data()
cdata.set('HAVE_UDFREAD_VERSION_H', 1)
add_project_arguments('-DHAVE_CONFIG_H=1', language: 'c')

# Include directories
libudfread_inc_dirs = include_directories('.', 'src', 'src/udfread')

# The version number for the shared library
libudfread_soname_version = '3.0.0'

libudfread_version = meson.project_version()
libudfread_version_split = libudfread_version.split('.')

#
# OS/Compiler checks and defines
#

# Arguments in test_args will be used even on feature tests
test_args = ['-D_ISOC99_SOURCE']

optional_arguments = []

if host_machine.system() != 'windows'
    test_args += '-D_POSIX_C_SOURCE=200809L'
endif

# Header checks

check_headers = [
    'unistd.h',
    'fcntl.h',
]

foreach h: check_headers
    if cc.has_header(h, args: test_args)
        cdata.set('HAVE_' + h.underscorify().to_upper(), 1)
    endif
endforeach

if host_machine.system() != 'windows'
    if not cc.has_function('pread', prefix: '#include <unistd.h>', args: test_args)
        error('Function pread not found.')
    endif
endif

# External dependencies and libraries

if host_machine.system() == 'windows'
    thread_dependency = []
else
    thread_dependency = dependency('threads', required: false)
    if thread_dependency.found()
        cdata.set('HAVE_PTHREAD_H', 1)
    endif
endif

# Compiler flag tests

# Compiler flags that should be set
# But when the compiler does not supports them
# it is not an error and silently tolerated
if cc.get_argument_syntax() != 'msvc'
    optional_arguments += [
        '-Wdisabled-optimization',
        '-Wpointer-arith',
        '-Wredundant-decls',
        '-Wcast-qual',
        '-Wwrite-strings',
        '-Wundef',
        '-Wshadow',
        '-Wmissing-prototypes',
        '-Werror=implicit-function-declaration',
        '-Werror-implicit-function-declaration',
    ]
    if get_option('warning_level') in ['3', 'everything']
        optional_arguments += '-Winline'
    endif
    if get_option('optimization') == '3'
        optional_arguments += '-fomit-frame-pointer'
    endif
endif

add_project_arguments(test_args, language: 'c')
add_project_arguments(cc.get_supported_arguments(optional_arguments), language: 'c')

# Generate config.h
config_h_target = configure_file(output: 'config.h', configuration: cdata)

# Include subdir meson.build files
# The order is important!

subdir('src/udfread')

subdir('src')

subdir('examples')
