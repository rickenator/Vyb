; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() {
entry:
  %count = alloca i64, align 8
  store i64 0, ptr %count, align 4
  br label %loop.header

loop.header:                                      ; preds = %ifcont7, %then, %entry
  %count1 = load i64, ptr %count, align 4
  %icmpslttmp = icmp slt i64 %count1, 10
  br i1 %icmpslttmp, label %loop.body, label %loop.exit

loop.body:                                        ; preds = %loop.header
  %count2 = load i64, ptr %count, align 4
  %addtmp = add i64 %count2, 1
  store i64 %addtmp, ptr %count, align 4
  %count3 = load i64, ptr %count, align 4
  %icmpeqtmp = icmp eq i64 %count3, 3
  br i1 %icmpeqtmp, label %then, label %ifcont

loop.exit:                                        ; preds = %then6, %loop.header
  %count8 = load i64, ptr %count, align 4
  ret i64 %count8

then:                                             ; preds = %loop.body
  br label %loop.header

ifcont:                                           ; preds = %loop.body
  %count4 = load i64, ptr %count, align 4
  %icmpeqtmp5 = icmp eq i64 %count4, 7
  br i1 %icmpeqtmp5, label %then6, label %ifcont7

then6:                                            ; preds = %ifcont
  br label %loop.exit

ifcont7:                                          ; preds = %ifcont
  br label %loop.header
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
