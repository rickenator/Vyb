; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @test_standard(i64 %a, i64 %b) {
entry:
  %b2 = alloca i64, align 8
  %a1 = alloca i64, align 8
  store i64 %a, ptr %a1, align 4
  store i64 %b, ptr %b2, align 4
  %a3 = load i64, ptr %a1, align 4
  %b4 = load i64, ptr %b2, align 4
  %addtmp = add i64 %a3, %b4
  ret i64 %addtmp
}

define i64 @test_shorthand(i64 %a, i64 %b) {
entry:
  %b2 = alloca i64, align 8
  %a1 = alloca i64, align 8
  store i64 %a, ptr %a1, align 4
  store i64 %b, ptr %b2, align 4
  %a3 = load i64, ptr %a1, align 4
  %b4 = load i64, ptr %b2, align 4
  %multmp = mul i64 %a3, %b4
  ret i64 %multmp
}

define i64 @test_mixed(i64 %x, i64 %y, i64 %z) {
entry:
  %z3 = alloca i64, align 8
  %y2 = alloca i64, align 8
  %x1 = alloca i64, align 8
  store i64 %x, ptr %x1, align 4
  store i64 %y, ptr %y2, align 4
  store i64 %z, ptr %z3, align 4
  %x4 = load i64, ptr %x1, align 4
  %y5 = load i64, ptr %y2, align 4
  %addtmp = add i64 %x4, %y5
  %z6 = load i64, ptr %z3, align 4
  %addtmp7 = add i64 %addtmp, %z6
  ret i64 %addtmp7
}

define i64 @main() {
entry:
  %r3 = alloca i64, align 8
  %r2 = alloca i64, align 8
  %r1 = alloca i64, align 8
  %calltmp = call i64 @test_standard(i64 10, i64 5)
  store i64 %calltmp, ptr %r1, align 4
  %calltmp1 = call i64 @test_shorthand(i64 4, i64 3)
  store i64 %calltmp1, ptr %r2, align 4
  %calltmp2 = call i64 @test_mixed(i64 1, i64 2, i64 3)
  store i64 %calltmp2, ptr %r3, align 4
  %r13 = load i64, ptr %r1, align 4
  %r24 = load i64, ptr %r2, align 4
  %addtmp = add i64 %r13, %r24
  %r35 = load i64, ptr %r3, align 4
  %addtmp6 = add i64 %addtmp, %r35
  ret i64 %addtmp6
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
