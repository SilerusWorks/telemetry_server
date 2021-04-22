project('config_loader',['cpp'],version:'0.1',license:'GPLv2+',default_options:['build_type=debug','cpp_std=gnu++2a','prefix=/usr'])

sources=[
  'src/config_loader.cpp',
  'src/device_config.cpp'
]
includes=[
  include_directories('include')
]

deps=[
]
add_project_link_arguments(['-lstdc++fs'], language : 'cpp')
lib_config_loader=static_library(meson.project_name(),sources,dependencies:deps,include_directories:includes,install:true)
lib_config_loader_dep=declare_dependency(include_directories: includes, dependencies: deps, link_with: lib_config_loader)
