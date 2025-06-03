; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [6 x i8] c"hello\00", align 1

define i32 @main() {
entry:
  ret i32 ptrtoint (ptr @0 to i32)
}
