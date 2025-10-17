; ModuleID = 'VynModule'
source_filename = "VynModule"

@0 = private unnamed_addr constant [6 x i8] c"Alice\00", align 1

define i64 @getUserId() {
entry:
  ret i64 42
}

define ptr @getUserName() {
entry:
  ret ptr @0
}

define i64 @getScore() {
entry:
  ret i64 100
}

define i32 @main() {
entry:
  %score = alloca i64, align 8
  %name = alloca ptr, align 8
  %id = alloca i64, align 8
  %call_getUserId = call i64 @getUserId()
  store i64 %call_getUserId, ptr %id, align 4
  %call_getUserName = call ptr @getUserName()
  store ptr %call_getUserName, ptr %name, align 8
  %call_getScore = call i64 @getScore()
  store i64 %call_getScore, ptr %score, align 4
  ret i32 0
}
