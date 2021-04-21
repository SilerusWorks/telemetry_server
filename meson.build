project('telemetry_server',['cpp'],version:'0.1',license:'GPLv2+',default_options:['buildtype=debug','cpp_std=gnu++2a','prefix=/usr'])

sources=[
  'src/main.cpp'
]
includes=[]

deps=[]

doxygen=find_program('doxygen',required : false)
if doxygen.found()
  message('doxygen найден')
  run_target('docs',command : [doxygen, meson.source_root() +'/'+meson.project_name()+'.doxy'])
 
else
  warning('Сборка документации отключена, отсутствует doxygen')
endif
executable(meson.project_name(),sources,dependencies:deps,include_directories: includes,install:true)
