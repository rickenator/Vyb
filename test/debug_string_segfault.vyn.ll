; ModuleID = 'VynModule'
source_filename = "VynModule"

@.str = private constant [6 x i8] c"hello\00"
@y = private global ptr @.str

define void @main() {
entry:
  ret void
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
