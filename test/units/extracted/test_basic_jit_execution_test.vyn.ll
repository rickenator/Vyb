; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() {
entry:
  %x = alloca i64, align 8
  store i64 42, ptr %x, align 4
  %x_load = load i64, ptr %x, align 4
  ret i64 %x_load
}
