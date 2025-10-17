; ModuleID = 'VynModule'
source_filename = "VynModule"

%Person = type { ptr, i64 }

@0 = private unnamed_addr constant [6 x i8] c"Alice\00", align 1

define %Person @main() {
entry:
  %p = alloca %Person, align 8
  %Person_obj = alloca %Person, align 8
  %name_ptr = getelementptr inbounds %Person, ptr %Person_obj, i32 0, i32 0
  store ptr @0, ptr %name_ptr, align 8
  %age_ptr = getelementptr inbounds %Person, ptr %Person_obj, i32 0, i32 1
  store i64 30, ptr %age_ptr, align 4
  %Person_val = load %Person, ptr %Person_obj, align 8
  store %Person %Person_val, ptr %p, align 8
  %p1 = load %Person, ptr %p, align 8
  ret %Person %p1
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
