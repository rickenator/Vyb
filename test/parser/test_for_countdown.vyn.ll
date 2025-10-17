; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() {
entry:
  %i = alloca i64, align 8
  %sum = alloca i64, align 8
  store i64 0, ptr %sum, align 4
  br label %for.init

for.init:                                         ; preds = %entry
  store i64 5, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.update, %for.init
  %i1 = load i64, ptr %i, align 4
  %icmpsgttmp = icmp sgt i64 %i1, 0
  br i1 %icmpsgttmp, label %for.body, label %for.exit

for.body:                                         ; preds = %for.cond
  %sum2 = load i64, ptr %sum, align 4
  %i3 = load i64, ptr %i, align 4
  %addtmp = add i64 %sum2, %i3
  store i64 %addtmp, ptr %sum, align 4
  br label %for.update

for.update:                                       ; preds = %for.body
  %i4 = load i64, ptr %i, align 4
  %subtmp = sub i64 %i4, 1
  store i64 %subtmp, ptr %i, align 4
  br label %for.cond

for.exit:                                         ; preds = %for.cond
  %sum5 = load i64, ptr %sum, align 4
  ret i64 %sum5
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
