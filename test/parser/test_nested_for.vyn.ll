; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() {
entry:
  %j = alloca i64, align 8
  %i = alloca i64, align 8
  %sum = alloca i64, align 8
  store i64 0, ptr %sum, align 4
  br label %for.init

for.init:                                         ; preds = %entry
  store i64 1, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.update, %for.init
  %i1 = load i64, ptr %i, align 4
  %icmpsletmp = icmp sle i64 %i1, 3
  br i1 %icmpsletmp, label %for.body, label %for.exit

for.body:                                         ; preds = %for.cond
  br label %for.init2

for.update:                                       ; preds = %for.exit6
  %i14 = load i64, ptr %i, align 4
  %addtmp15 = add i64 %i14, 1
  store i64 %addtmp15, ptr %i, align 4
  br label %for.cond

for.exit:                                         ; preds = %for.cond
  %sum16 = load i64, ptr %sum, align 4
  ret i64 %sum16

for.init2:                                        ; preds = %for.body
  store i64 1, ptr %j, align 4
  br label %for.cond3

for.cond3:                                        ; preds = %for.update5, %for.init2
  %j7 = load i64, ptr %j, align 4
  %icmpsletmp8 = icmp sle i64 %j7, 2
  br i1 %icmpsletmp8, label %for.body4, label %for.exit6

for.body4:                                        ; preds = %for.cond3
  %sum9 = load i64, ptr %sum, align 4
  %i10 = load i64, ptr %i, align 4
  %j11 = load i64, ptr %j, align 4
  %multmp = mul i64 %i10, %j11
  %addtmp = add i64 %sum9, %multmp
  store i64 %addtmp, ptr %sum, align 4
  br label %for.update5

for.update5:                                      ; preds = %for.body4
  %j12 = load i64, ptr %j, align 4
  %addtmp13 = add i64 %j12, 1
  store i64 %addtmp13, ptr %j, align 4
  br label %for.cond3

for.exit6:                                        ; preds = %for.cond3
  br label %for.update
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
