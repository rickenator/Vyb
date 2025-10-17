; ModuleID = 'VynModule'
source_filename = "VynModule"

define i32 @multiply(i32 %a, i32 %b) {
entry:
  %b2 = alloca i32, align 4
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  %a3 = load i32, ptr %a1, align 4
  %b4 = load i32, ptr %b2, align 4
  %multmp = mul i32 %a3, %b4
  ret i32 %multmp
}

define i32 @calculate(i32 %x, i32 %y) {
entry:
  %product = alloca i32, align 4
  %sum = alloca i32, align 4
  %y2 = alloca i32, align 4
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  store i32 %y, ptr %y2, align 4
  %x3 = load i32, ptr %x1, align 4
  %y4 = load i32, ptr %y2, align 4
  %addtmp = add i32 %x3, %y4
  store i32 %addtmp, ptr %sum, align 4
  %sum5 = load i32, ptr %sum, align 4
  %calltmp = call i32 @multiply(i32 %sum5, i32 2)
  store i32 %calltmp, ptr %product, align 4
  %product6 = load i32, ptr %product, align 4
  ret i32 %product6
}

define i32 @main() {
entry:
  %result = alloca i32, align 4
  %calltmp = call i32 @calculate(i32 3, i32 7)
  store i32 %calltmp, ptr %result, align 4
  %result1 = load i32, ptr %result, align 4
  ret i32 %result1
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
