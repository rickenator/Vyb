; ModuleID = 'VynModule'
source_filename = "VynModule"

define i32 @main() {
entry:
  %result = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 5, ptr %x, align 4
  store i32 0, ptr %result, align 4
  %x1 = load i32, ptr %x, align 4
  %icmpsgttmp = icmp sgt i32 %x1, 10
  br i1 %icmpsgttmp, label %then, label %else

then:                                             ; preds = %entry
  store i64 42, ptr %result, align 4
  br label %ifcont

else:                                             ; preds = %entry
  store i64 99, ptr %result, align 4
  br label %ifcont

ifcont:                                           ; preds = %else, %then
  %result2 = load i32, ptr %result, align 4
  ret i32 %result2
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
