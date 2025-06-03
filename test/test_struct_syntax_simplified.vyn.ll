; ModuleID = 'VynModule'
source_filename = "VynModule"

%Point = type { i64, i64 }
%Rectangle = type { i64, i64 }
%Person = type { i64, ptr, double }
%Company = type { ptr, i64, i1 }

@0 = private unnamed_addr constant [6 x i8] c"Alice\00", align 1
@1 = private unnamed_addr constant [9 x i8] c"Acme Inc\00", align 1

define %Point @createPoint() {
entry:
  %Point_obj = alloca %Point, align 8
  %x_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 0
  store i64 10, ptr %x_ptr, align 4
  %y_ptr = getelementptr inbounds %Point, ptr %Point_obj, i32 0, i32 1
  store i64 20, ptr %y_ptr, align 4
  ret %Point undef
}

define %Rectangle @createRectangle() {
entry:
  %Rectangle_obj = alloca %Rectangle, align 8
  %width_ptr = getelementptr inbounds %Rectangle, ptr %Rectangle_obj, i32 0, i32 0
  store i64 100, ptr %width_ptr, align 4
  %height_ptr = getelementptr inbounds %Rectangle, ptr %Rectangle_obj, i32 0, i32 1
  store i64 50, ptr %height_ptr, align 4
  ret %Rectangle undef
}

define i64 @main() {
entry:
  %Person_obj = alloca %Person, align 8
  %id_ptr = getelementptr inbounds %Person, ptr %Person_obj, i32 0, i32 0
  store i64 42, ptr %id_ptr, align 4
  %name_ptr = getelementptr inbounds %Person, ptr %Person_obj, i32 0, i32 1
  store ptr @0, ptr %name_ptr, align 8
  %salary_ptr = getelementptr inbounds %Person, ptr %Person_obj, i32 0, i32 2
  store double 7.500050e+04, ptr %salary_ptr, align 8
  %Company_obj = alloca %Company, align 8
  %name_ptr1 = getelementptr inbounds %Company, ptr %Company_obj, i32 0, i32 0
  store ptr @1, ptr %name_ptr1, align 8
  %employeeCount_ptr = getelementptr inbounds %Company, ptr %Company_obj, i32 0, i32 1
  store i64 250, ptr %employeeCount_ptr, align 4
  %isPublic_ptr = getelementptr inbounds %Company, ptr %Company_obj, i32 0, i32 2
  store i1 true, ptr %isPublic_ptr, align 1
  ret i64 0
}
