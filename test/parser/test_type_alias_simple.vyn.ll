; ModuleID = 'VynModule'
source_filename = "VynModule"

define i32 @main() {
entry:
  %x = alloca i64, align 8
  store i64 47, ptr %x, align 4
  %x_value = load i64, ptr %x, align 4
  %main_exit_code = trunc i64 %x_value to i32
  ret i32 %main_exit_code
}
