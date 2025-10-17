; ModuleID = 'VynModule'
source_filename = "VynModule"

define i64 @main() {
entry:
  %x = alloca i64, align 8
  store i64 42, ptr %x, align 4
  %x1 = load i64, ptr %x, align 4
  br label %match.case.0

match.end:                                        ; preds = %match.default, %match.case.body.0
  ret i64 0

match.case.0:                                     ; preds = %entry
  %match.icmp = icmp eq i64 %x1, 42
  br i1 %match.icmp, label %match.case.body.0, label %match.default

match.case.body.0:                                ; preds = %match.case.0
  br label %match.end

match.default:                                    ; preds = %match.case.0
  br label %match.end
}

declare void @__vyn_println(ptr)

declare ptr @__vyn_serialize_to_json(ptr, ptr)

declare ptr @__vyn_convert_lit_string(ptr)
