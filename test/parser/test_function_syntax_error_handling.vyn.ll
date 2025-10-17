; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [6 x i8] c"works\00", align 1

define ptr @correct_syntax() {
entry:
  ret ptr @0
}

define i32 @main() {
entry:
  %call_correct_syntax = call ptr @correct_syntax()
  %return_cast = ptrtoint ptr %call_correct_syntax to i32
  ret i32 %return_cast
}
