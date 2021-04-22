project('telemetry_server',['cpp'],version:'0.1',license:'GPLv2+',default_options:['buildtype=debug','cpp_std=gnu++2a','prefix=/usr'])

sources=[
  'src/main.cpp'
]
includes=[
  include_directories('subprojects/config_loader/include')
]

lib_config_loader_dep= dependency('lib_config_loader',required: false)
if not lib_config_loader_dep.found()
  lib_config_loader_proj = subproject('config_loader')
  lib_config_loader_dep = lib_config_loader_proj.get_variable('lib_config_loader_dep')
endif

deps=[
  lib_config_loader_dep
]

doxygen=find_program('doxygen',required : false)
if doxygen.found()
  message('doxygen найден')
  run_target('docs',command : [doxygen, meson.source_root() +'/'+meson.project_name()+'.doxy'])
 
else
  warning('Сборка документации отключена, отсутствует doxygen')
endif
executable(meson.project_name(),sources,dependencies:deps,include_directories: includes,install:true)
