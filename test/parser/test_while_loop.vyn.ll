; ModuleID = 'VynModule'
source_filename = "VynModule"

define i32 @main() {
entry:
  %sum = alloca i32, align 4
  %counter = alloca i32, align 4
  store i32 0, ptr %counter, align 4
  store i32 0, ptr %sum, align 4
  br label %loop.header

loop.header:                                      ; preds = %loop.body, %entry
  %counter1 = load i32, ptr %counter, align 4
  %icmpslttmp = icmp slt i32 %counter1, 5
  br i1 %icmpslttmp, label %loop.body, label %loop.exit

loop.body:                                        ; preds = %loop.header
  %sum2 = load i32, ptr %sum, align 4
  %counter3 = load i32, ptr %counter, align 4
  %addtmp = add i32 %sum2, %counter3
  store i32 %addtmp, ptr %sum, align 4
  %counter4 = load i32, ptr %counter, align 4
  %addtmp5 = add i32 %counter4, 1
  store i32 %addtmp5, ptr %counter, align 4
  br label %loop.header

loop.exit:                                        ; preds = %loop.header
  %sum6 = load i32, ptr %sum, align 4
  ret i32 %sum6
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
