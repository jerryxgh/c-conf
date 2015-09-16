message(STATUS "in src directory")
# 查找当前目录下的所有源文件
# 并将名称保存到CCONF_SRCS变量
# aux_source_directory(. CCONF_SRCS)
set(CCONF_SRCS
  cfg.c
  cfg.h
  str.c
  str.h
  )
# 生成静态链接库
add_library(cconf ${CCONF_SRCS})
