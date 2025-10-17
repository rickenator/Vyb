; ModuleID = 'VynModule'
source_filename = "VynModule"

@.str = private constant [23 x i8] c"Hello, unified syntax!\00"
@message = private global ptr @.str

define ptr @main() {
entry:
  ret ptr undef
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
