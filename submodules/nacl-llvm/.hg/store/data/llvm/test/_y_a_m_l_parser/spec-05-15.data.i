         ]   \       W��������E9>\����ä�9iJ�MY�            u# RUN: yaml-bench -canonical %s |& FileCheck %s

Bad escapes:
  "\c
  \xq-"

# CHECK: error
